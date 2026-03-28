#pragma once
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/core/component.h"
#include "../goodwe_aa55_base_sensor.h"
#include "../const.h"
#include <string>
#include <vector>

namespace esphome {
namespace goodwe_aa55 {

class GoodweAA55TextSensor : public GoodweAA55BaseSensor, public text_sensor::TextSensor, public Component {
 public:
  void parse_payload(const std::vector<uint8_t> &payload) {
    switch (this->type_) {
      case SENSOR_TYPE::WORK_MODE:
        this->parse_work_mode_payload(payload);
        break;
      case SENSOR_TYPE::ERROR_CODES:
        this->parse_error_codes_payload(payload);
        break;
      case SENSOR_TYPE::FIRMWARE_VERSION:
      case SENSOR_TYPE::MODEL:
      case SENSOR_TYPE::SERIAL_NUMBER:
      case SENSOR_TYPE::NOM_VPV:
      case SENSOR_TYPE::INTERNAL_VERSION:
      case SENSOR_TYPE::SAFETY_COUNTRY_CODE:
        this->parse_ascii_payload(payload);
    }
  }

  std::string get_newest_value() { return this->newest_value; }

  void dump_config() override {
    ESP_LOGCONFIG(LOGGING_TAG, "Goodwe AA55 text sensor");
    ESP_LOGCONFIG(LOGGING_TAG, "  Id: %s", this->id_);
    ESP_LOGCONFIG(LOGGING_TAG, "  Skip Updates: %d", this->skip_updates_);
    ESP_LOGCONFIG(LOGGING_TAG, "  Payload location: %d", MAP_SENSOR_PAYLOAD_LOCATION.at(this->type_));
    ESP_LOGCONFIG(LOGGING_TAG, "  Payload length: %d", MAP_SENSOR_PAYLOAD_LENGTH.at(this->type_));
  }

 protected:
  std::string newest_value{};

  void parse_ascii_payload(const std::vector<uint8_t> &payload) {
    this->newest_value = std::string(payload.begin() + this->get_payload_location(),
                                     payload.begin() + this->get_payload_location() + this->get_payload_length());
  }

  void parse_work_mode_payload(const std::vector<uint8_t> &payload) {
    uint32_t work_mode_code = this->parse_int(payload);
    if (work_mode_code > 2) {
      this->newest_value = "Unknown: " + std::to_string(work_mode_code);
    } else {
      this->newest_value = WORK_MODE_LIST[work_mode_code];
    }
  }

  void parse_error_codes_payload(const std::vector<uint8_t> &payload) {
    uint32_t error_codes_code = this->parse_int(payload);
    if (error_codes_code) {
      this->newest_value = "";
      for (uint8_t i = 0; i < 32; ++i) {
        if (error_codes_code & (1 << i)) {
          if (!this->newest_value.empty()) {
            this->newest_value += ", ";
          }
          this->newest_value += ERROR_CODE_LIST[i];
        }
      }
    } else {
      this->newest_value = "No errors";
    }
  }
};

}  // namespace goodwe_aa55
}  // namespace esphome
