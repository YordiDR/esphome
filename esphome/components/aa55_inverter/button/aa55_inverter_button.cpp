#include "esphome/core/log.h"
#include "esphome/components/aa55_inverter/button/aa55_inverter_button.h"
#include "esphome/components/aa55_inverter/aa55_inverter.h"

namespace esphome {
namespace aa55_inverter {
static const char *LOGGING_TAG = "aa55_input";

AA55InverterButton::AA55InverterButton(std::string id, aa55_inverter::INPUT_TYPE type, AA55Inverter *parent_inverter)
    : AA55InverterBaseInput(id, type, parent_inverter, false), button::Button(), Component() {}

void AA55InverterButton::dump_config() {
  ESP_LOGCONFIG(LOGGING_TAG, "Goodwe AA55 Inverter button");
  ESP_LOGCONFIG(LOGGING_TAG, "  Id: %s", this->id_.c_str());
}

void AA55InverterButton::handle_response(aa55_bus::FUNCTION_CODE function_code, uint8_t response) {
  if (response != 6) {
    ESP_LOGW(LOGGING_TAG, "Inverter %x responded with NACK on inverter command %x.",
             this->parent_inverter_->get_device_address(), ((uint8_t) function_code) & 0x7F);
    return;
  }

  if (this->type_ == aa55_inverter::INPUT_TYPE::RECONNECT_GRID &&
      function_code == aa55_bus::FUNCTION_CODE::RECONNECT_GRID_RESPONSE) {
    ESP_LOGD(LOGGING_TAG, "Inverter %x ACK'ed the reconnect grid command.",
             this->parent_inverter_->get_device_address());
  } else {
    ESP_LOGD(LOGGING_TAG, "Inverter %x button %s got an incorrect function code %x as response.",
             this->parent_inverter_->get_device_address(), this->id_, function_code);
    return;
  }
}

void AA55InverterButton::handle_inverter_offline() {
  ESP_LOGD(LOGGING_TAG, "Nothing to change for button %s when inverter goes offline.", this->id_.c_str());
}

void AA55InverterButton::handle_inverter_online() {
  ESP_LOGD(LOGGING_TAG, "Nothing to change for button %s when inverter comes online.", this->id_.c_str());
}

void AA55InverterButton::press_action() {
  ESP_LOGD(LOGGING_TAG, "Button %s was pressed", this->id_.c_str());
  if (this->type_ == aa55_inverter::INPUT_TYPE::RECONNECT_GRID) {
    this->parent_inverter_->send_execute_command(aa55_bus::FUNCTION_CODE::RECONNECT_GRID);
  }
}
}  // namespace aa55_inverter
}  // namespace esphome
