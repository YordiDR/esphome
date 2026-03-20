#pragma once

#include "esphome/core/component.h"
#include "esphome/core/defines.h"
#include "esphome/components/uart/uart.h"
#include "sensor/goodwe_aa55_sensor.h"
#include "text_sensor/goodwe_aa55_text_sensor.h"
#include <string>
#include <vector>
#include <deque>

namespace esphome {
namespace goodwe_aa55 {

class GoodweAA55 : public uart::UARTDevice, public PollingComponent {
 public:
  GoodweAA55(std::string serial_number, uint8_t slave_address, uint8_t master_address, uint32_t update_interval);
  void setup() override;
  void dump_config() override;
  void loop() override;
  void update() override;
  void add_sensor(GoodweAA55Sensor *sensor);
  void add_text_sensor(GoodweAA55TextSensor *sensor);

 protected:
  // Internal variables
  std::string serial_number_;
  uint8_t slave_address_, master_address_, inverter_offline_countdown_ = INVERTER_OFFLINE_COUNTDOWN_RESET;
  uint16_t update_interval_;
  uint32_t loop_counter_ = 0;
  std::vector<uint8_t> receive_buffer_;
  std::vector<GoodweAA55Sensor *> sensors_;
  std::vector<GoodweAA55TextSensor *> text_sensors_;
  bool inverter_online_ = false;

  // Functions
  void parse_data(std::vector<uint8_t> &payload);         // A method to parse the data read from the sensor hardware
  std::vector<uint8_t> calculate_checksum(auto &packet);  // Method that calculates the CRC checksum for an AA55 packet
  std::string create_hex_string(std::vector<uint8_t> &data);  // Method that converts vector of bytes to a hex string
  std::string create_hex_string(std::deque<uint8_t> &data);   // Method that converts deque of bytes to a hex string
  uint32_t parse_int(std::vector<uint8_t> message, uint8_t start, uint8_t bytes);
};

}  // namespace goodwe_aa55
}  // namespace esphome
