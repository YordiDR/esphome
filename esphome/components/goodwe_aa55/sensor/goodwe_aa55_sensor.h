#pragma once
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/component.h"
#include "../goodwe_aa55_base_sensor.h"
#include "../const.h"

namespace esphome {
namespace goodwe_aa55 {

class GoodweAA55Sensor : public GoodweAA55BaseSensor, public sensor::Sensor, public Component {
 public:
  void parse_payload(const std::vector<uint8_t> &payload) {
    this->newest_value =
        this->parse_int(payload) / std::pow(10.0, (float) this->get_accuracy_decimals());  // Apply decimal precision
  }

  float get_newest_value() { return this->newest_value; }

  void dump_config() override {
    ESP_LOGCONFIG(LOGGING_TAG, "Goodwe AA55 text sensor");
    ESP_LOGCONFIG(LOGGING_TAG, "  Id: %s", this->id_);
    ESP_LOGCONFIG(LOGGING_TAG, "  Skip Updates: %d", this->skip_updates_);
    ESP_LOGCONFIG(LOGGING_TAG, "  Payload location: %d", MAP_SENSOR_PAYLOAD_LOCATION.at(this->type_));
    ESP_LOGCONFIG(LOGGING_TAG, "  Payload length: %d", MAP_SENSOR_PAYLOAD_LENGTH.at(this->type_));
  }

 protected:
  float newest_value{NAN};
};

}  // namespace goodwe_aa55
}  // namespace esphome
