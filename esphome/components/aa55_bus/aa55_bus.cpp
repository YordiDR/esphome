#include "esphome/core/log.h"
#include "aa55_bus.h"
#include <deque>
#include <iterator>
#include <cmath>
#include <algorithm>

namespace esphome {
namespace aa55_bus {

AA55Bus::AA55Bus(uint8_t master_address) { master_address_ = master_address; }

void AA55Bus::setup() {}

void AA55Bus::dump_config() {
  ESP_LOGCONFIG(LOGGING_TAG, "Goodwe AA55 Bus component");
  ESP_LOGCONFIG(LOGGING_TAG, "  Bus ID: %s", this->id_.c_str());
  ESP_LOGCONFIG(LOGGING_TAG, "  Master address: %x", this->master_address_);
  ESP_LOGCONFIG(LOGGING_TAG, "  Update interval: %d", this->update_interval_);
}

void AA55Bus::loop() {}

void AA55Bus::update() {}

void AA55Bus::send_packet(const aa55_const::AA55Command &command) {
  std::vector<uint8_t> packet = aa55_const::HEADERS;  // Initialize message with AA55 header, then add command details
  packet.push_back(command.source_address);
  packet.push_back(command.destination_address);
  packet.push_back((uint8_t) command.control_code);
  packet.push_back((uint8_t) command.function_code);
  packet.push_back(command.payload.size());
  if (command.payload != aa55_const::EMPTY_VECTOR) {
    packet.insert(packet.end(), command.payload.begin(), command.payload.end());
  }
  const std::vector<uint8_t> checksum = this->calculate_checksum(packet);
  packet.insert(packet.end(), checksum.begin(), checksum.end());
  ESP_LOGD(LOGGING_TAG, "Sending packet %s", this->create_hex_string(packet).c_str());
  this->write_array(packet);  // Send packet over UART
}

std::vector<uint8_t> AA55Bus::await_response(const aa55_const::AA55Command &command) {
  const uint8_t buffer_max_size{64};
  bool packet_header_found{false};
  int packet_size{-1};
  bool packet_fully_received{false};
  std::deque<uint8_t> buffer_deque;

  while (this->available() && !packet_fully_received && buffer_deque.size() < aa55_const::MAX_BUFFER_LENGTH) {
    // Read all available bytes in batches to reduce UART call overhead.
    uint8_t avail = this->available();
    size_t to_read = std::min(avail, buffer_max_size);
    uint8_t buffer_array[to_read];
    ESP_LOGV(LOGGING_TAG, "%d bytes are available from UART, max buffer size is %d, reading %d bytes...", avail,
             buffer_max_size, to_read);
    if (!this->read_array(buffer_array, to_read)) {
      break;
    }

    // Add received bytes in the buffer array to the deque
    buffer_deque.insert(buffer_deque.end(), buffer_array, buffer_array + sizeof(buffer_array));
    ESP_LOGV(LOGGING_TAG, "Updated buffer_deque contents: %s", this->create_hex_string(buffer_deque).c_str());

    // Find header index if it is still unknown
    if (!packet_header_found) {
      ESP_LOGV(LOGGING_TAG, "Looking for header in buffer_deque...");
      // Look for the header in the newly added indices
      for (size_t i = buffer_deque.size() - to_read; i < buffer_deque.size() - 1; i++) {
        ESP_LOGV(LOGGING_TAG, "Checking if header starts at buffer_deque[%d] (= %x)", i, buffer_deque.at(i));
        if (buffer_deque.at(i) == 0xAA && buffer_deque.at(i + 1) == 0x55) {
          ESP_LOGV(LOGGING_TAG, "Found header at buffer_deque[%d].", i);
          // Strip bytes received before the packet
          if (i != 0) {
            ESP_LOGV(LOGGING_TAG, "Stripping %d bytes before header from deque", i);
            buffer_deque.erase(buffer_deque.begin(), buffer_deque.begin() + i);
            ESP_LOGV(LOGGING_TAG, "New buffer_deque contents: %s", this->create_hex_string(buffer_deque).c_str());
          }

          packet_header_found = true;
          break;
        }
      }

      if (!packet_header_found) {
        ESP_LOGV(LOGGING_TAG, "Could not yet find AA55 header in buffer_deque.");
      }
    }

    // Find packet size if it is still unknown
    if (packet_header_found && packet_size == -1) {
      ESP_LOGV(LOGGING_TAG, "Checking if buffer_deque contains packet size byte...");
      if (buffer_deque.size() >= 7) {
        ESP_LOGV(LOGGING_TAG, "Found payload size byte, value: %d..", buffer_deque.at(6));
        packet_size = 9 + buffer_deque.at(6);  // Packet total size = AA55 header + source address + destination
                                               // address + control code + function code + payload size + CRC +
                                               // payload size. Everything except payload = 9 bytes
      } else {
        ESP_LOGV(LOGGING_TAG, "Buffer_deque does not yet contain packet size byte...");
      }
    }

    // Check if packet is fully received
    if (packet_size != -1 && buffer_deque.size() >= packet_size) {
      ESP_LOGD(LOGGING_TAG, "Packet of %d bytes was fully received from UART.", packet_size);
      ESP_LOGD(LOGGING_TAG, "Verifying received packet checksum...");
      std::vector<uint8_t> calculated_crc_bytes = this->calculate_checksum(std::vector<uint8_t>(
          buffer_deque.begin(),
          buffer_deque.begin() + packet_size - 2));  // Calculate checksum of received packet without received CRC bytes

      if (buffer_deque.at(packet_size - 2) != calculated_crc_bytes.at(0) ||
          buffer_deque.at(packet_size - 1) != calculated_crc_bytes.at(1)) {
        ESP_LOGW(LOGGING_TAG, "Packet has an incorrect checksum, removing from buffer...");
        buffer_deque.erase(buffer_deque.begin(), buffer_deque.begin() + packet_size);
        packet_header_found = false;
        packet_size = -1;
        continue;
      }

      // Check if the packet is what we expect, if not, drop it and continue looking for the response
      if (buffer_deque.at(2) == command.destination_address && buffer_deque.at(3) == command.source_address &&
          buffer_deque.at(4) == (uint8_t) command.control_code &&
          buffer_deque.at(5) == (uint8_t) command.function_code + 128) {
        packet_fully_received = true;
        ESP_LOGD(LOGGING_TAG, "Received packet with expected headers");
      } else {
        ESP_LOGD(LOGGING_TAG, "Received packet which is not what we expect. Received packet with headers: %s",
                 this->create_hex_string(std::vector<uint8_t>(buffer_deque.begin(), buffer_deque.begin() + 6)).c_str());
        ESP_LOGD(LOGGING_TAG, "Removing unexpected packet from buffer...");
        buffer_deque.erase(buffer_deque.begin(), buffer_deque.begin() + packet_size);
        packet_header_found = false;
        packet_size = -1;
      }
    }
  }

  if (!packet_fully_received) {
    ESP_LOGI(LOGGING_TAG, "Failed to receive response packet");
    return aa55_const::EMPTY_VECTOR;
  }

  return std::vector<uint8_t>(buffer_deque.begin() + 7, buffer_deque.begin() + packet_size - 2);  // Return payload
}

std::vector<uint8_t> AA55Bus::calculate_checksum(const std::vector<uint8_t> &packet) {
  uint16_t crc = 0;
  ESP_LOGD(LOGGING_TAG, "Calculating CRC for packet '%s'...", this->create_hex_string(packet).c_str());
  for (uint8_t byte : packet) {
    ESP_LOGV(LOGGING_TAG, "Checksum calculation: adding value %x to current CRC value (%d)", byte, crc);
    crc += byte;
  }

  ESP_LOGD(LOGGING_TAG, "Calculated CRC value: %d, {%x, %x}", crc, (uint8_t) (crc >> 8), (uint8_t) crc);
  const std::vector<uint8_t> crc_bytes{(uint8_t) (crc >> 8), (uint8_t) crc};
  return crc_bytes;
}

void AA55Bus::drain_uart_rx_buffer() {
  uint8_t buf[64];
  size_t avail;
  while ((avail = this->available()) > 0) {
    if (!this->read_array(buf, std::min(avail, sizeof(buf)))) {
      break;
    }
  }
}
}  // namespace aa55_bus
}  // namespace esphome
