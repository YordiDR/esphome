#pragma once
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/component.h"
#include "../aa55_inverter_base_sensor.h"
#include <string>

namespace esphome {
namespace aa55_inverter {

class AA55InverterSensor : public AA55InverterBaseSensor, public sensor::Sensor, public Component {
 public:
  void parse_payload(const std::vector<uint8_t> &payload) {
    this->newest_value_ =
        this->parse_int(payload) / std::pow(10.0, (float) this->get_accuracy_decimals());  // Apply decimal precision
  }

  float get_newest_value() { return this->newest_value_; }

  void dump_config() override {
    ESP_LOGCONFIG(LOGGING_TAG, "Goodwe AA55 Inverter text sensor");
    ESP_LOGCONFIG(LOGGING_TAG, "  Id: %s", this->id_);
    ESP_LOGCONFIG(LOGGING_TAG, "  Skip Updates: %d", this->skip_updates_);
    ESP_LOGCONFIG(LOGGING_TAG, "  Payload location: %d", aa55_const::MAP_SENSOR_PAYLOAD_LOCATION.at(this->type_));
    ESP_LOGCONFIG(LOGGING_TAG, "  Payload length: %d", aa55_const::MAP_SENSOR_PAYLOAD_LENGTH.at(this->type_));
  }

 protected:
  float newest_value_{NAN};
};

}  // namespace aa55_inverter
}  // namespace esphome
