#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace goodwe_aa55 {

static const uint8_t MAX_LINE_LENGTH = 150;  // Max characters for serial buffer, 150 bytes is the length of the
                                             // response to the "read running info list" command

class GoodweAA55 : public uart::UARTDevice, public PollingComponent {
 public:
  GoodweAA55(std::string serial_number, uint8_t slave_address, uint8_t master_address, uint32_t update_interval);
  void setup() override;
  void dump_config() override;
  void loop() override;
  void set_pac_sensor(sensor::Sensor *pac_sensor) { pac_sensor_ = pac_sensor; }
  void set_vpv1_sensor(sensor::Sensor *vpv1_sensor) { vpv1_sensor_ = vpv1_sensor; }
  void update() override {
    this->pac_sensor_->publish_state(pac_);
    this->vpv1_sensor_->publish_state(vpv1_);
  }

 protected:
  // Internal variables
  std::string serial_number_;
  uint8_t slave_address_;
  uint8_t master_address_;
  uint8_t update_interval_;
  uint32_t loop_counter_ = 0;
  std::vector<uint8_t> receive_buffer_;
  bool inverter_registered_ = false;
  uint16_t pac_ = 0;
  float vpv1_ = 0.0;

  // Sensors
  sensor::Sensor *pac_sensor_{nullptr};
  sensor::Sensor *vpv1_sensor_{nullptr};

  // Functions
  void parse_data();  // A method to parse the data read from the sensor hardware
  std::vector<uint8_t> calculate_checksum(
      std::vector<uint8_t> message);                   // Method that calculates the CRC checksum for an AA55 message
  bool verify_checksum(std::vector<uint8_t> message);  // Method that verifies the AA55 CRC checksum
  std::string create_hex_string(std::vector<uint8_t> data);  // Method that converts an array of bytes to a hex string
  float parse_int(std::vector<uint8_t> message, uint8_t start, uint8_t bytes, uint8_t precision = 0);
};

}  // namespace goodwe_aa55
}  // namespace esphome
