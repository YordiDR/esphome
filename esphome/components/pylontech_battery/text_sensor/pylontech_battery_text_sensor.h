#pragma once
#include <string>
#include "esphome/core/component.h"
#include "../../text_sensor/text_sensor.h"
#include "../pylontech_battery_base_sensor.h"

namespace esphome {
namespace pylontech_battery {
class PylontechBatteryTextSensor : public PylontechBatteryBaseSensor, public text_sensor::TextSensor, public Component {
 public:
  PylontechBatteryTextSensor(const std::string &id, pylontech_battery::SENSOR_TYPE type, uint16_t skip_updates);
  void process_response(const char *payload) override;
  void dump_config() override;
};
}  // namespace pylontech_battery
}  // namespace esphome
