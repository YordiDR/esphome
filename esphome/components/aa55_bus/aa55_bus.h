#pragma once
#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "const.h"
#include <string>
#include <vector>

namespace esphome {
namespace aa55_bus {

static const char *LOGGING_TAG = "aa55_bus";

class AA55Bus : public uart::UARTDevice, public PollingComponent {
 public:
  AA55Bus(uint8_t master_address);
  void setup() override;
  void dump_config() override;
  void loop() override;
  void update() override;

  uint8_t get_master_address() { return this->master_address_; };
  std::string get_component_id() { return this->id_; };
  void set_component_id(std::string id) { this->id_ = id; };
  void send_packet(const aa55_const::AA55Command &command);  // Function that generates the packet and sends it via UART
  std::vector<uint8_t> await_response(
      const aa55_const::AA55Command
          &command);            // Function that awaits the response to a packet via UART, returns packet payload
  void drain_uart_rx_buffer();  // Function that removes all content from the UART RX buffer

 protected:
  // Internal variables
  uint8_t master_address_;
  std::vector<uint8_t> receive_buffer_;
  std::string id_;

  // Functions
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

}  // namespace aa55_bus
}  // namespace esphome
