#include "esphome/core/log.h"
#include "goodwe_aa55.h"
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <iterator>
#include <cmath>

namespace esphome {
namespace goodwe_aa55 {

static const char *LOGGING_TAG = "goodwe_aa55";
const std::vector<uint8_t> HEADERS = {0xaa, 0x55};
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

GoodweAA55::GoodweAA55(std::string serial_number, uint8_t slave_address, uint8_t master_address,
                       uint32_t update_interval) {
  serial_number_ = serial_number;
  slave_address_ = slave_address;
  master_address_ = master_address;
  update_interval_ = update_interval;
}

void GoodweAA55::setup() {
  // TODO Send deregister command to inverter address so we can register it again
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
  uint8_t buffer_byte;
  receive_buffer_.clear();
  uint8_t message_length = MAX_LINE_LENGTH;  // Initialize bytes to read as maximum AA55 message length
  std::vector<uint8_t> message = HEADERS;    // Initialize message with AA55 header, then add command details
  message.push_back(master_address_);
  message.push_back(0x7f);
  message.push_back((uint8_t) CONTROL_CODE::READ);
  message.push_back((uint8_t) READ_FUNCTION_CODE::QUERY_RUN_INFO);
  message.push_back(0x00);
  std::vector<uint8_t> crc = this->calculate_checksum(message);  // Calculate & add checksum
  message.insert(message.end(), crc.begin(), crc.end());
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
  ESP_LOGD(LOGGING_TAG, "Parsing response %s (%d bytes)", this->create_hex_string(receive_buffer_),
           receive_buffer_.size());

  ESP_LOGD(LOGGING_TAG, "Verifying received checksum...");
  if (!this->verify_checksum(receive_buffer_)) {
    ESP_LOGW(LOGGING_TAG, "Response has an incorrect checksum, ignoring...");
    return;
  }

  ESP_LOGD(LOGGING_TAG,
           "Verifying that the packet is destined for me (my address: %x, packet destination address: %x)...",
           master_address_, receive_buffer_.at(3));

  if (master_address_ != receive_buffer_.at(3)) {
    ESP_LOGD(LOGGING_TAG, "Received packet which is not for me. Stopping processing...");
    return;
  }

  vpv1_ = parse_int(receive_buffer_, 7, 2, 1);                                 // PV string 1 voltage
  const float vpv2 = parse_int(receive_buffer_, 9, 2, 1);                      // PV string 2 voltage
  const float ipv1 = parse_int(receive_buffer_, 11, 2, 1);                     // PV string 1 current
  const float ipv2 = parse_int(receive_buffer_, 13, 2, 1);                     // PV string 2 current
  const float vac1 = parse_int(receive_buffer_, 15, 2, 1);                     // Phase 1 voltage
  const float iac1 = parse_int(receive_buffer_, 17, 2, 1);                     // Phase 1 current
  const float fac1 = parse_int(receive_buffer_, 19, 2, 2);                     // Phase 1 frequency
  pac_ = parse_int(receive_buffer_, 21, 2, 0);                                 // Feeding power
  const uint16_t work_mode = parse_int(receive_buffer_, 23, 2, 0);             // Inverter work mode
  const float temperature = parse_int(receive_buffer_, 25, 2, 1);              // Inverter temperature
  const uint32_t error_code = parse_int(receive_buffer_, 27, 4, 0);            // Error code
  const float e_total = parse_int(receive_buffer_, 31, 4, 1);                  // Total generated energy
  const uint32_t h_total = parse_int(receive_buffer_, 35, 4, 0);               // Total inverter runtime
  const float temperature_fault_value = parse_int(receive_buffer_, 39, 2, 1);  // Temperature fault value
  const float pv1v_fault_value = parse_int(receive_buffer_, 41, 2, 1);         // PV string 1 voltage fault value
  const float pv2v_fault_value = parse_int(receive_buffer_, 43, 2, 1);         // PV string 2 voltage fault value
  const float l1v_fault_value = parse_int(receive_buffer_, 45, 2, 1);          // Phase 1 voltage fault value
  const float l1f_fault_value = parse_int(receive_buffer_, 47, 2, 2);          // Phase 1 frequency fault value
  const uint16_t gfci_fault_value = parse_int(receive_buffer_, 49, 2, 0);      // GFCI fault value
  const float e_day = parse_int(receive_buffer_, 51, 2, 1);                    // Energy feed into grid today

  ESP_LOGV(LOGGING_TAG, "Parsed Vpv1: %f", vpv1_);
  ESP_LOGV(LOGGING_TAG, "Parsed Vpv2: %f", vpv2);
  ESP_LOGV(LOGGING_TAG, "Parsed Ipv1: %f", ipv1);
  ESP_LOGV(LOGGING_TAG, "Parsed Ipv1: %f", ipv2);
  ESP_LOGV(LOGGING_TAG, "Parsed Vac1: %f", vac1);
  ESP_LOGV(LOGGING_TAG, "Parsed Iac1: %f", iac1);
  ESP_LOGV(LOGGING_TAG, "Parsed Fac1: %f", fac1);
  ESP_LOGV(LOGGING_TAG, "Parsed Pac: %d", pac_);
  ESP_LOGV(LOGGING_TAG, "Parsed work mode: %d", work_mode);
  ESP_LOGV(LOGGING_TAG, "Parsed temperature: %f", temperature);
  ESP_LOGV(LOGGING_TAG, "Parsed error code: %d", error_code);
  ESP_LOGV(LOGGING_TAG, "Parsed e_total: %f", e_total);
  ESP_LOGV(LOGGING_TAG, "Parsed h_total: %d", h_total);
  ESP_LOGV(LOGGING_TAG, "Parsed temperature fault value: %f", temperature_fault_value);
  ESP_LOGV(LOGGING_TAG, "Parsed pv1v fault value: %f", pv1v_fault_value);
  ESP_LOGV(LOGGING_TAG, "Parsed pv2v fault value: %f", pv2v_fault_value);
  ESP_LOGV(LOGGING_TAG, "Parsed l1v fault value: %f", l1v_fault_value);
  ESP_LOGV(LOGGING_TAG, "Parsed l1f fault value: %f", l1f_fault_value);
  ESP_LOGV(LOGGING_TAG, "Parsed GFCI fault value: %d", gfci_fault_value);
  ESP_LOGV(LOGGING_TAG, "Parsed e_day: %f", e_day);
}

std::vector<uint8_t> GoodweAA55::calculate_checksum(std::vector<uint8_t> message) {
  uint16_t crc = 0;
  ESP_LOGD(LOGGING_TAG, "Calculating CRC for message '%s'...", this->create_hex_string(message));
  for (uint8_t byte : message) {
    ESP_LOGV(LOGGING_TAG, "Checksum calculation: adding value %x to current CRC value (%d)", byte, crc);
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

float GoodweAA55::parse_int(std::vector<uint8_t> message, uint8_t start, uint8_t bytes, uint8_t precision) {
  uint32_t response_int = 0;
  switch (bytes) {
    case 2:
      response_int |= message.at(start) << 8;
      response_int |= message.at(start + 1);
      break;
    case 4:
      response_int |= message.at(start) << 24;
      response_int |= message.at(start + 1) << 16;
      response_int |= message.at(start + 2) << 8;
      response_int |= message.at(start + 3);
      break;
    default:
      ESP_LOGE(LOGGING_TAG, "Received incorrect value for bytes parameter in GoodweAA55::parse_int. Value: %d", bytes);
      return 0.0;
  }

  float response_float;
  if (precision > 0) {
    response_float = (float) response_int / std::pow(10.0, (float) precision);
  } else {
    response_float = (float) response_int;
  }

  return response_float;
}
}  // namespace goodwe_aa55
}  // namespace esphome
