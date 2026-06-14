#pragma once
#include "const.h"
#include <string>

namespace esphome {
namespace pylontech_battery {

class PylontechBatteryBaseSensor {
 public:
  PylontechBatteryBaseSensor(const std::string &id, pylontech_battery::SENSOR_TYPE type, uint16_t skip_updates);
  const std::string &get_id();
  SENSOR_TYPE get_type();
  pylontech_group::COMMAND get_payload_source();
  virtual void process_response(const char *payload) = 0;

 protected:
  uint16_t skip_updates_{0};
  uint16_t skipped_updates_{0};
  SENSOR_TYPE type_{};
  std::string id_{};
  uint8_t payload_location_{};
  uint8_t payload_length_{};
  pylontech_group::COMMAND payload_source_{};

  bool time_to_update();
};

}  // namespace pylontech_battery
}  // namespace esphome
