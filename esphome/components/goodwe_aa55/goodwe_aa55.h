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

struct AA55Command {
  const uint8_t source_address;
  const uint8_t destination_address;
  const CONTROL_CODE control_code;
  const FUNCTION_CODE function_code;
  const std::vector<uint8_t> payload;
};

class GoodweAA55 : public uart::UARTDevice, public PollingComponent {
 public:
  GoodweAA55(std::string serial_number, uint8_t slave_address, uint8_t master_address);
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
  std::vector<uint8_t> receive_buffer_;
  std::vector<GoodweAA55Sensor *> sensors_;
  std::vector<GoodweAA55TextSensor *> text_sensors_;
  bool inverter_online_ = false;

  // Functions
  void parse_run_info_response(
      const std::vector<uint8_t> &payload);  // A method to parse the running info data read from the inverter
  void parse_id_info_response(
      const std::vector<uint8_t> &payload);  // A method to parse the ID info data read from the inverter
  uint32_t parse_int(const std::vector<uint8_t> &message, uint8_t start, uint8_t bytes);
  void send_packet(const AA55Command &command);  // Function that generates the packet and sends it via UART
  std::vector<uint8_t> await_response(
      const AA55Command &command);  // Function that awaits the response to a packet via UART, returns packet payload
  std::vector<uint8_t> calculate_checksum(const std::vector<uint8_t> &packet);
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

}  // namespace goodwe_aa55
}  // namespace esphome
