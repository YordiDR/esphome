#pragma once
#include <string>
#include <vector>
#include "esphome/core/component.h"
#include "../aa55_bus/aa55_bus.h"
#include "aa55_inverter_base_sensor.h"
#include "aa55_inverter_base_input.h"
#include "const.h"

namespace esphome {
namespace aa55_inverter {

class AA55Inverter : public PollingComponent {
 public:
  AA55Inverter(std::string serial_number, uint8_t device_address);
  void setup() override;
  void dump_config() override;
  void loop() override;
  void update() override;
  void add_sensor(AA55InverterBaseSensor *sensor);
  void add_input(AA55InverterBaseInput *input);
  uint8_t get_device_address();
  void set_parent_bus(aa55_bus::AA55Bus *bus);
  void handle_packet(const aa55_bus::AA55RXPacket &packet);
  void send_execute_command(aa55_bus::FUNCTION_CODE function_code, uint8_t payload = 0);
  std::string get_serial_number();

 protected:
  // Internal variables
  std::string serial_number_;
  uint8_t device_address_;
  std::vector<AA55InverterBaseSensor *> sensors_;
  std::vector<AA55InverterBaseInput *> inputs_;
  bool inverter_online_{false};
  aa55_bus::AA55Bus *parent_bus_{nullptr};
  // Initialised to max-timeout so the offline check doesn't fire before the inverter has registered
  uint32_t last_packet_received_{UINT32_MAX - aa55_inverter::INVERTER_OFFLINE_TIMEOUT};

  // Functions
  void parse_run_info_response(const aa55_bus::AA55RXPacket &packet);
  void parse_id_info_response(const aa55_bus::AA55RXPacket &packet);
  void parse_execute_response(const aa55_bus::AA55RXPacket &packet);
  void handle_registration_request(const aa55_bus::AA55RXPacket &packet);
  void handle_address_confirm(const aa55_bus::AA55RXPacket &packet);
  std::string create_hex_string(const uint8_t *data, size_t length) {
    std::string result;
    result.reserve(length * 3);

    const char *hex = "0123456789ABCDEF";

    for (size_t i = 0; i < length; i++) {
      result.push_back(hex[(data[i] >> 4) & 0xF]);
      result.push_back(hex[data[i] & 0xF]);
      result.push_back(' ');
    }

    if (!result.empty())
      result.pop_back();  // Remove trailing space
    return result;
  }
};

}  // namespace aa55_inverter
}  // namespace esphome
