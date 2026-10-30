#include "esphome/core/log.h"
#include "pylontech_battery_sensor.h"

namespace esphome {
namespace pylontech_battery {
static constexpr const char *LOGGING_TAG = "pylontech_battery_sensor";

PylontechBatterySensor::PylontechBatterySensor(const std::string &id, pylontech_battery::SENSOR_TYPE type,
                                               uint16_t skip_updates)
    : PylontechBatteryBaseSensor(id, type, skip_updates), sensor::Sensor(), Component() {}

void PylontechBatterySensor::process_data(const BatteryData &data) {
  float value = NAN;
  switch (this->type_) {
    case SENSOR_TYPE::VOLTAGE:
      value = data.voltage;
      break;
    case SENSOR_TYPE::CURRENT:
      value = data.current;
      break;
    case SENSOR_TYPE::POWER:
      value = data.power;
      break;
    case SENSOR_TYPE::SOC:
      value = data.soc;
      break;
    case SENSOR_TYPE::SOH:
      value = data.soh;
      break;
    case SENSOR_TYPE::TEMPERATURE:
      value = data.temperature;
      break;
    case SENSOR_TYPE::HIGHEST_CELL_VOLTAGE:
      value = data.highest_cell_voltage;
      break;
    case SENSOR_TYPE::LOWEST_CELL_VOLTAGE:
      value = data.lowest_cell_voltage;
      break;
    case SENSOR_TYPE::CELL_VOLTAGE_DELTA:
      value = data.cell_voltage_delta;
      break;
    default:
      return;
  }
  if (std::isnan(value)) {
    ESP_LOGV(LOGGING_TAG, "Sensor %s: value missing from response", this->id_.c_str());
    return;
  }
  ESP_LOGV(LOGGING_TAG, "Sensor %s: publishing %.3f", this->id_.c_str(), value);
  this->publish_state(value);
}

void PylontechBatterySensor::on_offline() { this->publish_state(NAN); }

void PylontechBatterySensor::dump_config() {
  ESP_LOGCONFIG(LOGGING_TAG, "Pylontech Battery sensor");
  ESP_LOGCONFIG(LOGGING_TAG, "  Id: %s", this->id_.c_str());
  ESP_LOGCONFIG(LOGGING_TAG, "  Skip Updates: %d", this->skip_updates_);
}

}  // namespace pylontech_battery
}  // namespace esphome
