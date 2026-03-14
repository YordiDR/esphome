#pragma once
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/core/component.h"
#include "../goodwe_aa55_base_sensor.h"
#include "../const.h"

namespace esphome {
namespace goodwe_aa55 {

class GoodweAA55TextSensor : public GoodweAA55BaseSensor, public text_sensor::TextSensor, public Component {
  void dump_config() override {
    ESP_LOGCONFIG(LOGGING_TAG, "Goodwe AA55 text sensor");
    ESP_LOGCONFIG(LOGGING_TAG, "  Skip Updates: %d", this->skip_updates_);
  }
};

}  // namespace goodwe_aa55
}  // namespace esphome
