#include "esphome/core/log.h"
#include "pylontech_battery_text_sensor.h"

namespace esphome {
namespace pylontech_battery {
static constexpr const char *LOGGING_TAG = "pylontech_battery_sensor";

PylontechBatteryTextSensor::PylontechBatteryTextSensor(const std::string &id, pylontech_battery::SENSOR_TYPE type,
                                                       uint16_t skip_updates)
    : PylontechBatteryBaseSensor(id, type, skip_updates), text_sensor::TextSensor(), Component() {}

void PylontechBatteryTextSensor::process_response(const char *payload) {
  ESP_LOGV(LOGGING_TAG, "Checking if it's time to update text sensor %s: %s", this->id_.c_str(),
           this->time_to_update() ? "yes" : "no");

  if (this->time_to_update()) {
    ESP_LOGV(LOGGING_TAG, "Sensor %s would be updated from payload '%s'", this->id_.c_str(), payload);
    // ESP_LOGV(LOGGING_TAG, "Parsing text sensor %s from payload[%d], length %d bytes.", this->id_.c_str(),
    //          this->payload_location_, this->payload_length_);
    // this->publish_state(payload);

    if (this->skip_updates_ != 0)  // Reset skipped updates counter since we just updated
      this->skipped_updates_ = 0;
  } else {
    this->skipped_updates_++;  // Increment skipped updates counter since we skipped an update
  }
}

void PylontechBatteryTextSensor::dump_config() {
  ESP_LOGCONFIG(LOGGING_TAG, "Pylontech Battery text sensor");
  ESP_LOGCONFIG(LOGGING_TAG, "  Id: %s", this->id_.c_str());
  ESP_LOGCONFIG(LOGGING_TAG, "  Skip Updates: %d", this->skip_updates_);
  // ESP_LOGCONFIG(LOGGING_TAG, "  Payload location: %d", this->payload_location_);
  // ESP_LOGCONFIG(LOGGING_TAG, "  Payload length: %d", this->payload_length_);
}
}  // namespace pylontech_battery
}  // namespace esphome
