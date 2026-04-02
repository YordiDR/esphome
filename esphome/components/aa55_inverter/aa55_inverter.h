#pragma once

#include "esphome/core/component.h"
#include "sensor/aa55_inverter_sensor.h"
#include "text_sensor/aa55_inverter_text_sensor.h"
#include "aa55_inverter_base_input.h"
#include "const.h"
#include <string>
#include <vector>
#include <queue>

namespace esphome {
namespace aa55_bus {
class AA55Bus;  // Forward declaration of AA55Bus class to avoid circular dependency
}
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
  void add_input(AA55InverterBaseInput *input);
  uint8_t get_slave_address() { return this->slave_address_; };
  void set_parent_bus(aa55_bus::AA55Bus *bus) { this->parent_bus_ = bus; };
  void queue_response_packet(const aa55_const::AA55Packet &packet) { this->response_packets_buffer_.push(packet); };
  void send_execute_command(aa55_const::FUNCTION_CODE function_code, uint8_t payload = 0);

 protected:
  // Internal variables
  std::string serial_number_;
  uint8_t slave_address_;
  std::vector<AA55InverterSensor *> sensors_;
  std::vector<AA55InverterTextSensor *> text_sensors_;
  std::vector<AA55InverterBaseInput *> inputs_;
  bool inverter_online_ = false;
  aa55_bus::AA55Bus *parent_bus_{nullptr};
  std::queue<aa55_const::AA55Packet> response_packets_buffer_;
  uint32_t last_packet_received_{0};

  // Functions
  void parse_run_info_response(
      const std::vector<uint8_t> &payload);  // A method to parse the running info data read from the inverter
  void parse_id_info_response(
      const std::vector<uint8_t> &payload);  // A method to parse the ID info data read from the inverter
  void parse_execute_response(aa55_const::FUNCTION_CODE function_code, uint8_t response);
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
