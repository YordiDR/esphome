#pragma once
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/core/component.h"
#include "../goodwe_aa55_base_sensor.h"
#include "../const.h"
#include <string>

namespace esphome {
namespace goodwe_aa55 {

class GoodweAA55TextSensor : public GoodweAA55BaseSensor, public text_sensor::TextSensor, public Component {
 public:
  std::string newest_value{};
  uint32_t newest_value_code{};
  void map_code_to_string() {
    switch (this->type_) {
      case SENSOR_TYPE::WORK_MODE:
        this->map_work_mode_code();
        break;
      case SENSOR_TYPE::ERROR_CODES:
        this->map_error_codes_code();
        break;
    }
  }

  void dump_config() override {
    ESP_LOGCONFIG(LOGGING_TAG, "Goodwe AA55 text sensor");
    ESP_LOGCONFIG(LOGGING_TAG, "  Id: %s", this->id_);
    ESP_LOGCONFIG(LOGGING_TAG, "  Skip Updates: %d", this->skip_updates_);
    ESP_LOGCONFIG(LOGGING_TAG, "  Payload location: %d", this->payload_location_);
    ESP_LOGCONFIG(LOGGING_TAG, "  Payload length: %d", this->payload_length_);
  }

 protected:
  void map_work_mode_code() {
    if (this->newest_value_code > 2) {
      this->newest_value = "Unknown: " + std::to_string(this->newest_value_code);
    } else {
      this->newest_value = WORK_MODE_LIST[this->newest_value_code];
    }
  }

  void map_error_codes_code() {
    if (this->newest_value_code) {
      this->newest_value = "";
      for (uint8_t i = 0; i < 32; ++i) {
        if (this->newest_value_code & (1 << i)) {
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
