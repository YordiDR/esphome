#pragma once
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/component.h"
#include "../aa55_inverter_base_sensor.h"
#include <string>

namespace esphome {
namespace aa55_inverter {

class AA55InverterSensor : public AA55InverterBaseSensor, public sensor::Sensor, public Component {
 public:
  AA55InverterSensor(std::string id, aa55_const::SENSOR_TYPE type, uint16_t skip_updates)
      : AA55InverterBaseSensor(id, type, skip_updates), sensor::Sensor(), Component(){};

  void parse_payload(const std::vector<uint8_t> &payload) {
    ESP_LOGV(LOGGING_TAG, "Parsing %s from payload[%d], length %d bytes.", this->id_.c_str(), this->payload_location_,
             this->payload_length_);
    this->newest_value_ =
        this->parse_int(payload) / std::pow(10.0, (float) this->get_accuracy_decimals());  // Apply decimal precision
    ESP_LOGV(LOGGING_TAG, "Parsed %s: %f", this->id_.c_str(), this->newest_value_);
  }

  float get_newest_value() { return this->newest_value_; }

  void dump_config() override {
    ESP_LOGCONFIG(LOGGING_TAG, "Goodwe AA55 Inverter sensor");
    ESP_LOGCONFIG(LOGGING_TAG, "  Id: %s", this->id_.c_str());
    ESP_LOGCONFIG(LOGGING_TAG, "  Skip Updates: %d", this->skip_updates_);
    ESP_LOGCONFIG(LOGGING_TAG, "  Payload location: %d", this->payload_location_);
    ESP_LOGCONFIG(LOGGING_TAG, "  Payload length: %d", this->payload_length_);
  }

 protected:
  float newest_value_{NAN};
};

}  // namespace aa55_inverter
}  // namespace esphome
