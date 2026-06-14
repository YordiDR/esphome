#pragma once
#include <cstdint>
#include "../pylontech_group/const.h"

namespace esphome {
namespace pylontech_battery {
static constexpr uint32_t BATTERY_OFFLINE_TIMEOUT =
    300000;  // Time in ms after which a battery is considered offline if no packets have been received

enum class SENSOR_TYPE : uint8_t {
  VOLTAGE,
  CURRENT,
  POWER,
  STATE,
  SOC,
  SOH,
  TEMPERATURE,
  HIGHEST_CELL_VOLTAGE,
  LOWEST_CELL_VOLTAGE,
  CELL_VOLTAGE_DELTA,
  VOLTAGE_STATE,
  CURRENT_STATE,
  TEMPERATURE_STATE
};

constexpr uint8_t get_sensor_payload_location(SENSOR_TYPE type) {
  switch (type) {
    case SENSOR_TYPE::VOLTAGE:
      return 0;
    case SENSOR_TYPE::CURRENT:
      return 2;
    case SENSOR_TYPE::POWER:
      return 4;
    case SENSOR_TYPE::STATE:
      return 6;
    case SENSOR_TYPE::SOC:
      return 8;
    case SENSOR_TYPE::SOH:
      return 10;
    case SENSOR_TYPE::TEMPERATURE:
      return 12;
    case SENSOR_TYPE::HIGHEST_CELL_VOLTAGE:
      return 14;
    case SENSOR_TYPE::LOWEST_CELL_VOLTAGE:
      return 16;
    case SENSOR_TYPE::CELL_VOLTAGE_DELTA:
      return 18;
    case SENSOR_TYPE::VOLTAGE_STATE:
      return 20;
    case SENSOR_TYPE::CURRENT_STATE:
      return 24;
    case SENSOR_TYPE::TEMPERATURE_STATE:
      return 28;
    default:
      return 0;
  }
}

constexpr uint8_t get_sensor_payload_length(SENSOR_TYPE type) {
  switch (type) {
    case SENSOR_TYPE::VOLTAGE:
      return 4;
    case SENSOR_TYPE::CURRENT:
      return 3;
    case SENSOR_TYPE::POWER:
      return 7;
    case SENSOR_TYPE::STATE:
      return 6;
    case SENSOR_TYPE::SOC:
    case SENSOR_TYPE::SOH:
      return 3;
    case SENSOR_TYPE::TEMPERATURE:
      return 5;
    case SENSOR_TYPE::HIGHEST_CELL_VOLTAGE:
    case SENSOR_TYPE::LOWEST_CELL_VOLTAGE:
    case SENSOR_TYPE::CELL_VOLTAGE_DELTA:
      return 4;
    case SENSOR_TYPE::VOLTAGE_STATE:
    case SENSOR_TYPE::CURRENT_STATE:
    case SENSOR_TYPE::TEMPERATURE_STATE:
      return 6;
    default:
      return 0;
  }
}

constexpr pylontech_group::COMMAND get_sensor_response_source(SENSOR_TYPE type) {
  switch (type) {
    case SENSOR_TYPE::VOLTAGE:
    case SENSOR_TYPE::CURRENT:
    case SENSOR_TYPE::POWER:
    case SENSOR_TYPE::STATE:
    case SENSOR_TYPE::SOC:
    case SENSOR_TYPE::TEMPERATURE:
    case SENSOR_TYPE::VOLTAGE_STATE:
    case SENSOR_TYPE::CURRENT_STATE:
    case SENSOR_TYPE::TEMPERATURE_STATE:
      return pylontech_group::COMMAND::PWR;
    case SENSOR_TYPE::SOH:
      return pylontech_group::COMMAND::STAT;
    case SENSOR_TYPE::HIGHEST_CELL_VOLTAGE:
    case SENSOR_TYPE::LOWEST_CELL_VOLTAGE:
    case SENSOR_TYPE::CELL_VOLTAGE_DELTA:
      return pylontech_group::COMMAND::UNKNOWN;
    default:
      return pylontech_group::COMMAND::UNKNOWN;
  }
}
}  // namespace pylontech_battery
}  // namespace esphome
