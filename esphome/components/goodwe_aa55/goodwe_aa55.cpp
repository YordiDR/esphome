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
#define GOODWE_AA55_SET_SENSOR_UNAVAILABLE(s) this->s_##s##_->publish_state(NAN);
  GOODWE_AA55_SENSOR_LIST(GOODWE_AA55_SET_SENSOR_UNAVAILABLE, )
  this->s_work_mode_->publish_state("Ofline");
  this->s_error_codes_->publish_state("");

  // Send deregister command to inverter address at ESP startup so we can register it again
  std::vector<uint8_t> message = HEADERS;  // Initialize message with AA55 header, then add command details
  message.push_back(master_address_);
  message.push_back(slave_address_);
  message.push_back((uint8_t) CONTROL_CODE::REGISTER);
  message.push_back((uint8_t) FUNCTION_CODE::REMOVE_REG);
  message.push_back(0x00);
  this->add_checksum(message);  // Calculate & add checksum
  ESP_LOGD(LOGGING_TAG, "Sending message %s", this->create_hex_string(message));
}

void GoodweAA55::dump_config() {
  ESP_LOGCONFIG(LOGGING_TAG, "Goodwe AA55 component");
  ESP_LOGCONFIG(LOGGING_TAG, "  Serial number: %s", serial_number_);
  ESP_LOGCONFIG(LOGGING_TAG, "  Slave address: %x", slave_address_);
  ESP_LOGCONFIG(LOGGING_TAG, "  Master address: %x", master_address_);
  ESP_LOGCONFIG(LOGGING_TAG, "  Update interval: %d", update_interval_);
}

void GoodweAA55::loop() {
  // TODO If inverter is unregistered, send out offline requests to discover when it comes online, process registration
  loop_counter_++;

  if (loop_counter_ < 1000) {
    return;
  }

  loop_counter_ = 0;
  // Work to be done at each update interval
  uint8_t buffer_byte, message_length = MAX_LINE_LENGTH;  // Initialize bytes to read as maximum AA55 message length
  receive_buffer_.clear();
  std::vector<uint8_t> message = HEADERS;  // Initialize message with AA55 header, then add command details
  message.push_back(master_address_);
  message.push_back(0x7f);
  message.push_back((uint8_t) CONTROL_CODE::READ);
  message.push_back((uint8_t) FUNCTION_CODE::QUERY_RUN_INFO);
  message.push_back(0x00);
  this->add_checksum(message);  // Calculate & add checksum
  ESP_LOGD(LOGGING_TAG, "Sending message %s", this->create_hex_string(message));

  this->write_array(message);  // Send query running info command to inverter
  // Read the response from the device, up to MAX_LINE_LENGTH bytes
  while (this->available() && receive_buffer_.size() < message_length) {
    this->read_byte(&buffer_byte);

    if (receive_buffer_.size() == 6) {  // 7th byte is payload size
      message_length =
          9 + buffer_byte;  // Calculate total message size = AA55 header + source address + destination address +
                            // control code + function code + payload size byte + CRC + payload size
    }
    receive_buffer_.push_back(buffer_byte);
  }

  if (receive_buffer_.size() > 0) {
    // Reset inverter offline counter & flag since inverter is online
    if (!inverter_online_) {
      inverter_online_ = true;
      inverter_offline_countdown_ = INVERTER_OFFLINE_COUNTDOWN_RESET;
    }
    this->parse_data();
  } else {
    ESP_LOGI(LOGGING_TAG, "No response received from inverter");
    if (inverter_online_) {
      inverter_offline_countdown_--;
      if (inverter_offline_countdown_ == 0) {
        ESP_LOGI(LOGGING_TAG, "Considering inverter offline due to countdown.");
        inverter_online_ = false;
      }
    }
  }
}

void GoodweAA55::update() {
  if (this->inverter_online_) {
#define GOODWE_AA55_PUBLISH_SENSOR_STATE(s) \
  if (s_##s##_->time_to_update()) { \
    this->s_##s##_->publish_state(this->v_##s##_); \
    if (s_##s##_->get_skip_updates() != 0) { \
      this->s_##s##_->reset_skipped_updates(); \
    } \
  } else { \
    this->s_##s##_->increment_skipped_updates(); \
  }
    GOODWE_AA55_SENSOR_LIST(GOODWE_AA55_PUBLISH_SENSOR_STATE, )
#define GOODWE_AA55_PUBLISH_TEXT_SENSOR_STATE(s) \
  if (s_##s##_->time_to_update()) { \
    this->s_##s##_->publish_state(this->v_##s##_); \
    if (s_##s##_->get_skip_updates() != 0) { \
      this->s_##s##_->reset_skipped_updates(); \
    } \
  } else { \
    this->s_##s##_->increment_skipped_updates(); \
  }
    GOODWE_AA55_TEXT_SENSOR_LIST(GOODWE_AA55_PUBLISH_TEXT_SENSOR_STATE, )
  } else {
#define GOODWE_AA55_SET_SENSOR_UNAVAILABLE(s) this->s_##s##_->publish_state(NAN);
    GOODWE_AA55_SENSOR_LIST(GOODWE_AA55_SET_SENSOR_UNAVAILABLE, )
    this->s_work_mode_->publish_state("Offline");
    this->s_error_codes_->publish_state("");
  }
}

void GoodweAA55::parse_data() {
  // Example parsing method
  // Translates data received into buffer_data_ and stores it in parsed_value_ for
  ESP_LOGD(LOGGING_TAG, "Parsing response %s (%d bytes)", this->create_hex_string(receive_buffer_),
           receive_buffer_.size());

  ESP_LOGD(LOGGING_TAG, "Verifying received checksum...");
  if (!this->verify_checksum(receive_buffer_)) {  // CRC checksum is removed in this function
    ESP_LOGW(LOGGING_TAG, "Response has an incorrect checksum, ignoring...");
    return;
  }

  ESP_LOGD(LOGGING_TAG, "Message checksum is correct. Parsing headers...");
  ESP_LOGD(LOGGING_TAG,
           "Verifying that the packet is destined for me (my address: %x, packet destination address: %x)...",
           master_address_, receive_buffer_.at(3));

  if (master_address_ != receive_buffer_.at(3)) {
    ESP_LOGD(LOGGING_TAG, "Received packet which is not for me. Stopping processing...");
    return;
  }

  ESP_LOGD(LOGGING_TAG, "Received packet is for me. Parsing payload...");

  // During boot, sometimes the inverter returns an all 0 payload to the read command.
  // By checking if the E-total value is > 0, we discard these responses.
  if (parse_int(receive_buffer_, 31, 4, 1) == 0) {
    ESP_LOGI(LOGGING_TAG, "Received read response with all 0 payload. Discarding response...");
    return;
  }

  v_vpv1_ = parse_int(receive_buffer_, 7, 2, 1);
  v_vpv2_ = parse_int(receive_buffer_, 9, 2, 1);
  v_ipv1_ = parse_int(receive_buffer_, 11, 2, 1);
  v_ipv2_ = parse_int(receive_buffer_, 13, 2, 1);
  v_vac1_ = parse_int(receive_buffer_, 15, 2, 1);
  v_iac1_ = parse_int(receive_buffer_, 17, 2, 1);
  v_fac1_ = parse_int(receive_buffer_, 19, 2, 2);
  v_pac_ = parse_int(receive_buffer_, 21, 2, 0);
  v_work_mode_code_ = parse_int(receive_buffer_, 23, 2, 0);
  v_temperature_ = parse_int(receive_buffer_, 25, 2, 1);
  v_error_codes_code_ = parse_int(receive_buffer_, 27, 4, 0);
  v_e_total_ = parse_int(receive_buffer_, 31, 4, 1);
  v_h_total_ = parse_int(receive_buffer_, 35, 4, 0);
  v_gfci_fault_value_ = parse_int(receive_buffer_, 49, 2, 0);
  v_e_today_ = parse_int(receive_buffer_, 51, 2, 1);

  if (v_work_mode_code_ > 2) {
    v_work_mode_ = "Unknown: " + std::to_string(v_work_mode_code_);
  } else {
    v_work_mode_ = work_mode_list[v_work_mode_code_];
  }
  if (v_error_codes_code_) {
    v_error_codes_ = "";
    for (uint8_t i = 0; i < 32; ++i) {
      if (v_error_codes_code_ & (1 << i)) {
        if (!v_error_codes_.empty()) {
          v_error_codes_ += ", ";
        }
        v_error_codes_ += error_code_list[i];
      }
    }
  } else {
    v_error_codes_ = "No errors";
  }

#define GOODWE_AA55_PRINT_SENSOR_VALUES(s) ESP_LOGV(LOGGING_TAG, "Parsed %s: %f", s, (float) v_##s);
  GOODWE_AA55_SENSOR_LIST(GOODWE_AA55_PRINT_SENSOR_VALUES, )
#define GOODWE_AA55_PRINT_TEXT_SENSOR_VALUES(s) \
  ESP_LOGV(LOGGING_TAG, "Parsed %s: %d -> %s", s, v_##s##_code_, v_##s##_);
  GOODWE_AA55_SENSOR_LIST(GOODWE_AA55_PRINT_TEXT_SENSOR_VALUES, )
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

float GoodweAA55::parse_int(std::vector<uint8_t> message, uint8_t start, uint8_t bytes, uint8_t precision) {
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

  if (precision > 0) {
    return (float) response / std::pow(10.0, (float) precision);
  }

  return (float) response;
}
}  // namespace goodwe_aa55
}  // namespace esphome
