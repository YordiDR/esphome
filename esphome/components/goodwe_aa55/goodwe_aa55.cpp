#include "esphome/core/log.h"
#include "goodwe_aa55.h"
#include "const.h"
#include <vector>
#include <deque>
#include <iterator>
#include <cmath>
#include <string>
#include <algorithm>
#include <chrono>  // to remove after aa55 bus component implementation
#include <thread>  // to remove after aa55 bus component implementation

namespace esphome {
namespace goodwe_aa55 {

GoodweAA55::GoodweAA55(std::string serial_number, uint8_t slave_address, uint8_t master_address) {
  serial_number_ = serial_number;
  slave_address_ = slave_address;
  master_address_ = master_address;
}

void GoodweAA55::setup() {
  // Mark all sensors as unavailable
  for (GoodweAA55Sensor *sensor : this->sensors_) {
    sensor->publish_state(NAN);
  }
  for (GoodweAA55TextSensor *sensor : this->text_sensors_) {
    sensor->publish_state("");
  }

  // Send deregister command to inverter at ESP startup so we can register it again
  this->send_packet(this->slave_address_, CONTROL_CODE::REGISTER, FUNCTION_CODE::REMOVE_REG, EMPTY_VECTOR);
}

void GoodweAA55::dump_config() {
  ESP_LOGCONFIG(LOGGING_TAG, "Goodwe AA55 component");
  ESP_LOGCONFIG(LOGGING_TAG, "  Serial number: %s", this->serial_number_.c_str());
  ESP_LOGCONFIG(LOGGING_TAG, "  Slave address: %x", this->slave_address_);
  ESP_LOGCONFIG(LOGGING_TAG, "  Master address: %x", this->master_address_);
  ESP_LOGCONFIG(LOGGING_TAG, "  Update interval: %d", this->update_interval_);
}

void GoodweAA55::loop() {
  // TODO If inverter is unregistered, send out offline requests to discover when it comes online, process registration
  // TODO send register command every 10s
}

void GoodweAA55::update() {
  // Get updated running info from inverter
  this->send_packet(DEFAULT_ADDRESS, CONTROL_CODE::READ, FUNCTION_CODE::QUERY_RUN_INFO, EMPTY_VECTOR);
  std::vector<uint8_t> response_payload =
      this->await_packet(DEFAULT_ADDRESS, CONTROL_CODE::READ, FUNCTION_CODE::RUN_INFO_RESPONSE);

  if (response_payload == EMPTY_VECTOR) {
    return;
  }

  // Parse updated sensor info
  this->parse_run_info_response(response_payload);

  std::this_thread::sleep_for(std::chrono::milliseconds(450));
  // Get updated ID info from inverter
  this->send_packet(DEFAULT_ADDRESS, CONTROL_CODE::READ, FUNCTION_CODE::QUERY_ID_INFO, EMPTY_VECTOR);
  response_payload = this->await_packet(DEFAULT_ADDRESS, CONTROL_CODE::READ, FUNCTION_CODE::ID_INFO_RESPONSE);

  if (response_payload == EMPTY_VECTOR) {
    return;
  }

  // Parse updated sensor info
  this->parse_id_info_response(response_payload);

  if (!this->inverter_online_) {
    // Set all sensors to an unknown state
    for (GoodweAA55Sensor *sensor : this->sensors_) {
      sensor->publish_state(NAN);
    }

    // Set all text sensors to an empty string besides WORK_MODE if it is defined
    for (GoodweAA55TextSensor *sensor : this->text_sensors_) {
      if (sensor->get_type() == SENSOR_TYPE::WORK_MODE) {
        sensor->publish_state("Offline");
      } else {
        sensor->publish_state("");
      }
    }
  }

  // Publish most recent sensor values if applicable
  for (GoodweAA55Sensor *sensor : this->sensors_) {
    if (sensor->time_to_update()) {
      sensor->publish_state(sensor->get_newest_value());
      if (sensor->get_skip_updates() != 0) {
        sensor->reset_skipped_updates();
      }
    } else {
      sensor->increment_skipped_updates();
    }
  }
  for (GoodweAA55TextSensor *sensor : this->text_sensors_) {
    if (sensor->time_to_update()) {
      sensor->publish_state(sensor->get_newest_value());
      if (sensor->get_skip_updates() != 0) {
        sensor->reset_skipped_updates();
      }
    } else {
      sensor->increment_skipped_updates();
    }
  }
}

void GoodweAA55::add_sensor(GoodweAA55Sensor *sensor) { this->sensors_.push_back(sensor); }

void GoodweAA55::add_text_sensor(GoodweAA55TextSensor *sensor) { this->text_sensors_.push_back(sensor); }

void GoodweAA55::parse_run_info_response(const std::vector<uint8_t> &payload) {
  ESP_LOGD(LOGGING_TAG, "Parsing run info response payload %s (%d bytes)", this->create_hex_string(payload).c_str(),
           payload.size());
  ESP_LOGD(LOGGING_TAG, "Parsing packet payload...");

  // During boot, sometimes the inverter returns an all 0 payload to the read command.
  bool all_zeroes = std::all_of(payload.begin(), payload.end(), [](int i) { return i == 0; });

  if (all_zeroes) {
    ESP_LOGI(LOGGING_TAG, "Received read response with all 0 payload. Discarding response...");
    return;
  }

  // Save received values in the sensor attributes
  for (GoodweAA55Sensor *sensor : this->sensors_) {
    if (MAP_SENSOR_PAYLOAD_SOURCE.at(sensor->get_type()) == FUNCTION_CODE::RUN_INFO_RESPONSE) {
      ESP_LOGV(LOGGING_TAG, "Parsing %s from payload[%d], length %d bytes.", sensor->get_id().c_str(),
               sensor->get_payload_location(), sensor->get_payload_length());
      sensor->parse_payload(payload);
      ESP_LOGV(LOGGING_TAG, "Parsed %s: %f", sensor->get_id().c_str(), sensor->get_newest_value());
    }
  }

  for (GoodweAA55TextSensor *sensor : this->text_sensors_) {
    if (MAP_SENSOR_PAYLOAD_SOURCE.at(sensor->get_type()) == FUNCTION_CODE::RUN_INFO_RESPONSE) {
      ESP_LOGV(LOGGING_TAG, "Parsing %s from payload[%d], length %d bytes.", sensor->get_id().c_str(),
               sensor->get_payload_location(), sensor->get_payload_length());
      sensor->parse_payload(payload);
      ESP_LOGV(LOGGING_TAG, "Parsed %s: %s", sensor->get_id().c_str(), sensor->get_newest_value().c_str());
    }
  }
}

void GoodweAA55::parse_id_info_response(const std::vector<uint8_t> &payload) {
  ESP_LOGD(LOGGING_TAG, "Parsing ID info response payload %s (%d bytes)", this->create_hex_string(payload).c_str(),
           payload.size());
  ESP_LOGD(LOGGING_TAG, "Parsing packet payload...");

  // During boot, sometimes the inverter returns an all 0 payload to the read command.
  bool all_zeroes = std::all_of(payload.begin(), payload.end(), [](int i) { return i == 0; });

  if (all_zeroes) {
    ESP_LOGI(LOGGING_TAG, "Received read ID info response with all 0 payload. Discarding response...");
    return;
  }

  // Save received values in the sensor attributes
  for (GoodweAA55Sensor *sensor : this->sensors_) {
    if (MAP_SENSOR_PAYLOAD_SOURCE.at(sensor->get_type()) == FUNCTION_CODE::ID_INFO_RESPONSE) {
      ESP_LOGV(LOGGING_TAG, "Parsing %s from payload[%d], length %d bytes.", sensor->get_id().c_str(),
               sensor->get_payload_location(), sensor->get_payload_length());
      sensor->parse_payload(payload);
      ESP_LOGV(LOGGING_TAG, "Parsed %s: %f", sensor->get_id().c_str(), sensor->get_newest_value());
    }
  }

  for (GoodweAA55TextSensor *sensor : this->text_sensors_) {
    if (MAP_SENSOR_PAYLOAD_SOURCE.at(sensor->get_type()) == FUNCTION_CODE::ID_INFO_RESPONSE) {
      ESP_LOGV(LOGGING_TAG, "Parsing %s from payload[%d], length %d bytes.", sensor->get_id().c_str(),
               sensor->get_payload_location(), sensor->get_payload_length());
      sensor->parse_payload(payload);
      ESP_LOGV(LOGGING_TAG, "Parsed %s: %s", sensor->get_id().c_str(), sensor->get_newest_value().c_str());
    }
  }
}

void GoodweAA55::send_packet(uint8_t destination_address, CONTROL_CODE control_code, FUNCTION_CODE function_code,
                             const std::vector<uint8_t> &payload) {
  std::vector<uint8_t> packet = HEADERS;  // Initialize message with AA55 header, then add command details
  packet.push_back(this->master_address_);
  packet.push_back(destination_address);
  packet.push_back((uint8_t) control_code);
  packet.push_back((uint8_t) function_code);
  packet.push_back(payload.size());
  if (payload != EMPTY_VECTOR) {
    packet.insert(packet.end(), payload.begin(), payload.end());
  }
  const std::vector<uint8_t> checksum = this->calculate_checksum(packet);
  packet.insert(packet.end(), checksum.begin(), checksum.end());
  ESP_LOGD(LOGGING_TAG, "Sending packet %s", this->create_hex_string(packet).c_str());
  this->write_array(packet);  // Send packet over UART
}

std::vector<uint8_t> GoodweAA55::await_packet(uint8_t expected_source_address, CONTROL_CODE expected_control_code,
                                              FUNCTION_CODE expected_function_code) {
  const uint8_t buffer_max_size{64};
  bool packet_header_found{false};
  int packet_size{-1};
  bool packet_fully_received{false};
  std::deque<uint8_t> buffer_deque;

  while (this->available() && !packet_fully_received && buffer_deque.size() < MAX_BUFFER_LENGTH) {
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
        if (buffer_deque.at(i) == 0xaa && buffer_deque.at(i + 1) == 0x55) {
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
      if (buffer_deque.at(2) == expected_source_address && buffer_deque.at(3) == this->master_address_ &&
          buffer_deque.at(4) == (uint8_t) expected_control_code &&
          buffer_deque.at(5) == (uint8_t) expected_function_code) {
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
    if (this->inverter_online_) {
      this->inverter_offline_countdown_--;
      if (this->inverter_offline_countdown_ == 0) {
        ESP_LOGI(LOGGING_TAG, "Considering inverter offline due to countdown.");
        this->inverter_online_ = false;
      }
    }
    return EMPTY_VECTOR;
  }

  // Mark inverter as online if it was previously online since we succesfully received a response
  if (!this->inverter_online_) {
    this->inverter_online_ = true;
    this->inverter_offline_countdown_ = INVERTER_OFFLINE_COUNTDOWN_RESET;
  }

  return std::vector<uint8_t>(buffer_deque.begin() + 7, buffer_deque.begin() + packet_size - 2);  // Return payload
}

std::vector<uint8_t> GoodweAA55::calculate_checksum(const std::vector<uint8_t> &packet) {
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
}  // namespace goodwe_aa55
}  // namespace esphome
