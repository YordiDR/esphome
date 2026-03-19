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

  uint8_t get_payload_location() { return this->payload_location_; }

  uint8_t get_payload_length() { return this->payload_length_; }

  std::string get_id() { return this->id_; }

  void set_properties(std::string id, SENSOR_TYPE sensor_type) {
    this->id_ = id;
    this->type_ = sensor_type;
#define GOODWE_AA55_SET_SENSOR_PROPERTIES(sensor) \
  if (sensor_type == SENSOR_TYPE::sensor) { \
    this->payload_location_ = (uint8_t) SENSOR_PAYLOAD_LOCATION::sensor; \
    this->payload_length_ = (uint8_t) SENSOR_PAYLOAD_LENGTH::sensor; \
  }
    GOODWE_AA55_TEXT_SENSOR_LIST(GOODWE_AA55_SET_SENSOR_PROPERTIES, )
    GOODWE_AA55_SENSOR_LIST(GOODWE_AA55_SET_SENSOR_PROPERTIES, )
  }

 protected:
  uint16_t skip_updates_{0};
  uint16_t skipped_updates_{0};
  SENSOR_TYPE type_{};
  uint8_t payload_location_{};
  uint8_t payload_length_{};
  std::string id_{};
};

}  // namespace goodwe_aa55
}  // namespace esphome
