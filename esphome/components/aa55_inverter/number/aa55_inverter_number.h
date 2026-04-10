#pragma once
#include "esphome/components/number/number.h"
#include "esphome/core/component.h"
#include "../aa55_inverter_base_input.h"
#include <string>

namespace esphome {
namespace aa55_inverter {

class AA55InverterNumber : public AA55InverterBaseInput, public number::Number, public Component {
 public:
  AA55InverterNumber(std::string id, aa55_const::INPUT_TYPE type, AA55Inverter *parent_inverter, bool offline_hold,
                     float offline_value, float online_initial_value)
      : AA55InverterBaseInput(id, type, parent_inverter, offline_hold), number::Number(), Component() {
    this->offline_value_ = offline_value;
    this->online_initial_value_ = online_initial_value;
  }

  void setup() override {
    // Initialize as unknown
    this->publish_state(NAN);
  }

  void dump_config() override {
    ESP_LOGCONFIG(LOGGING_TAG, "Goodwe AA55 Inverter number");
    ESP_LOGCONFIG(LOGGING_TAG, "  Id: %s", this->id_.c_str());
  }

  void control(float value) {
    this->last_sent_value_ = value;
    ESP_LOGD(LOGGING_TAG, "Number %s was changed, new state: %f", this->id_.c_str(), value);
    if (this->type_ == aa55_const::INPUT_TYPE::ADJUST_POWER) {
      this->parent_inverter_->send_execute_command(aa55_const::FUNCTION_CODE::ADJUST_POWER,
                                                   static_cast<uint8_t>(value));
    }
  }

  void handle_response(aa55_const::FUNCTION_CODE function_code, uint8_t response) override {
    if (response != 6) {
      ESP_LOGW(LOGGING_TAG, "Inverter %x responded with NACK on inverter command %x.",
               this->parent_inverter_->get_slave_address(), ((uint8_t) function_code) & 0x7F);
      return;
    }

    if (this->type_ == aa55_const::INPUT_TYPE::ADJUST_POWER &&
        function_code == aa55_const::FUNCTION_CODE::ADJUST_POWER_RESPONSE) {
      ESP_LOGD(LOGGING_TAG, "Inverter %x ACK'ed the adjust power command.",
               this->parent_inverter_->get_slave_address());
    } else {
      ESP_LOGD(LOGGING_TAG, "Inverter %x sensor %s got an incorrect function code %x as response.",
               this->parent_inverter_->get_slave_address(), this->id_, function_code);
      return;
    }

    this->publish_state(this->last_sent_value_);
  }

  void handle_inverter_offline() override {
    if (!this->offline_hold_) {
      ESP_LOGD(LOGGING_TAG, "Publishing offline value %f for number %s because the inverter stopped responding.",
               this->offline_value_, this->id_.c_str());
      this->publish_state(this->offline_value_);
    }
  }

  void handle_inverter_online() override {
    ESP_LOGD(LOGGING_TAG, "Publishing online initial value %f for number %s because the inverter came online.",
             this->online_initial_value_, this->id_.c_str());
    this->publish_state(this->online_initial_value_);
  }

 protected:
  float last_sent_value_{}, offline_value_{}, online_initial_value_{};
};
}  // namespace aa55_inverter
}  // namespace esphome
