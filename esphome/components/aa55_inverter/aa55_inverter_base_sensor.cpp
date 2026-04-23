#include "esphome/core/log.h"
#include "aa55_inverter_base_sensor.h"

namespace esphome {
namespace aa55_inverter {
static const char *LOGGING_TAG = "aa55_sensor";

AA55InverterBaseSensor::AA55InverterBaseSensor(std::string id, aa55_inverter::SENSOR_TYPE type, uint16_t skip_updates,
                                               bool offline_hold) {
  this->id_ = id;
  this->type_ = type;
  this->skip_updates_ = skip_updates;
  this->offline_hold_ = offline_hold;
  this->payload_location_ = aa55_inverter::MAP_SENSOR_PAYLOAD_LOCATION.at(type);
  this->payload_length_ = aa55_inverter::MAP_SENSOR_PAYLOAD_LENGTH.at(type);
  this->payload_source_ = aa55_inverter::MAP_SENSOR_RESPONSE_SOURCE.at(type);
}

std::string AA55InverterBaseSensor::get_id() { return this->id_; }

aa55_inverter::SENSOR_TYPE AA55InverterBaseSensor::get_type() { return this->type_; }

aa55_bus::FUNCTION_CODE AA55InverterBaseSensor::get_payload_source() { return this->payload_source_; }

void AA55InverterBaseSensor::force_next_update() { this->force_next_update_ = true; }

bool AA55InverterBaseSensor::time_to_update() {
  return this->skipped_updates_ == this->skip_updates_ || this->force_next_update_;
}

uint32_t AA55InverterBaseSensor::parse_int(const std::vector<uint8_t> &payload) {
  uint32_t response = 0;

  // Safety check to prevent out-of-bounds crash
  if (this->payload_location_ + this->payload_length_ > payload.size()) {
    ESP_LOGE(LOGGING_TAG, "Buffer overflow in parse_int at index %d", this->payload_location_);
    return 0;
  }

  for (size_t i = 0; i < this->payload_length_; i++) {
    // Shift left 8 bits for each byte to maintain Big-Endian order
    response = (response << 8) | payload.at(this->payload_location_ + i);
  }

  return response;
}
}  // namespace aa55_inverter
}  // namespace esphome
