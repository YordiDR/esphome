#include "esphome/core/log.h"
#include "pylontech_battery_text_sensor.h"

namespace esphome {
namespace pylontech_battery {
static constexpr const char *LOGGING_TAG = "pylontech_battery_text_sensor";

PylontechBatteryTextSensor::PylontechBatteryTextSensor(const std::string &id, pylontech_battery::SENSOR_TYPE type,
                                                       uint16_t skip_updates)
    : PylontechBatteryBaseSensor(id, type, skip_updates), text_sensor::TextSensor(), Component() {}

void PylontechBatteryTextSensor::process_data(const BatteryData &data) {
  const char *text = nullptr;
  switch (this->type_) {
    case SENSOR_TYPE::STATE:
      text = data.state;
      break;
    case SENSOR_TYPE::VOLTAGE_STATE:
      text = data.voltage_state;
      break;
    case SENSOR_TYPE::CURRENT_STATE:
      text = data.current_state;
      break;
    case SENSOR_TYPE::TEMPERATURE_STATE:
      text = data.temperature_state;
      break;
    case SENSOR_TYPE::DEVICE_NAME:
      text = data.device_name;
      break;
    case SENSOR_TYPE::BOARD_VERSION:
      text = data.board_version;
      break;
    case SENSOR_TYPE::BOARD:
      text = data.board;
      break;
    case SENSOR_TYPE::MAIN_SOFTWARE_VERSION:
      text = data.main_software_version;
      break;
    case SENSOR_TYPE::SOFTWARE_VERSION:
      text = data.software_version;
      break;
    case SENSOR_TYPE::BOOT_VERSION:
      text = data.boot_version;
      break;
    case SENSOR_TYPE::COMM_VERSION:
      text = data.comm_version;
      break;
    case SENSOR_TYPE::SERIAL_NUMBER:
      text = data.serial_number;
      break;
    default:
      return;
  }
  if (text == nullptr || text[0] == '\0') {
    ESP_LOGV(LOGGING_TAG, "Text sensor %s: value missing from response", this->id_.c_str());
    return;
  }
  ESP_LOGV(LOGGING_TAG, "Text sensor %s: publishing '%s'", this->id_.c_str(), text);
  this->publish_state(text);
}

void PylontechBatteryTextSensor::dump_config() {
  ESP_LOGCONFIG(LOGGING_TAG, "Pylontech Battery text sensor");
  ESP_LOGCONFIG(LOGGING_TAG, "  Id: %s", this->id_.c_str());
  ESP_LOGCONFIG(LOGGING_TAG, "  Skip Updates: %d", this->skip_updates_);
}

}  // namespace pylontech_battery
}  // namespace esphome
