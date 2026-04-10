#pragma once
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/core/component.h"
#include "../aa55_inverter_base_sensor.h"
#include <string>
#include <vector>

namespace esphome {
namespace aa55_inverter {

class AA55InverterTextSensor : public AA55InverterBaseSensor, public text_sensor::TextSensor, public Component {
 public:
  AA55InverterTextSensor(std::string id, aa55_const::SENSOR_TYPE type, uint16_t skip_updates, bool offline_hold,
                         std::string offline_value)
      : AA55InverterBaseSensor(id, type, skip_updates, offline_hold), text_sensor::TextSensor(), Component() {
    this->offline_value_ = offline_value;
  };

  void parse_payload(const std::vector<uint8_t> &payload) override {
    ESP_LOGV(LOGGING_TAG, "Parsing %s from payload[%d], length %d bytes.", this->id_.c_str(), this->payload_location_,
             this->payload_length_);
    switch (this->type_) {
      case aa55_const::SENSOR_TYPE::WORK_MODE:
        this->parse_work_mode_payload(payload);
        break;
      case aa55_const::SENSOR_TYPE::ERROR_CODES:
        this->parse_error_codes_payload(payload);
        break;
      default:
        this->parse_ascii_payload(payload);
    }
    ESP_LOGV(LOGGING_TAG, "Parsed %s: %s", this->id_.c_str(), this->newest_value_.c_str());
  }

  void emit_state() override { this->publish_state(this->newest_value_); }

  void handle_inverter_offline() override {
    if (!this->offline_hold_) {
      ESP_LOGD(LOGGING_TAG, "Publishing offline value '%s' for text sensor %s because the inverter stopped responding.",
               this->offline_value_.c_str(), this->id_.c_str());
      this->publish_state(this->offline_value_);
    }
  }

  void dump_config() override {
    ESP_LOGCONFIG(LOGGING_TAG, "Goodwe AA55 text sensor");
    ESP_LOGCONFIG(LOGGING_TAG, "  Id: %s", this->id_.c_str());
    ESP_LOGCONFIG(LOGGING_TAG, "  Skip Updates: %d", this->skip_updates_);
    ESP_LOGCONFIG(LOGGING_TAG, "  Offline Hold: %s", this->offline_hold_ ? "true" : "false");
    ESP_LOGCONFIG(LOGGING_TAG, "  Offline Value: %s", this->offline_value_.c_str());
    ESP_LOGCONFIG(LOGGING_TAG, "  Payload location: %d", this->payload_location_);
    ESP_LOGCONFIG(LOGGING_TAG, "  Payload length: %d", this->payload_length_);
  }

 protected:
  std::string newest_value_{}, offline_value_{};

  void parse_ascii_payload(const std::vector<uint8_t> &payload) {
    this->newest_value_ = std::string(payload.begin() + this->payload_location_,
                                      payload.begin() + this->payload_location_ + this->payload_length_);
  }

  void parse_work_mode_payload(const std::vector<uint8_t> &payload) {
    uint32_t work_mode_code = this->parse_int(payload);
    if (work_mode_code > 2) {
      this->newest_value_ = "Unknown: " + std::to_string(work_mode_code);
    } else {
      this->newest_value_ = aa55_const::WORK_MODE_LIST[work_mode_code];
    }
  }

  void parse_error_codes_payload(const std::vector<uint8_t> &payload) {
    uint32_t error_codes_code = this->parse_int(payload);
    if (error_codes_code) {
      this->newest_value_ = "";
      for (uint8_t i = 0; i < 32; ++i) {
        if (error_codes_code & (1 << i)) {
          if (!this->newest_value_.empty()) {
            this->newest_value_ += ", ";
          }
          this->newest_value_ += aa55_const::ERROR_CODE_LIST[i];
        }
      }
    } else {
      this->newest_value_ = "No errors";
    }
  }
};

}  // namespace aa55_inverter
}  // namespace esphome
