#include "esphome/core/log.h"
#include "pylontech_battery_base_sensor.h"

namespace esphome {
namespace pylontech_battery {
static constexpr const char *LOGGING_TAG = "pylontech_sensor";

PylontechBatteryBaseSensor::PylontechBatteryBaseSensor(const std::string &id, pylontech_battery::SENSOR_TYPE type,
                                                       uint16_t skip_updates) {
  this->id_ = id;
  this->type_ = type;
  this->skip_updates_ = skip_updates;
  // this->payload_location_ = get_sensor_payload_location(type);
  // this->payload_length_ = get_sensor_payload_length(type);
  this->payload_source_ = get_sensor_response_source(type);
}

const std::string &PylontechBatteryBaseSensor::get_id() { return this->id_; }

SENSOR_TYPE PylontechBatteryBaseSensor::get_type() { return this->type_; }

pylontech_group::COMMAND PylontechBatteryBaseSensor::get_payload_source() { return this->payload_source_; }

bool PylontechBatteryBaseSensor::time_to_update() { return this->skipped_updates_ == this->skip_updates_; }
}  // namespace pylontech_battery
}  // namespace esphome
