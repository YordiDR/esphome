#pragma once
#include "esphome/components/button/button.h"
#include "esphome/core/component.h"
#include "../aa55_inverter_base_input.h"
#include <string>

namespace esphome {
namespace aa55_inverter {

class AA55InverterButton : public AA55InverterBaseInput, public button::Button, public Component {
 public:
  AA55InverterButton(std::string id, aa55_const::INPUT_TYPE type, AA55Inverter *parent_inverter)
      : AA55InverterBaseInput(id, type, parent_inverter, false), button::Button(), Component(){};
  void dump_config() override {
    ESP_LOGCONFIG(LOGGING_TAG, "Goodwe AA55 Inverter button");
    ESP_LOGCONFIG(LOGGING_TAG, "  Id: %s", this->id_.c_str());
  }

  void handle_response(aa55_const::FUNCTION_CODE function_code, uint8_t response) override {
    if (response != 6) {
      ESP_LOGW(LOGGING_TAG, "Inverter %x responded with NACK on inverter command %x.",
               this->parent_inverter_->get_slave_address(), ((uint8_t) function_code) & 0x7F);
      return;
    }

    if (this->type_ == aa55_const::INPUT_TYPE::RECONNECT_GRID &&
        function_code == aa55_const::FUNCTION_CODE::RECONNECT_GRID_RESPONSE) {
      ESP_LOGD(LOGGING_TAG, "Inverter %x ACK'ed the reconnect grid command.",
               this->parent_inverter_->get_slave_address());
    } else {
      ESP_LOGD(LOGGING_TAG, "Inverter %x button %s got an incorrect function code %x as response.",
               this->parent_inverter_->get_slave_address(), this->id_, function_code);
      return;
    }
  }

  void handle_inverter_offline() override {
    ESP_LOGD(LOGGING_TAG, "Nothing to change for button %s when inverter goes offline.", this->id_.c_str());
  }

  void handle_inverter_online() override {
    ESP_LOGD(LOGGING_TAG, "Nothing to change for button %s when inverter comes online.", this->id_.c_str());
  }

 protected:
  void press_action() override {
    ESP_LOGD(LOGGING_TAG, "Button %s was pressed", this->id_.c_str());
    if (this->type_ == aa55_const::INPUT_TYPE::RECONNECT_GRID) {
      this->parent_inverter_->send_execute_command(aa55_const::FUNCTION_CODE::RECONNECT_GRID);
    }
  }
};

}  // namespace aa55_inverter
}  // namespace esphome
