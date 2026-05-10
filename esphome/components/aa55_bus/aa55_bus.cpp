#include <deque>
#include <iterator>
#include <cmath>
#include <algorithm>
#include <climits>
#include <numeric>
#include "esphome/core/log.h"
#include "aa55_bus.h"
#include "../aa55_inverter/aa55_inverter.h"

namespace esphome {
namespace aa55_bus {
static constexpr const char *LOGGING_TAG = "aa55_bus";

bool RingBuffer::push(uint8_t byte) {
  if (this->full())
    return false;
  this->data_[this->head_] = byte;
  this->head_ = (this->head_ + 1) % this->CAPACITY;
  this->size_++;
  return true;
}

uint8_t RingBuffer::peek(size_t offset) const { return this->data_[(this->tail_ + offset) % this->CAPACITY]; }

void RingBuffer::consume(size_t bytes_to_remove) {
  bytes_to_remove = std::min(bytes_to_remove, this->size_);
  this->tail_ = (this->tail_ + bytes_to_remove) % this->CAPACITY;
  this->size_ -= bytes_to_remove;
}

void RingBuffer::clear() {
  this->head_ = 0;
  this->tail_ = 0;
  this->size_ = 0;
}

size_t RingBuffer::size() const { return this->size_; }

bool RingBuffer::empty() const { return this->size_ == 0; }

bool RingBuffer::full() const { return this->size_ == this->CAPACITY; }

AA55Bus::AA55Bus(std::string id, uint8_t controller_address) : uart::UARTDevice(), Component() {
  this->id_ = id;
  this->controller_address_ = controller_address;
}

void AA55Bus::setup() {}

void AA55Bus::add_inverter(aa55_inverter::AA55Inverter *inverter) { this->configured_inverters_.push_back(inverter); }

void AA55Bus::queue_command(aa55_bus::AA55TXPacket command) { this->commands_to_send_.push(command); }

std::string AA55Bus::get_component_id() { return this->id_; }

void AA55Bus::add_registered_inverter(aa55_inverter::AA55Inverter *inverter) {
  this->registered_inverters_.push_back(inverter);
}

void AA55Bus::remove_registered_inverter(aa55_inverter::AA55Inverter *inverter) {
  this->registered_inverters_.erase(
      std::remove(this->registered_inverters_.begin(), this->registered_inverters_.end(), inverter),
      this->registered_inverters_.end());
}

void AA55Bus::dump_config() {
  ESP_LOGCONFIG(LOGGING_TAG, "Goodwe AA55 Bus component");
  ESP_LOGCONFIG(LOGGING_TAG, "  Bus ID: %s", this->id_.c_str());
  ESP_LOGCONFIG(LOGGING_TAG, "  Controller address: %x", this->controller_address_);
}

void AA55Bus::loop() {
  const uint32_t loop_start_time = millis();

  // Process received data if applicable
  if (this->available()) {
    this->process_rx(loop_start_time);
  }

  // Check if we need to send out an offline query (every 60s if a configured inverter is not yet registered)
  if (this->configured_inverters_.size() > this->registered_inverters_.size() &&
      loop_start_time - this->last_offline_request_send_time_ >= aa55_bus::OFFLINE_QUERY_INTERVAL) {
    // Check if queue already contains an offline query command to avoid flooding
    if (!this->commands_to_send_.contains_if([](const aa55_bus::AA55TXPacket &packet) {
          return packet.function_code == aa55_bus::FUNCTION_CODE::OFFLINE_QUERY;
        })) {
      ESP_LOGD(
          LOGGING_TAG,
          "Queuing offline query command because there are unregistered inverters and the offline query interval of "
          "%dms has passed.",
          aa55_bus::OFFLINE_QUERY_INTERVAL);
      this->queue_command(make_offline_query_packet());
    }
  }

  // Send first queued packet if applicable, take into account 500ms delay (see AA55 doc) before last sent packet
  if (!this->commands_to_send_.empty() && loop_start_time - this->last_send_time_ >= aa55_bus::COMMAND_DELAY) {
    this->send_packet(this->commands_to_send_.front());

    this->last_send_time_ = loop_start_time;
    if (this->commands_to_send_.front().function_code == aa55_bus::FUNCTION_CODE::OFFLINE_QUERY) {
      this->last_offline_request_send_time_ = loop_start_time;
    }

    this->commands_to_send_.pop();
    ESP_LOGV(LOGGING_TAG, "Remaining commands in queue for bus %s: %d", this->id_.c_str(),
             this->commands_to_send_.size());
  }
}

void AA55Bus::send_packet(const aa55_bus::AA55TXPacket &command) {
  // Max packet = Allocate register address, payload 17 bytes + 9 bytes header/metadata = 26 bytes
  uint8_t packet[26];
  uint8_t length = 0;
  packet[length++] = 0xAA;
  packet[length++] = 0x55;
  packet[length++] = this->controller_address_;
  packet[length++] = command.destination_address;
  packet[length++] = (uint8_t) command.control_code;
  packet[length++] = (uint8_t) command.function_code;
  packet[length++] = command.payload_length;
  memcpy(packet + length, command.payload, command.payload_length);
  length += command.payload_length;
  uint16_t checksum = 0;
  for (uint8_t i = 0; i < length; i++)
    checksum += packet[i];
  packet[length++] = (uint8_t) (checksum >> 8);
  packet[length++] = (uint8_t) checksum;

  ESP_LOGD(LOGGING_TAG, "Sending packet %s", this->create_hex_string(packet, length).c_str());
  this->write_array(packet, length);  // Send packet over UART
}

void AA55Bus::process_rx(const uint32_t &loop_start_time) {
  // Drop all RX buffer contents on buffer overload
  if (this->receive_buffer_.full()) {
    ESP_LOGV(LOGGING_TAG, "UART RX buffer contents: %s", this->create_hex_string(this->receive_buffer_).c_str());
    ESP_LOGW(LOGGING_TAG, "UART RX buffer for bus %s has filled up. Clearing buffer...", this->id_.c_str());

    this->receive_buffer_.clear();

    // Drain UART hardware buffer
    uint8_t buf[64];
    while (this->available() > 0 && millis() - loop_start_time < 30) {
      const size_t to_drain = std::min((size_t) this->available(), sizeof(buf));
      if (!this->read_array(buf, to_drain))
        break;
    }
    return;
  }

  uint8_t read_buffer[READ_BATCH_SIZE];

  bool packet_header_found{false};
  size_t packet_header_start_search_index{0};
  uint8_t packet_size{UINT8_MAX};

  while (this->available() && !this->receive_buffer_.full() &&
         millis() - loop_start_time < 30) {  // Avoid blocking the thread for 30ms+
    // Read available bytes in fixed-size batches to reduce UART call overhead
    const size_t avail = (size_t) this->available();
    const size_t to_read = std::min(avail, READ_BATCH_SIZE);
    ESP_LOGD(LOGGING_TAG, "%d bytes are available from UART, max buffer size is %d, reading %d bytes...", avail,
             READ_BATCH_SIZE, to_read);
    if (!this->read_array(read_buffer, to_read))
      break;

    for (size_t i = 0; i < to_read; i++)
      this->receive_buffer_.push(read_buffer[i]);

    ESP_LOGV(LOGGING_TAG, "Updated receive_buffer_ contents: %s",
             this->create_hex_string(this->receive_buffer_).c_str());

    // Find header
    if (!packet_header_found) {
      ESP_LOGV(LOGGING_TAG, "Looking for header in receive_buffer_...");

      // Search for 0xAA 0x55 header by scanning ring buffer via peek()
      size_t header_pos = SIZE_MAX;
      for (size_t i = packet_header_start_search_index; i + 1 < this->receive_buffer_.size(); i++) {
        if (this->receive_buffer_.peek(i) == 0xAA && this->receive_buffer_.peek(i + 1) == 0x55) {
          header_pos = i;
          break;
        }
      }

      if (header_pos == SIZE_MAX) {
        ESP_LOGV(LOGGING_TAG, "Could not find header in receive_buffer_ yet. Reading more data...");
        packet_header_start_search_index = this->receive_buffer_.size() > 1 ? this->receive_buffer_.size() - 1 : 0;
        continue;
      }

      ESP_LOGD(LOGGING_TAG, "Found header at receive_buffer_ index %d", header_pos);

      // Discard bytes before the header
      if (header_pos > 0) {
        ESP_LOGV(LOGGING_TAG, "Stripping %d bytes before header from ring buffer", header_pos);
        this->receive_buffer_.consume(header_pos);
        ESP_LOGV(LOGGING_TAG, "New receive_buffer_ contents: %s",
                 this->create_hex_string(this->receive_buffer_).c_str());
      }

      packet_header_found = true;
    }

    // Determine packet size once we have at least 7 bytes
    if (packet_size == UINT8_MAX) {
      if (this->receive_buffer_.size() < 7) {
        ESP_LOGV(LOGGING_TAG, "Could not find payload size in receive_buffer_ yet. Reading more data...");
        continue;
      }

      ESP_LOGD(LOGGING_TAG, "Found payload size byte, value: %d..", this->receive_buffer_.peek(6));
      // Total = 2 (header) + 1 (source) + 1 (destination) + 1 (control code) + 1 (function code) + 1 (length) + payload
      // + 2 (CRC) = 9 + payload
      packet_size = 9 + this->receive_buffer_.peek(6);
    }

    // Check if packet is fully received
    if (this->receive_buffer_.size() < packet_size) {
      ESP_LOGV(LOGGING_TAG, "Packet of %d bytes was not yet fully received. Reading more data...", packet_size);
      continue;
    }

    ESP_LOGD(LOGGING_TAG, "Packet of %d bytes was fully received from UART.", packet_size);
    ESP_LOGD(LOGGING_TAG, "Verifying received packet checksum...");

    // Verify checksum by summing all bytes except the last 2 (= CRC)
    uint16_t calculated_checksum = 0;
    for (size_t i = 0; i < packet_size - 2; i++)
      calculated_checksum += this->receive_buffer_.peek(i);
    const uint16_t received_checksum =
        ((uint16_t) this->receive_buffer_.peek(packet_size - 2) << 8) | this->receive_buffer_.peek(packet_size - 1);

    if (received_checksum != calculated_checksum) {
      ESP_LOGW(LOGGING_TAG,
               "Packet has an incorrect checksum, received checksum %d, calculated checksum %d. Discarding...",
               received_checksum, calculated_checksum);
      // Skip past the header so the next search finds the next valid header
      this->receive_buffer_.consume(2);
      packet_header_found = false;
      packet_header_start_search_index = 0;
      packet_size = UINT8_MAX;
      continue;
    }

    // Check if the packet is destined for this controller
    if (this->receive_buffer_.peek(3) != this->controller_address_) {
      ESP_LOGV(LOGGING_TAG, "Received packet for another device (%x). Discarding...", this->receive_buffer_.peek(3));
      this->receive_buffer_.consume(packet_size);
      packet_header_found = false;
      packet_size = UINT8_MAX;
      continue;
    }

    // Parse received data into AA55 packet struct
    aa55_bus::AA55RXPacket response_packet{};
    response_packet.source_address = this->receive_buffer_.peek(2);
    response_packet.control_code = static_cast<aa55_bus::CONTROL_CODE>(this->receive_buffer_.peek(4));
    response_packet.function_code = static_cast<aa55_bus::FUNCTION_CODE>(this->receive_buffer_.peek(5));
    response_packet.payload_length = this->receive_buffer_.peek(6);
    for (uint8_t i = 0; i < response_packet.payload_length; i++)
      response_packet.payload[i] = this->receive_buffer_.peek(7 + i);

    std::vector<aa55_inverter::AA55Inverter *>::iterator find_inverter_it;
    if (response_packet.control_code == aa55_bus::CONTROL_CODE::REGISTER &&
        response_packet.function_code == aa55_bus::FUNCTION_CODE::REG_REQUEST) {
      // Registration request: match by serial number embedded in payload
      std::string discovered_serial_number(response_packet.payload,
                                           response_packet.payload + response_packet.payload_length);

      ESP_LOGD(LOGGING_TAG,
               "Received register request from inverter with serial number %s. Looking for configured inverter...",
               discovered_serial_number.c_str());

      find_inverter_it = std::find_if(this->configured_inverters_.begin(), this->configured_inverters_.end(),
                                      [&discovered_serial_number](aa55_inverter::AA55Inverter *inverter) {
                                        return inverter->get_serial_number() == discovered_serial_number;
                                      });

      if (find_inverter_it == this->configured_inverters_.end()) {
        ESP_LOGW(LOGGING_TAG, "Could not find a configured inverter with serial number %s. Discarding...",
                 discovered_serial_number.c_str());
        this->receive_buffer_.consume(packet_size);
        packet_header_found = false;
        packet_size = UINT8_MAX;
        continue;
      }

      ESP_LOGD(LOGGING_TAG, "Received register request from a configured inverter with serial number %s.",
               discovered_serial_number.c_str());
    } else {
      // All other packets: match by source address
      uint8_t packet_source_address = response_packet.source_address;
      find_inverter_it = std::find_if(this->configured_inverters_.begin(), this->configured_inverters_.end(),
                                      [packet_source_address](aa55_inverter::AA55Inverter *inverter) {
                                        return inverter->get_device_address() == packet_source_address;
                                      });
      if (find_inverter_it == this->configured_inverters_.end()) {
        ESP_LOGD(LOGGING_TAG, "Received packet from an unregistered inverter (%x). Discarding...",
                 packet_source_address);
        this->receive_buffer_.consume(packet_size);
        packet_header_found = false;
        packet_size = UINT8_MAX;
        continue;
      }

      ESP_LOGD(LOGGING_TAG, "Received packet from a registered inverter (%x).", packet_source_address);
    }

    // Dispatch to inverter
    (*find_inverter_it)->handle_packet(response_packet);

    this->receive_buffer_.consume(packet_size);
    packet_header_found = false;
    packet_size = UINT8_MAX;
  }
}

std::string AA55Bus::create_hex_string(const uint8_t *data, size_t length) {
  std::string result;
  result.reserve(length * 3);
  const char *hex = "0123456789ABCDEF";
  for (size_t i = 0; i < length; i++) {
    result.push_back(hex[(data[i] >> 4) & 0xF]);
    result.push_back(hex[data[i] & 0xF]);
    result.push_back(' ');
  }
  if (!result.empty())
    result.pop_back();
  return result;
}

std::string AA55Bus::create_hex_string(const RingBuffer &buffer) {
  std::string result;
  result.reserve(buffer.size() * 3);
  const char *hex = "0123456789ABCDEF";
  for (size_t i = 0; i < buffer.size(); i++) {
    uint8_t byte = buffer.peek(i);
    result.push_back(hex[(byte >> 4) & 0xF]);
    result.push_back(hex[byte & 0xF]);
    result.push_back(' ');
  }
  if (!result.empty())
    result.pop_back();
  return result;
}

}  // namespace aa55_bus
}  // namespace esphome
