#pragma once
#include "esphome/components/number/number.h"
#include "esphome/core/component.h"
#include <string>

namespace esphome {
namespace aa55_inverter {

class AA55InverterNumber : public number::Number, public Component {
 public:
  void dump_config() override {
    ESP_LOGCONFIG(LOGGING_TAG, "Goodwe AA55 Inverter text sensor");
    ESP_LOGCONFIG(LOGGING_TAG, "  Id: %s", this->id_);
    ESP_LOGCONFIG(LOGGING_TAG, "  Skip Updates: %d", this->skip_updates_);
    ESP_LOGCONFIG(LOGGING_TAG, "  Payload location: %d", aa55_const::MAP_SENSOR_PAYLOAD_LOCATION.at(this->type_));
    ESP_LOGCONFIG(LOGGING_TAG, "  Payload length: %d", aa55_const::MAP_SENSOR_PAYLOAD_LENGTH.at(this->type_));
  }

  aa55_const::NUMBER_TYPE get_type() { return this->type_; }

  void set_type(aa55_const::NUMBER_TYPE type) { this->type_ = type; }

  std::string get_id() { return this->id_; }

  void set_id(std::string id) { this->id_ = id; }

 protected:
  aa55_const::NUMBER_TYPE type_{};
  std::string id_{};
};

}  // namespace aa55_inverter
}  // namespace esphome
