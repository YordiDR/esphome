#pragma once

#include "esphome/core/component.h"
#include "sensor/aa55_inverter_sensor.h"
#include "text_sensor/aa55_inverter_text_sensor.h"
#include "const.h"
#include "../aa55_bus/aa55_bus.h"
#include <string>
#include <vector>
#include <deque>

namespace esphome {
namespace aa55_inverter {

class AA55Inverter : public PollingComponent {
 public:
  AA55Inverter(std::string serial_number, uint8_t slave_address);
  void setup() override;
  void dump_config() override;
  void loop() override;
  void update() override;
  void add_sensor(AA55InverterSensor *sensor);
  void add_text_sensor(AA55InverterTextSensor *sensor);
  aa55_bus::AA55Bus *get_parent_bus() { return this->parent_bus_; };
  void set_parent_bus(aa55_bus::AA55Bus *bus) { this->parent_bus_ = bus; };

 protected:
  // Internal variables
  std::string serial_number_;
  uint8_t slave_address_, inverter_offline_countdown_ = aa55_const::INVERTER_OFFLINE_COUNTDOWN_RESET;
  std::vector<AA55InverterSensor *> sensors_;
  std::vector<AA55InverterTextSensor *> text_sensors_;
  bool inverter_online_ = false;
  aa55_bus::AA55Bus *parent_bus_{nullptr};

  // Functions
  void parse_run_info_response(
      const std::vector<uint8_t> &payload);  // A method to parse the running info data read from the inverter
  void parse_id_info_response(
      const std::vector<uint8_t> &payload);  // A method to parse the ID info data read from the inverter
  template<typename T> std::string create_hex_string(const T &data) {
    std::string result;
    result.reserve(data.size() * 3);

    const char *hex = "0123456789ABCDEF";

    for (uint8_t byte : data) {
      result.push_back(hex[(byte >> 4) & 0xF]);
      result.push_back(hex[byte & 0xF]);
      result.push_back(' ');
    }

    if (!result.empty())
      result.pop_back();  // Remove trailing space
    return result;
  }
};

}  // namespace aa55_inverter
}  // namespace esphome
