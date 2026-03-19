#include "esphome/core/log.h"
#include "goodwe_aa55.h"
#include "const.h"
#include <iomanip>
#include <sstream>
#include <vector>
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
  std::vector<uint8_t> message = HEADERS;  // Initialize message with AA55 header, then add command details
  message.push_back(this->master_address_);
  message.push_back(this->slave_address_);
  message.push_back((uint8_t) CONTROL_CODE::REGISTER);
  message.push_back((uint8_t) FUNCTION_CODE::REMOVE_REG);
  message.push_back(0x00);
  this->add_checksum(message);  // Calculate & add checksum
  ESP_LOGD(LOGGING_TAG, "Sending message %s", this->create_hex_string(message));
}

void GoodweAA55::dump_config() {
  ESP_LOGCONFIG(LOGGING_TAG, "Goodwe AA55 component");
  ESP_LOGCONFIG(LOGGING_TAG, "  Serial number: %s", this->serial_number_);
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
  uint8_t buffer_byte, message_length = MAX_LINE_LENGTH;  // Initialize bytes to read as maximum AA55 message length
  this->receive_buffer_.clear();
  std::vector<uint8_t> message = HEADERS;  // Initialize message with AA55 header, then add command details
  message.push_back(this->master_address_);
  message.push_back(0x7f);
  message.push_back((uint8_t) CONTROL_CODE::READ);
  message.push_back((uint8_t) FUNCTION_CODE::QUERY_RUN_INFO);
  message.push_back(0x00);
  this->add_checksum(message);  // Calculate & add checksum
  ESP_LOGD(LOGGING_TAG, "Sending message %s", this->create_hex_string(message));

  this->write_array(message);  // Send query running info command to inverter
  // Read the response from the device, up to MAX_LINE_LENGTH bytes
  while (this->available() && this->receive_buffer_.size() < message_length) {
    this->read_byte(&buffer_byte);

    if (this->receive_buffer_.size() == 6) {  // 7th byte is payload size
      message_length =
          9 + buffer_byte;  // Calculate total message size = AA55 header + source address + destination address +
                            // control code + function code + payload size byte + CRC + payload size
    }
    this->receive_buffer_.push_back(buffer_byte);
  }

  if (this->receive_buffer_.size() > 0) {
    // Reset inverter offline counter & flag since inverter is online
    if (!this->inverter_online_) {
      this->inverter_online_ = true;
      this->inverter_offline_countdown_ = INVERTER_OFFLINE_COUNTDOWN_RESET;
    }
    this->parse_data();
  } else {
    ESP_LOGI(LOGGING_TAG, "No response received from inverter");
    if (this->inverter_online_) {
      this->inverter_offline_countdown_--;
      if (this->inverter_offline_countdown_ == 0) {
        ESP_LOGI(LOGGING_TAG, "Considering inverter offline due to countdown.");
        this->inverter_online_ = false;
      }
    }
  }
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

void GoodweAA55::parse_data() {
  // Example parsing method
  // Translates data received into buffer_data_ and stores it in parsed_value_ for
  ESP_LOGD(LOGGING_TAG, "Parsing response %s (%d bytes)", this->create_hex_string(this->receive_buffer_),
           this->receive_buffer_.size());

  ESP_LOGD(LOGGING_TAG, "Verifying received checksum...");
  if (!this->verify_checksum(this->receive_buffer_)) {  // CRC checksum is removed in this function
    ESP_LOGW(LOGGING_TAG, "Response has an incorrect checksum, ignoring...");
    return;
  }

  ESP_LOGD(LOGGING_TAG, "Message checksum is correct. Parsing headers...");
  ESP_LOGD(LOGGING_TAG,
           "Verifying that the packet is destined for me (my address: %x, packet destination address: %x)...",
           this->master_address_, this->receive_buffer_.at(3));

  if (this->master_address_ != this->receive_buffer_.at(3)) {
    ESP_LOGD(LOGGING_TAG, "Received packet for another device. Skipping processing...");
    return;
  }

  ESP_LOGD(LOGGING_TAG, "Received packet, parsing payload...");

  // During boot, sometimes the inverter returns an all 0 payload to the read command.
  // By checking if the E-total value is 0, we discard these responses.
  if (this->parse_int(this->receive_buffer_, 31, 4) == 0) {
    ESP_LOGI(LOGGING_TAG, "Received read response with all 0 payload. Discarding response...");
    return;
  }

  // Save received values in the sensor attributes
  for (GoodweAA55Sensor *sensor : this->sensors_) {
    sensor->newest_value =
        this->parse_int(this->receive_buffer_, sensor->get_payload_location(), sensor->get_payload_length());
    ESP_LOGV(LOGGING_TAG, "Parsed %s: %f", sensor->get_id(), sensor->newest_value);
  }

  for (GoodweAA55TextSensor *sensor : this->text_sensors_) {
    sensor->newest_value_code =
        this->parse_int(this->receive_buffer_, sensor->get_payload_location(), sensor->get_payload_length());
    sensor->map_code_to_string();
    ESP_LOGV(LOGGING_TAG, "Parsed %s: %d -> %s", sensor->get_id(), sensor->newest_value_code, sensor->newest_value);
  }
}

void GoodweAA55::add_checksum(std::vector<uint8_t> &message) {
  uint16_t crc = 0;
  ESP_LOGD(LOGGING_TAG, "Calculating CRC for message '%s'...", this->create_hex_string(message));
  for (uint8_t byte : message) {
    ESP_LOGV(LOGGING_TAG, "Checksum calculation: adding value %x to current CRC value (%d)", byte, crc);
    crc += byte;
  }

  ESP_LOGD(LOGGING_TAG, "Calculated CRC value: %d, {%x, %x}", crc, (uint8_t) (crc >> 8), (uint8_t) crc);
  message.push_back((uint8_t) (crc >> 8));
  message.push_back((uint8_t) crc);
}

bool GoodweAA55::verify_checksum(std::vector<uint8_t> &message) {
  // Save & remove CRC bytes from message
  const uint8_t crc_received_low_byte = message.back();
  message.pop_back();
  const uint8_t crc_received_high_byte = message.back();
  message.pop_back();

  // Calculate CRC for message
  this->add_checksum(message);

  const uint8_t crc_calculated_low_byte = message.back();
  message.pop_back();
  const uint8_t crc_calculated_high_byte = message.back();
  message.pop_back();

  // Check if calculated CRC matches received CRC
  ESP_LOGD(LOGGING_TAG, "Checking if CRC for received message is correct (calculated CRC: %x%x, received CRC: %x%x)",
           crc_calculated_high_byte, crc_calculated_low_byte, crc_received_high_byte, crc_received_low_byte);
  return (crc_calculated_high_byte == crc_received_high_byte && crc_calculated_low_byte == crc_received_low_byte);
}

std::string GoodweAA55::create_hex_string(std::vector<uint8_t> &data) {
  std::stringstream ss;
  ss << std::hex;

  for (uint8_t byte : data) {
    ss << std::setw(2) << std::setfill('0') << byte;
  }

  return ss.str();
}

uint32_t GoodweAA55::parse_int(std::vector<uint8_t> message, uint8_t start, uint8_t bytes) {
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
      return 0.0;
  }

  return response;
}
}  // namespace goodwe_aa55
}  // namespace esphome
