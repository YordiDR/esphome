#pragma once
#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "const.h"
#include <string>
#include <vector>
#include <queue>

namespace esphome {
namespace aa55_inverter {
class AA55Inverter;  // Forward declaration of AA55Inverter class to avoid circular dependency
}
namespace aa55_bus {

static const char *LOGGING_TAG = "aa55_bus";

class AA55Bus : public uart::UARTDevice, public Component {
 public:
  AA55Bus(uint8_t master_address);
  void setup() override;
  void dump_config() override;
  void loop() override;
  void register_inverter(aa55_inverter::AA55Inverter *inverter) { this->registered_inverters_.push_back(inverter); };
  void queue_command(aa55_const::AA55Packet command) { this->commands_to_send_.push(command); };
  uint8_t get_master_address() { return this->master_address_; };
  std::string get_component_id() { return this->id_; };
  void set_component_id(std::string id) { this->id_ = id; };

 protected:
  // Internal variables
  uint8_t master_address_;
  std::deque<uint8_t> receive_buffer_;
  std::queue<aa55_const::AA55Packet> commands_to_send_;
  std::string id_;
  std::vector<aa55_inverter::AA55Inverter *> registered_inverters_;
  std::uint32_t last_send_time_{millis()};

  // Functions
  void send_packet(const aa55_const::AA55Packet &command);  // Function that generates the packet and sends it via UART
  void process_rx();  // Function that parses incoming data from UART and hands it over to the configured inverter
                      // objects
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

}  // namespace aa55_bus
}  // namespace esphome
