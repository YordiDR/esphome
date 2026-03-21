#include "esphome/core/log.h"
#include "goodwe_aa55.h"
#include "const.h"
#include <vector>
#include <deque>
#include <iterator>
#include <cmath>
#include <string>

namespace esphome {
namespace goodwe_aa55 {

GoodweAA55::GoodweAA55(std::string serial_number, uint8_t slave_address, uint8_t master_address,
                       uint32_t update_interval) {
  serial_number_ = serial_number;
  slave_address_ = slave_address;
  master_address_ = master_address;
  update_interval_ = update_interval;
}

void GoodweAA55::setup() {
  // Mark all sensors as unavailable
  for (GoodweAA55Sensor *sensor : this->sensors_) {
    sensor->publish_state(NAN);
  }
  for (GoodweAA55TextSensor *sensor : this->text_sensors_) {
    sensor->publish_state("");
  }

  // Send deregister command to inverter address at ESP startup so we can register it again
  std::vector<uint8_t> packet = HEADERS;  // Initialize message with AA55 header, then add command details
  packet.push_back(this->master_address_);
  packet.push_back(this->slave_address_);
  packet.push_back((uint8_t) CONTROL_CODE::REGISTER);
  packet.push_back((uint8_t) FUNCTION_CODE::REMOVE_REG);
  packet.push_back(0x00);
  std::vector<uint8_t> checksum = this->calculate_checksum(packet);  // Calculate checksum
  packet.push_back(checksum.at(0));
  packet.push_back(checksum.at(1));
  ESP_LOGD(LOGGING_TAG, "Sending message %s", this->create_hex_string(packet).c_str());
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
  this->loop_counter_++;

  if (this->loop_counter_ < 1000) {
    return;
  }

  this->loop_counter_ = 0;
  // Work to be done at each update interval
  std::vector<uint8_t> packet = HEADERS;  // Initialize message with AA55 header, then add command details
  packet.push_back(this->master_address_);
  packet.push_back(0x7f);
  packet.push_back((uint8_t) CONTROL_CODE::READ);
  packet.push_back((uint8_t) FUNCTION_CODE::QUERY_RUN_INFO);
  packet.push_back(0x00);
  std::vector<uint8_t> checksum = this->calculate_checksum(packet);  // Calculate checksum
  packet.push_back(checksum.at(0));
  packet.push_back(checksum.at(1));
  ESP_LOGD(LOGGING_TAG, "Sending message %s", this->create_hex_string(packet).c_str());
  this->write_array(packet);  // Send query running info command to inverter

  // Receive response
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
      packet_fully_received = true;

      // Strip bytes received after the packet
      if (buffer_deque.size() > packet_size) {
        ESP_LOGV(LOGGING_TAG, "Removing %d trailing bytes from buffer_deque...", buffer_deque.size() - packet_size);
        buffer_deque.erase(buffer_deque.begin() + packet_size, buffer_deque.end());
      }
    }
  }

  if (!packet_fully_received) {
    ESP_LOGI(LOGGING_TAG, "Failed to receive response packet from inverter.");
    if (this->inverter_online_) {
      this->inverter_offline_countdown_--;
      if (this->inverter_offline_countdown_ == 0) {
        ESP_LOGI(LOGGING_TAG, "Considering inverter offline due to countdown.");
        this->inverter_online_ = false;
      }
    }
    return;
  }

  if (!this->inverter_online_) {
    this->inverter_online_ = true;
    this->inverter_offline_countdown_ = INVERTER_OFFLINE_COUNTDOWN_RESET;
  }

  ESP_LOGD(LOGGING_TAG, "Parsing response packet %s (%d bytes)", this->create_hex_string(buffer_deque).c_str(),
           buffer_deque.size());

  ESP_LOGD(LOGGING_TAG, "Verifying packet checksum...");
  std::vector<uint8_t> received_crc_bytes = std::vector<uint8_t>(buffer_deque.end() - 2, buffer_deque.end());
  buffer_deque.erase(buffer_deque.end() - 2, buffer_deque.end());
  std::vector<uint8_t> calculated_crc_bytes = this->calculate_checksum(buffer_deque);

  if (received_crc_bytes != calculated_crc_bytes) {
    ESP_LOGW(LOGGING_TAG, "Packet has an incorrect checksum, ignoring...");
  }

  ESP_LOGD(LOGGING_TAG, "Packet checksum is correct. Parsing headers...");
  ESP_LOGD(LOGGING_TAG,
           "Verifying that the packet is destined for this device (my address: %x, packet destination address: %x)...",
           this->master_address_, buffer_deque.at(3));

  if (this->master_address_ != buffer_deque.at(3)) {
    ESP_LOGD(LOGGING_TAG, "Received packet for another device. Skipping processing...");
    return;
  }

  std::vector<uint8_t> packet_payload =
      std::vector<uint8_t>(buffer_deque.begin() + 7, buffer_deque.end());  // Remove first 7 bytes (headers)
  this->parse_data(packet_payload);
}

void GoodweAA55::update() {
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

    return;
  }

  // Publish most recent sensor values if applicable
  for (GoodweAA55Sensor *sensor : this->sensors_) {
    if (sensor->time_to_update()) {
      if (sensor->get_accuracy_decimals() > 0) {
        sensor->publish_state(sensor->newest_value /
                              std::pow(10.0, (float) sensor->get_accuracy_decimals()));  // Apply decimal precision
      } else {
        sensor->publish_state(sensor->newest_value);
      }
      if (sensor->get_skip_updates() != 0) {
        sensor->reset_skipped_updates();
      }
    } else {
      sensor->increment_skipped_updates();
    }
  }
  for (GoodweAA55TextSensor *sensor : this->text_sensors_) {
    if (sensor->time_to_update()) {
      sensor->publish_state(sensor->newest_value);
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

void GoodweAA55::parse_data(const std::vector<uint8_t> &payload) {
  ESP_LOGD(LOGGING_TAG, "Parsing packet payload...");

  // During boot, sometimes the inverter returns an all 0 payload to the read command.
  // By checking if the E-total value is 0, we discard these responses.
  if (this->parse_int(payload, (uint8_t) SENSOR_PAYLOAD_LOCATION::E_TOTAL, (uint8_t) SENSOR_PAYLOAD_LENGTH::E_TOTAL) ==
      0) {
    ESP_LOGI(LOGGING_TAG, "Received read response with all 0 payload. Discarding response...");
    return;
  }

  // Save received values in the sensor attributes
  for (GoodweAA55Sensor *sensor : this->sensors_) {
    ESP_LOGV(LOGGING_TAG, "Parsing %s from payload[%d], length %d bytes.", sensor->get_id().c_str(),
             sensor->get_payload_location(), sensor->get_payload_length());
    sensor->newest_value = this->parse_int(payload, sensor->get_payload_location(), sensor->get_payload_length());
    ESP_LOGV(LOGGING_TAG, "Parsed %s: %f", sensor->get_id().c_str(), sensor->newest_value);
  }

  for (GoodweAA55TextSensor *sensor : this->text_sensors_) {
    ESP_LOGV(LOGGING_TAG, "Parsing %s from payload[%d], length %d bytes.", sensor->get_id().c_str(),
             sensor->get_payload_location(), sensor->get_payload_length());
    sensor->newest_value_code = this->parse_int(payload, sensor->get_payload_location(), sensor->get_payload_length());
    sensor->map_code_to_string();
    ESP_LOGV(LOGGING_TAG, "Parsed %s: %d -> %s", sensor->get_id().c_str(), sensor->newest_value_code,
             sensor->newest_value.c_str());
  }
}

uint32_t GoodweAA55::parse_int(const std::vector<uint8_t> &message, uint8_t start, uint8_t bytes) {
  uint32_t response = 0;
  switch (bytes) {
    case 2:
      response |= message.at(start) << 8;
      response |= message.at(start + 1);
      break;
    case 4:
      response |= message.at(start) << 24;
      response |= message.at(start + 1) << 16;
      response |= message.at(start + 2) << 8;
      response |= message.at(start + 3);
      break;
    default:
      ESP_LOGE(LOGGING_TAG, "Received incorrect value for bytes parameter in GoodweAA55::parse_int. Value: %d", bytes);
      return 0;
  }

  return response;
}
}  // namespace goodwe_aa55
}  // namespace esphome
