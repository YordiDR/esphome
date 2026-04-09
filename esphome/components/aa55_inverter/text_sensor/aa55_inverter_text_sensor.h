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
  AA55InverterTextSensor(std::string id, aa55_const::SENSOR_TYPE type, uint16_t skip_updates)
      : AA55InverterBaseSensor(id, type, skip_updates), text_sensor::TextSensor(), Component(){};

  void parse_payload(const std::vector<uint8_t> &payload) {
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

  std::string get_newest_value() { return this->newest_value_; }

  void dump_config() override {
    ESP_LOGCONFIG(LOGGING_TAG, "Goodwe AA55 text sensor");
    ESP_LOGCONFIG(LOGGING_TAG, "  Id: %s", this->id_.c_str());
    ESP_LOGCONFIG(LOGGING_TAG, "  Skip Updates: %d", this->skip_updates_);
    ESP_LOGCONFIG(LOGGING_TAG, "  Payload location: %d", this->payload_location_);
    ESP_LOGCONFIG(LOGGING_TAG, "  Payload length: %d", this->payload_length_);
  }

 protected:
  std::string newest_value_{};

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
