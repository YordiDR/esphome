#pragma once
#include <cstdint>
#include "../aa55_bus/const.h"

namespace esphome {
namespace aa55_inverter {
static constexpr uint32_t INVERTER_OFFLINE_TIMEOUT =
    30000;  // Time in ms after which an inverter is considered offline if no packets have been received

static constexpr const char *WORK_MODE_LIST[] = {"Waiting", "Normal", "Fault"};

static constexpr const char *ERROR_CODE_LIST[] = {"GFCI Device Failure",
                                                  "AC HCT Failure",
                                                  "Unknown bit 2",
                                                  "DCI Consistency Failure",
                                                  "GFCI Consistency Failure",
                                                  "Unknown bit 5",
                                                  "Unknown bit 6",
                                                  "Unknown bit 7",
                                                  "Unknown bit 8",
                                                  "Utility Loss",
                                                  "Ground I Failure",
                                                  "DC Bus High",
                                                  "Internal Version Mismatch",
                                                  "High Temperature",
                                                  "Auto Test Failure",
                                                  "PV Over Voltage",
                                                  "Fan Failure",
                                                  "Vac Failure",
                                                  "Isolation Failure",
                                                  "DC Injection High",
                                                  "Unknown bit 20",
                                                  "Unknown bit 21",
                                                  "Fac Consistency Failure",
                                                  "Vac Consistency Failure",
                                                  "Unknown bit 24",
                                                  "Relay Check Failure",
                                                  "Unknown bit 26",
                                                  "Unknown bit 27",
                                                  "Unknown bit 28",
                                                  "Fac Failure",
                                                  "EEPROM R/W Failure",
                                                  "Internal Communication Failure"};

enum class SENSOR_TYPE : uint8_t {
  VPV1,
  VPV2,
  IPV1,
  IPV2,
  VAC1,
  IAC1,
  FAC1,
  PAC,
  WORK_MODE,
  TEMPERATURE,
  ERROR_CODES,
  E_TOTAL,
  H_TOTAL,
  GFCI_FAULT_VALUE,
  E_TODAY,
  MODEL,
  SERIAL_NUMBER,
  COUNTRY_CODE
};
enum class INPUT_TYPE : uint8_t { START_STOP, RECONNECT_GRID, ADJUST_POWER };
enum class ON_OFF : uint8_t { OFF = 0, ON = 1 };

constexpr uint8_t get_sensor_payload_location(SENSOR_TYPE type) {
  switch (type) {
    case SENSOR_TYPE::VPV1:
      return 0;
    case SENSOR_TYPE::VPV2:
      return 2;
    case SENSOR_TYPE::IPV1:
      return 4;
    case SENSOR_TYPE::IPV2:
      return 6;
    case SENSOR_TYPE::VAC1:
      return 8;
    case SENSOR_TYPE::IAC1:
      return 10;
    case SENSOR_TYPE::FAC1:
      return 12;
    case SENSOR_TYPE::PAC:
      return 14;
    case SENSOR_TYPE::WORK_MODE:
      return 16;
    case SENSOR_TYPE::TEMPERATURE:
      return 18;
    case SENSOR_TYPE::ERROR_CODES:
      return 20;
    case SENSOR_TYPE::E_TOTAL:
      return 24;
    case SENSOR_TYPE::H_TOTAL:
      return 28;
    case SENSOR_TYPE::GFCI_FAULT_VALUE:
      return 42;
    case SENSOR_TYPE::E_TODAY:
      return 44;
    case SENSOR_TYPE::MODEL:
      return 5;
    case SENSOR_TYPE::SERIAL_NUMBER:
      return 31;
    case SENSOR_TYPE::COUNTRY_CODE:
      return 63;
    default:
      return 0;
  }
}

constexpr uint8_t get_sensor_payload_length(SENSOR_TYPE type) {
  switch (type) {
    case SENSOR_TYPE::VPV1:
    case SENSOR_TYPE::VPV2:
    case SENSOR_TYPE::IPV1:
    case SENSOR_TYPE::IPV2:
    case SENSOR_TYPE::VAC1:
    case SENSOR_TYPE::IAC1:
    case SENSOR_TYPE::FAC1:
    case SENSOR_TYPE::PAC:
    case SENSOR_TYPE::WORK_MODE:
    case SENSOR_TYPE::TEMPERATURE:
    case SENSOR_TYPE::GFCI_FAULT_VALUE:
    case SENSOR_TYPE::E_TODAY:
      return 2;
    case SENSOR_TYPE::ERROR_CODES:
      return 4;
    case SENSOR_TYPE::E_TOTAL:
      return 4;
    case SENSOR_TYPE::H_TOTAL:
      return 4;
    case SENSOR_TYPE::COUNTRY_CODE:
      return 1;
    case SENSOR_TYPE::MODEL:
      return 10;
    case SENSOR_TYPE::SERIAL_NUMBER:
      return 16;
    default:
      return 0;
  }
}

constexpr aa55_bus::FUNCTION_CODE get_sensor_response_source(SENSOR_TYPE type) {
  switch (type) {
    case SENSOR_TYPE::MODEL:
    case SENSOR_TYPE::SERIAL_NUMBER:
    case SENSOR_TYPE::COUNTRY_CODE:
      return aa55_bus::FUNCTION_CODE::ID_INFO_RESPONSE;
    default:
      return aa55_bus::FUNCTION_CODE::RUN_INFO_RESPONSE;
  }
}
}  // namespace aa55_inverter
}  // namespace esphome
