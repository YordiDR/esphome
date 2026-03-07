#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"

namespace esphome {
namespace goodwe_aa55 {

static const uint8_t MAX_LINE_LENGTH = 150;  // Max characters for serial buffer, 150 bytes is the length of the
                                             // response to the "read running info list" command

class GoodweAA55 : public uart::UARTDevice, public Component {
 public:
  void setup() override;
  void dump_config() override;
  void loop() override;

 protected:
  int loop_counter_ = 0;
  std::vector<uint8_t> receive_buffer_;

  void parse_data();  // A method to parse the data read from the sensor hardware
  std::vector<uint8_t> calculate_checksum(
      std::vector<uint8_t> message);                   // Method that calculates the CRC checksum for an AA55 message
  bool verify_checksum(std::vector<uint8_t> message);  // Method that verifies the AA55 CRC checksum
  std::string create_hex_string(std::vector<uint8_t> data);  // Method that converts an array of bytes to a hex string
  float parse_int(std::vector<uint8_t> message, uint8_t start, uint8_t bytes, uint8_t precision = 0);
};

}  // namespace goodwe_aa55
}  // namespace esphome
