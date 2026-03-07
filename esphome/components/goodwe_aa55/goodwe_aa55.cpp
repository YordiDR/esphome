#include "esphome/core/log.h"
#include "goodwe_aa55.h"
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

namespace esphome {
namespace goodwe_aa55 {

static const char *LOGGING_TAG = "goodwe_aa55";
const std::vector<uint8_t> HEADERS = {0xaa, 0x55};
const uint8_t MASTER_ADDR = 0xff;
const uint8_t SLAVE_ADDR = 0x7f;
enum class CONTROL_CODE { REGISTER = 0x00, READ = 0x01, EXECUTE = 0x03 };
enum class REG_FUNCTION_CODE {
  OFFLINE_QUERY = 0x00,
  ALLOC_REG_ADDR = 0x01,
  REMOVE_REG = 0x02,
  REG_REQUEST = 0x80,
  ADDR_CONFIRM = 0x81,
  REMOVE_CONFIRM = 0x82
};
enum class READ_FUNCTION_CODE {
  QUERY_RUN_INFO = 0x01,
  QUERY_ID_INFO = 0x02,
  QUERY_SET_INFO = 0x03,
  RUN_INFO_RESPONSE = 0x81,
  ID_INFO_RESPONSE = 0x82,
  SET_INFO_RESPONSE = 0x83
};

void GoodweAA55::setup() {
  // Serial/UART device initialization is typically done here.
}

void GoodweAA55::dump_config() { ESP_LOGCONFIG(LOGGING_TAG, "Empty UART sensor"); }

void GoodweAA55::loop() {
  loop_counter++;

  if (loop_counter < 1000) {
    return;
  }

  loop_counter = 0;
  // Work to be done at each update interval
  uint8_t buffer_pos = 0;                  // Counter used for populating the buffer
  std::vector<uint8_t> message = HEADERS;  // Initialize message with AA55 header, then add command details
  message.push_back(MASTER_ADDR);
  message.push_back(SLAVE_ADDR);
  message.push_back((uint8_t) CONTROL_CODE::READ);
  message.push_back((uint8_t) READ_FUNCTION_CODE::QUERY_RUN_INFO);
  message.push_back(0x00);
  std::vector<uint8_t> crc = this->calculate_checksum(message);  // Calculate & add checksum
  message.insert(message.end(), crc.begin(), crc.end());
  ESP_LOGD(LOGGING_TAG, "Sending message %x", this->create_hex_string(message));

  this->write_array(message);  // Send query running info command to inverter
  // Read the response from the device, up to MAX_LINE_LENGTH bytes
  while (this->available() && buffer_pos < MAX_LINE_LENGTH && this->read_byte(&this->buffer_data_[buffer_pos++])) {
  }

  if (buffer_pos > 0) {
    this->parse_data();  // If we have read some data, parse it
    // this->publish_state(this->parsed_value_);  // Publish the parsed value as a sensor state
  } else {
    ESP_LOGW(LOGGING_TAG, "No data received");
    this->status_set_warning();  // We can indicate a warning if no data was read
  }
}

void GoodweAA55::parse_data() {
  // Example parsing method
  // Translates data received into buffer_data_ and stores it in parsed_value_ for
  uint8_t message_length =
      9 + this->buffer_data_[6];  // Intitialize message length as the sum of all non-payload bytes + the payload length
  ESP_LOGD(LOGGING_TAG, "Parsing response %x", this->create_hex_string(this->buffer_data_, message_length));
  const float vpv1 = (((uint16_t) this->buffer_data_[7] << 8) + this->buffer_data_[8]) / 10;
  ESP_LOGD(LOGGING_TAG, "Parsed Vpv1: %x", vpv1);
}

std::vector<uint8_t> GoodweAA55::calculate_checksum(std::vector<uint8_t> message) {
  uint16_t crc = 0;
  ESP_LOGD(LOGGING_TAG, "Calculating CRC for message '%x'...", this->create_hex_string(message));
  for (uint8_t byte : message) {
    ESP_LOGD(LOGGING_TAG, "Checksum calculation: adding value %x to current CRC value (%d)", byte, crc);
    crc += byte;
  }

  ESP_LOGD(LOGGING_TAG, "Calculated CRC value: %d", crc);
  const std::vector<uint8_t> crc_bytes = {(uint8_t) (crc >> 8), (uint8_t) crc};
  ESP_LOGD(LOGGING_TAG, "Returning CRC split into bytes: {%x, %x}", crc_bytes.at(0), crc_bytes.at(1));
  return crc_bytes;
}

bool GoodweAA55::verify_checksum(std::vector<uint8_t> message) {
  // Save & remove CRC bytes from message
  const uint8_t crc_received_low_byte = message.back();
  message.pop_back();
  const uint8_t crc_received_high_byte = message.back();
  message.pop_back();

  // Calculate CRC from message
  const std::vector<uint8_t> calculated_checksum = this->calculate_checksum(message);

  // Check if calculated CRC matches received CRC
  ESP_LOGD(LOGGING_TAG, "Checking if CRC for received message is correct (calculated CRC: %x%x, received CRC: %x%x)",
           calculated_checksum.at(0), calculated_checksum.at(1), crc_received_high_byte, crc_received_low_byte);
  return (calculated_checksum.at(0) == crc_received_high_byte && calculated_checksum.at(1) == crc_received_low_byte);
}

std::string GoodweAA55::create_hex_string(std::vector<uint8_t> data) {
  std::stringstream ss;
  ss << std::hex;

  for (uint8_t byte : data) {
    ss << std::setw(2) << std::setfill('0') << byte;
  }

  return ss.str();
}

std::string GoodweAA55::create_hex_string(uint8_t *data, uint8_t len) {
  std::stringstream ss;
  ss << std::hex;

  for (int i(0); i < len; ++i) {
    ss << std::setw(2) << std::setfill('0') << (uint8_t) data[i];
  }

  return ss.str();
}
}  // namespace goodwe_aa55
}  // namespace esphome
