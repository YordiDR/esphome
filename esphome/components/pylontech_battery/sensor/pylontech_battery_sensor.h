#pragma once
#include <string>
#include "esphome/core/component.h"
#include "../../sensor/sensor.h"
#include "../pylontech_battery_base_sensor.h"

namespace esphome {
namespace pylontech_battery {

class PylontechBatterySensor : public PylontechBatteryBaseSensor, public sensor::Sensor, public Component {
 public:
  PylontechBatterySensor(const std::string &id, pylontech_battery::SENSOR_TYPE type, uint16_t skip_updates);
  void process_response(const char *payload) override;
  void dump_config() override;
};

}  // namespace pylontech_battery
}  // namespace esphome
