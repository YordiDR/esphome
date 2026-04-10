#pragma once
#include "esphome/components/switch/switch.h"
#include "esphome/core/component.h"
#include "../aa55_inverter_base_input.h"
#include <string>

namespace esphome {
namespace aa55_inverter {

class AA55InverterSwitch : public AA55InverterBaseInput, public switch_::Switch, public Component {
 public:
  AA55InverterSwitch(std::string id, aa55_const::INPUT_TYPE type, AA55Inverter *parent_inverter, bool offline_hold,
                     aa55_const::ON_OFF offline_value, aa55_const::ON_OFF online_initial_value)
      : AA55InverterBaseInput(id, type, parent_inverter, offline_hold), switch_::Switch(), Component() {
    this->offline_value_ = offline_value;
    this->online_initial_value_ = online_initial_value;
  };

  void setup() override {
    // Initialize as off
    this->publish_state(false);
  }
  void dump_config() override {
    ESP_LOGCONFIG(LOGGING_TAG, "Goodwe AA55 Inverter switch");
    ESP_LOGCONFIG(LOGGING_TAG, "  Id: %s", this->id_.c_str());
    ESP_LOGCONFIG(LOGGING_TAG, "  Offline Hold: %s", this->offline_hold_ ? "true" : "false");
    ESP_LOGCONFIG(LOGGING_TAG, "  Offline Value: %s", static_cast<uint8_t>(this->offline_value_) ? "ON" : "OFF");
  }

  void write_state(bool state) {
    this->last_sent_value_ = state;
    ESP_LOGD(LOGGING_TAG, "Switch %s was toggled, new state: %s", this->id_.c_str(), state ? "ON" : "OFF");
    if (this->type_ == aa55_const::INPUT_TYPE::START_STOP) {
      if (state) {
        this->parent_inverter_->send_execute_command(aa55_const::FUNCTION_CODE::START_INVERTER);
      } else {
        this->parent_inverter_->send_execute_command(aa55_const::FUNCTION_CODE::STOP_INVERTER);
      }
    }
  };

  void handle_response(aa55_const::FUNCTION_CODE function_code, uint8_t response) override {
    if (response != 6) {
      ESP_LOGW(LOGGING_TAG, "Inverter %x responded with NACK on inverter command %x.",
               this->parent_inverter_->get_slave_address(), ((uint8_t) function_code) & 0x7F);
      return;
    }

    if (this->type_ == aa55_const::INPUT_TYPE::START_STOP &&
        function_code == aa55_const::FUNCTION_CODE::START_INVERTER_RESPONSE) {
      ESP_LOGD(LOGGING_TAG, "Inverter %x ACK'ed the start command.", this->parent_inverter_->get_slave_address());
    } else if (this->type_ == aa55_const::INPUT_TYPE::START_STOP &&
               function_code == aa55_const::FUNCTION_CODE::STOP_INVERTER_RESPONSE) {
      ESP_LOGD(LOGGING_TAG, "Inverter %x ACK'ed the stop command.", this->parent_inverter_->get_slave_address());
    } else {
      ESP_LOGD(LOGGING_TAG, "Inverter %x sensor %s got an incorrect function code %x as response.",
               this->parent_inverter_->get_slave_address(), this->id_, function_code);
      return;
    }

    this->publish_state(this->last_sent_value_);
  }

  void handle_inverter_offline() override {
    if (!this->offline_hold_) {
      ESP_LOGD(LOGGING_TAG, "Publishing offline value '%s' for switch %s because the inverter stopped responding.",
               static_cast<uint8_t>(this->offline_value_) ? "ON" : "OFF", this->id_.c_str());
      this->publish_state(static_cast<uint8_t>(this->offline_value_));
    }
  }

  void handle_inverter_online() override {
    ESP_LOGD(LOGGING_TAG, "Publishing online initial value '%s' for switch %s because the inverter came online.",
             static_cast<uint8_t>(this->online_initial_value_) ? "ON" : "OFF", this->id_.c_str());
    this->publish_state(static_cast<uint8_t>(this->online_initial_value_));
  }

 protected:
  bool last_sent_value_{};
  aa55_const::ON_OFF offline_value_{}, online_initial_value_{};
};
}  // namespace aa55_inverter
}  // namespace esphome
