#pragma once
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/component.h"
#include "../goodwe_aa55_base_sensor.h"
#include "../const.h"

namespace esphome {
namespace goodwe_aa55 {

class GoodweAA55Sensor : public GoodweAA55BaseSensor, public sensor::Sensor, public Component {
 public:
  float newest_value{NAN};

  void dump_config() override {
    ESP_LOGCONFIG(LOGGING_TAG, "Goodwe AA55 text sensor");
    ESP_LOGCONFIG(LOGGING_TAG, "  Id: %s", this->id_);
    ESP_LOGCONFIG(LOGGING_TAG, "  Skip Updates: %d", this->skip_updates_);
    ESP_LOGCONFIG(LOGGING_TAG, "  Payload location: %d", this->payload_location_);
    ESP_LOGCONFIG(LOGGING_TAG, "  Payload length: %d", this->payload_length_);
  }
};

}  // namespace goodwe_aa55
}  // namespace esphome
