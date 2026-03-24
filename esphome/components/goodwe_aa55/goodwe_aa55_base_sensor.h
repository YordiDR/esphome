#pragma once
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/component.h"
#include "const.h"

namespace esphome {
namespace goodwe_aa55 {

class GoodweAA55BaseSensor {
 public:
  uint16_t get_skip_updates() { return this->skip_updates_; }

  void set_skip_updates(uint16_t skip_updates) { this->skip_updates_ = skip_updates; }

  uint16_t get_skipped_updates() { return this->skipped_updates_; }

  uint16_t increment_skipped_updates() { return ++this->skipped_updates_; }

  void reset_skipped_updates() { this->skipped_updates_ = 0; }

  bool time_to_update() { return this->skipped_updates_ == this->skip_updates_; }

  SENSOR_TYPE get_type() { return this->type_; }

  void set_type(SENSOR_TYPE type) { this->type_ = type; }

  std::string get_id() { return this->id_; }

  void set_id(std::string id) { this->id_ = id; }

  uint8_t get_payload_location() { return MAP_SENSOR_PAYLOAD_LOCATION.at(this->type_); }

  uint8_t get_payload_length() { return MAP_SENSOR_PAYLOAD_LENGTH.at(this->type_); }

 protected:
  uint16_t skip_updates_{0};
  uint16_t skipped_updates_{0};
  SENSOR_TYPE type_{};
  std::string id_{};
};

}  // namespace goodwe_aa55
}  // namespace esphome
