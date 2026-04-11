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

  void process_response(const std::vector<uint8_t> &payload) override {
    ESP_LOGV(LOGGING_TAG, "Checking if it's time to update text sensor %s: %s", this->id_.c_str(),
             this->time_to_update() ? "yes" : "no");

    if (this->time_to_update()) {
      ESP_LOGV(LOGGING_TAG, "Parsing text sensor%s from payload[%d], length %d bytes.", this->id_.c_str(), this->payload_location_,
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

      if (this->skip_updates_ != 0) { // Reset skipped updates counter since we just updated
        this->skipped_updates_ = 0;
      }

      if (this->force_next_update_) { // Reset force next update flag since we just updated
        this->force_next_update_ = false;
      }
    } else {
      this->skipped_updates_++; // Increment skipped updates counter since we skipped an update
    }
  }

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
  std::string offline_value_{};

  void parse_ascii_payload(const std::vector<uint8_t> &payload) {
    this->publish_state(std::string(payload.begin() + this->payload_location_,
                                    payload.begin() + this->payload_location_ + this->payload_length_));
  }

  void parse_work_mode_payload(const std::vector<uint8_t> &payload) {
    uint32_t work_mode_code = this->parse_int(payload);
    if (work_mode_code > 2) {
      this->publish_state("Unknown: " + std::to_string(work_mode_code));
    } else {
      this->publish_state(aa55_const::WORK_MODE_LIST[work_mode_code]);
    }
  }

  void parse_error_codes_payload(const std::vector<uint8_t> &payload) {
    uint32_t error_codes_code = this->parse_int(payload);
    if (error_codes_code) {
      std::string error_codes_string = "";
      for (uint8_t i = 0; i < 32; ++i) {
        if (error_codes_code & (1 << i)) {
          if (!error_codes_string.empty()) {
            error_codes_string += ", ";
          }
          error_codes_string += aa55_const::ERROR_CODE_LIST[i];
        }

        this->publish_state(error_codes_string);
      }
    } else {
      this->publish_state("No errors");
    }
  }
};

}  // namespace aa55_inverter
}  // namespace esphome
