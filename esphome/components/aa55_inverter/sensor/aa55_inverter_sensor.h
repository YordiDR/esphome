#pragma once
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/component.h"
#include "../aa55_inverter_base_sensor.h"
#include <string>

namespace esphome {
namespace aa55_inverter {

class AA55InverterSensor : public AA55InverterBaseSensor, public sensor::Sensor, public Component {
 public:
  AA55InverterSensor(std::string id, aa55_const::SENSOR_TYPE type, uint16_t skip_updates, bool offline_hold,
                     float offline_value)
      : AA55InverterBaseSensor(id, type, skip_updates, offline_hold), sensor::Sensor(), Component() {
    this->offline_value_ = offline_value;
  };

  void parse_payload(const std::vector<uint8_t> &payload) override {
    ESP_LOGV(LOGGING_TAG, "Parsing %s from payload[%d], length %d bytes.", this->id_.c_str(), this->payload_location_,
             this->payload_length_);
    this->newest_value_ =
        this->parse_int(payload) / std::pow(10.0, (float) this->get_accuracy_decimals());  // Apply decimal precision
    ESP_LOGV(LOGGING_TAG, "Parsed %s: %f", this->id_.c_str(), this->newest_value_);
  }

  void emit_state() override { this->publish_state(this->newest_value_); }

  void handle_inverter_offline() override {
    if (!this->offline_hold_) {
      ESP_LOGD(LOGGING_TAG, "Publishing offline value %f for sensor %s because the inverter stopped responding.",
               this->offline_value_, this->id_.c_str());
      this->publish_state(this->offline_value_);
    }
  }

  void dump_config() override {
    ESP_LOGCONFIG(LOGGING_TAG, "Goodwe AA55 Inverter sensor");
    ESP_LOGCONFIG(LOGGING_TAG, "  Id: %s", this->id_.c_str());
    ESP_LOGCONFIG(LOGGING_TAG, "  Skip Updates: %d", this->skip_updates_);
    ESP_LOGCONFIG(LOGGING_TAG, "  Offline Hold: %s", this->offline_hold_ ? "true" : "false");
    ESP_LOGCONFIG(LOGGING_TAG, "  Offline Value: %f", this->offline_value_);
    ESP_LOGCONFIG(LOGGING_TAG, "  Payload location: %d", this->payload_location_);
    ESP_LOGCONFIG(LOGGING_TAG, "  Payload length: %d", this->payload_length_);
  }

 protected:
  float newest_value_{NAN}, offline_value_{};
};

}  // namespace aa55_inverter
}  // namespace esphome
