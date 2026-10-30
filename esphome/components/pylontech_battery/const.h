#pragma once
#include <cmath>
#include <cstdint>
#include "../pylontech_group/const.h"

namespace esphome {
namespace pylontech_battery {
static constexpr uint32_t BATTERY_OFFLINE_TIMEOUT =
    300000;  // Time in ms after which a battery is considered offline if no valid response has been received

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
  TEMPERATURE_STATE,
  // Static device information, from the `info` command
  DEVICE_NAME,
  BOARD_VERSION,
  BOARD,
  MAIN_SOFTWARE_VERSION,
  SOFTWARE_VERSION,
  BOOT_VERSION,
  COMM_VERSION,
  SERIAL_NUMBER
};

// Which console command provides the data for a sensor
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
      return pylontech_group::COMMAND::BAT;
    case SENSOR_TYPE::DEVICE_NAME:
    case SENSOR_TYPE::BOARD_VERSION:
    case SENSOR_TYPE::BOARD:
    case SENSOR_TYPE::MAIN_SOFTWARE_VERSION:
    case SENSOR_TYPE::SOFTWARE_VERSION:
    case SENSOR_TYPE::BOOT_VERSION:
    case SENSOR_TYPE::COMM_VERSION:
    case SENSOR_TYPE::SERIAL_NUMBER:
      return pylontech_group::COMMAND::INFO;
    default:
      return pylontech_group::COMMAND::UNKNOWN;
  }
}

static constexpr size_t STATE_TEXT_LENGTH = 16;
static constexpr size_t INFO_TEXT_LENGTH = 32;

// Values parsed from one console response. Missing floats are NaN, missing text is an empty string.
// Units are already converted: V, A, W, degC, %, and mV for the cell voltage delta.
struct BatteryData {
  float voltage{NAN};
  float current{NAN};
  float power{NAN};
  float temperature{NAN};
  float soc{NAN};
  float soh{NAN};
  float highest_cell_voltage{NAN};
  float lowest_cell_voltage{NAN};
  float cell_voltage_delta{NAN};  // mV
  char state[STATE_TEXT_LENGTH]{};
  char voltage_state[STATE_TEXT_LENGTH]{};
  char current_state[STATE_TEXT_LENGTH]{};
  char temperature_state[STATE_TEXT_LENGTH]{};
  char device_name[INFO_TEXT_LENGTH]{};
  char board_version[INFO_TEXT_LENGTH]{};
  char board[INFO_TEXT_LENGTH]{};
  char main_software_version[INFO_TEXT_LENGTH]{};
  char software_version[INFO_TEXT_LENGTH]{};
  char boot_version[INFO_TEXT_LENGTH]{};
  char comm_version[INFO_TEXT_LENGTH]{};
  char serial_number[INFO_TEXT_LENGTH]{};

  // Parser bookkeeping
  uint8_t parsed_fields{0};  // number of recognised lines in this response
  uint8_t cell_count{0};
  int32_t cell_min_mv{0};
  int32_t cell_max_mv{0};
};

}  // namespace pylontech_battery
}  // namespace esphome
