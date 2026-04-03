#include "esphome/core/log.h"
#include "aa55_bus.h"
#include "../aa55_inverter/aa55_inverter.h"
#include <deque>
#include <iterator>
#include <cmath>
#include <algorithm>
#include <climits>
#include <numeric>

namespace esphome {
namespace aa55_bus {

AA55Bus::AA55Bus(uint8_t master_address) { master_address_ = master_address; }

void AA55Bus::setup() {}

void AA55Bus::dump_config() {
  ESP_LOGCONFIG(LOGGING_TAG, "Goodwe AA55 Bus component");
  ESP_LOGCONFIG(LOGGING_TAG, "  Bus ID: %s", this->id_.c_str());
  ESP_LOGCONFIG(LOGGING_TAG, "  Master address: %x", this->master_address_);
}

void AA55Bus::loop() {
  // Process received data if applicable
  if (this->available()) {
    this->process_rx();
  }

  // Send first queued packet if applicable, take into account 500ms delay (see AA55 doc) before last sent packet
  if (!this->commands_to_send_.empty() && millis() > this->last_send_time_ + 500) {
    this->send_packet(this->commands_to_send_.front());
    this->commands_to_send_.pop();
    this->last_send_time_ = millis();
    ESP_LOGV(LOGGING_TAG, "Remaining commands in queue for bus %s: %d", this->get_component_id().c_str(),
             this->commands_to_send_.size());
  }
}

void AA55Bus::send_packet(const aa55_const::AA55Packet &command) {
  std::vector<uint8_t> packet = aa55_const::HEADERS;  // Initialize message with AA55 header, then add command details
  packet.push_back(command.source_address);
  packet.push_back(command.destination_address);
  packet.push_back((uint8_t) command.control_code);
  packet.push_back((uint8_t) command.function_code);
  packet.push_back(command.payload.size());
  if (command.payload != aa55_const::EMPTY_VECTOR) {
    packet.insert(packet.end(), command.payload.begin(), command.payload.end());
  }
  const uint16_t checksum = std::accumulate(packet.begin(), packet.end(), 0);
  packet.push_back((uint8_t) (checksum >> 8));
  packet.push_back((uint8_t) checksum);

  ESP_LOGD(LOGGING_TAG, "Sending packet %s", this->create_hex_string(packet).c_str());
  this->write_array(packet);  // Send packet over UART
}

void AA55Bus::process_rx() {
  uint32_t start_time = millis();
  // Drop all RX buffer contents on buffer overload
  if (this->receive_buffer_.size() >= aa55_const::MAX_BUFFER_LENGTH) {
    ESP_LOGV(LOGGING_TAG, "UART RX buffer contents: %s", this->create_hex_string(this->receive_buffer_));
    ESP_LOGW(LOGGING_TAG, "UART RX buffer for bus %s has filled up. Clearing buffer...",
             this->get_component_id().c_str());

    // Clear deque buffer used for processing
    this->receive_buffer_.clear();

    // Clear UART buffer
    uint8_t buf[64];
    size_t avail;
    while ((avail = this->available()) > 0 && millis() < start_time + 30) {  // Avoid blocked the thread for 30ms+
      if (!this->read_array(buf, std::min(avail, sizeof(buf)))) {
        break;
      }
    }

    return;
  }

  const uint8_t buffer_max_size{64};
  bool packet_header_found{false};
  uint8_t packet_size{UINT8_MAX};
  bool packet_fully_received{false};

  while (this->available() && this->receive_buffer_.size() < aa55_const::MAX_BUFFER_LENGTH &&
         millis() < start_time + 30) {  // Avoid blocked the thread for 30ms+
    // Read all available bytes in batches to reduce UART call overhead.
    const uint8_t avail = this->available();
    const size_t to_read = std::min(avail, buffer_max_size);
    uint8_t buffer_array[to_read];
    ESP_LOGD(LOGGING_TAG, "%d bytes are available from UART, max buffer size is %d, reading %d bytes...", avail,
             buffer_max_size, to_read);
    if (!this->read_array(buffer_array, to_read)) {
      break;
    }

    // Add received bytes in the buffer array to the deque
    this->receive_buffer_.insert(this->receive_buffer_.end(), buffer_array, buffer_array + sizeof(buffer_array));
    ESP_LOGV(LOGGING_TAG, "Updated receive_buffer_ contents: %s",
             this->create_hex_string(this->receive_buffer_).c_str());

    // Find header
    if (!packet_header_found) {
      ESP_LOGV(LOGGING_TAG, "Looking for header in receive_buffer_...");

      // Search returns the iterator pointing to the start of the header match
      const auto find_header_it = std::search(this->receive_buffer_.begin(), this->receive_buffer_.end(),
                                              aa55_const::HEADERS.begin(), aa55_const::HEADERS.end());

      if (find_header_it == this->receive_buffer_.end()) {
        ESP_LOGV(LOGGING_TAG, "Could not find header in receive_buffer_ yet. Reading more data...");
        continue;
      }

      ESP_LOGD(LOGGING_TAG, "Found header at receive_buffer_ index %d",
               std::distance(this->receive_buffer_.begin(), find_header_it));

      // Strip bytes received before the packet header
      if (find_header_it != this->receive_buffer_.begin()) {
        ESP_LOGV(LOGGING_TAG, "Stripping %d bytes before header from deque",
                 std::distance(this->receive_buffer_.begin(), find_header_it));
        this->receive_buffer_.erase(this->receive_buffer_.begin(), find_header_it);
        ESP_LOGV(LOGGING_TAG, "New receive_buffer_ contents: %s",
                 this->create_hex_string(this->receive_buffer_).c_str());
      }

      packet_header_found = true;
    }

    // Calculate packet size if it is still unknown
    if (packet_size == UINT8_MAX) {
      ESP_LOGV(LOGGING_TAG, "Checking if receive_buffer_ contains packet size byte...");

      if (this->receive_buffer_.size() < 7) {
        ESP_LOGV(LOGGING_TAG, "Could not find payload size in receive_buffer_ yet. Reading more data...");
        continue;
      }

      ESP_LOGD(LOGGING_TAG, "Found payload size byte, value: %d..", this->receive_buffer_.at(6));
      packet_size = 9 + this->receive_buffer_.at(6);  // Packet total size = AA55 header + source address + destination
                                                      // address + control code + function code + payload size + CRC +
                                                      // payload size. Everything except payload = 9 bytes
    }

    // Check if packet is fully received
    if (this->receive_buffer_.size() < packet_size) {
      ESP_LOGV(LOGGING_TAG, "Packet of %d bytes was not yet fully received. Reading more data...", packet_size);
      continue;
    }

    ESP_LOGD(LOGGING_TAG, "Packet of %d bytes was fully received from UART.", packet_size);
    ESP_LOGD(LOGGING_TAG, "Verifying received packet checksum...");
    const uint16_t calculated_checksum =
        std::accumulate(this->receive_buffer_.begin(), this->receive_buffer_.begin() + packet_size - 2, 0);
    const uint16_t received_checksum =
        ((uint16_t) this->receive_buffer_.at(packet_size - 2) << 8) | this->receive_buffer_.at(packet_size - 1);

    if (received_checksum != calculated_checksum) {
      ESP_LOGW(LOGGING_TAG,
               "Packet has an incorrect checksum, received checksum %d, calculated checksum %d. Discarding...",
               received_checksum, calculated_checksum);
      this->receive_buffer_.erase(
          this->receive_buffer_.begin(),
          this->receive_buffer_.begin() +
              2);  // By removing the header, we're ignoring all data until the next packet header
      packet_header_found = false;
      packet_size = UINT8_MAX;
      continue;
    }

    // Check if the packet is destined for this master
    if (this->receive_buffer_.at(3) != this->master_address_) {
      ESP_LOGV(LOGGING_TAG, "Received packet for another device (%x). Discarding...", this->receive_buffer_.at(3));
      this->receive_buffer_.erase(this->receive_buffer_.begin(), this->receive_buffer_.begin() + packet_size);
      packet_header_found = false;
      packet_size = UINT8_MAX;
      continue;
    }

    // Figure out what inverter to send the packet to
    uint8_t packet_source_address = this->receive_buffer_.at(2);
    auto find_inverter_it = std::find_if(this->registered_inverters_.begin(), this->registered_inverters_.end(),
                                         [packet_source_address](aa55_inverter::AA55Inverter *inverter) {
                                           return inverter->get_slave_address() == packet_source_address;
                                         });
    if (find_inverter_it == this->registered_inverters_.end()) {
      ESP_LOGD(LOGGING_TAG, "Received packet from an unregistered inverter (%x). Discarding...", packet_source_address);
      this->receive_buffer_.erase(this->receive_buffer_.begin(), this->receive_buffer_.begin() + packet_size);
      packet_header_found = false;
      packet_size = UINT8_MAX;
      continue;
    }

    ESP_LOGD(LOGGING_TAG, "Received packet from a registered inverter (%x).", packet_source_address);

    // Pass packet to inverter object
    const std::vector<uint8_t> received_payload(this->receive_buffer_.begin() + 7,
                                                this->receive_buffer_.begin() + packet_size - 2);
    const aa55_const::AA55Packet response_packet = {packet_source_address, this->receive_buffer_.at(3),
                                                    static_cast<aa55_const::CONTROL_CODE>(this->receive_buffer_.at(4)),
                                                    static_cast<aa55_const::FUNCTION_CODE>(this->receive_buffer_.at(5)),
                                                    received_payload};
    (*find_inverter_it)->queue_response_packet(response_packet);

    this->receive_buffer_.erase(this->receive_buffer_.begin(), this->receive_buffer_.begin() + packet_size);
    packet_header_found = false;
    packet_size = UINT8_MAX;
  }
}
}  // namespace aa55_bus
}  // namespace esphome
