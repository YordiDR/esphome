#pragma once
#include <cstdint>
#include <unordered_map>
#include "../aa55_bus/const.h"

namespace esphome {
namespace aa55_const {

const std::vector<std::string> WORK_MODE_LIST = {"Waiting", "Normal", "Fault"};
const std::vector<std::string> ERROR_CODE_LIST = {"GFCI Device Failure",
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
  FIRMWARE_VERSION,
  MODEL,
  SERIAL_NUMBER,
  NOM_VPV,
  INTERNAL_VERSION,
  COUNTRY_CODE
};
enum class INPUT_TYPE : uint8_t { START_STOP, RECONNECT_GRID, ADJUST_POWER };
enum class ENCODING_TYPE : uint8_t { INTEGER, ASCII };

const std::unordered_map<SENSOR_TYPE, uint8_t> MAP_SENSOR_PAYLOAD_LOCATION = {
    // Query running info sensors
    {SENSOR_TYPE::VPV1, 0},
    {SENSOR_TYPE::VPV2, 2},
    {SENSOR_TYPE::IPV1, 4},
    {SENSOR_TYPE::IPV2, 6},
    {SENSOR_TYPE::VAC1, 8},
    {SENSOR_TYPE::IAC1, 10},
    {SENSOR_TYPE::FAC1, 12},
    {SENSOR_TYPE::PAC, 14},
    {SENSOR_TYPE::WORK_MODE, 16},
    {SENSOR_TYPE::TEMPERATURE, 18},
    {SENSOR_TYPE::ERROR_CODES, 20},
    {SENSOR_TYPE::E_TOTAL, 24},
    {SENSOR_TYPE::H_TOTAL, 28},
    {SENSOR_TYPE::GFCI_FAULT_VALUE, 42},
    {SENSOR_TYPE::E_TODAY, 44},

    // Query ID info sensors
    {SENSOR_TYPE::FIRMWARE_VERSION, 0},
    {SENSOR_TYPE::MODEL, 5},
    {SENSOR_TYPE::SERIAL_NUMBER, 31},
    {SENSOR_TYPE::NOM_VPV, 47},
    {SENSOR_TYPE::INTERNAL_VERSION, 51},
    {SENSOR_TYPE::COUNTRY_CODE, 63}};

const std::unordered_map<SENSOR_TYPE, uint8_t> MAP_SENSOR_PAYLOAD_LENGTH = {
    // Query running info sensors
    {SENSOR_TYPE::VPV1, 2},
    {SENSOR_TYPE::VPV2, 2},
    {SENSOR_TYPE::IPV1, 2},
    {SENSOR_TYPE::IPV2, 2},
    {SENSOR_TYPE::VAC1, 2},
    {SENSOR_TYPE::IAC1, 2},
    {SENSOR_TYPE::FAC1, 2},
    {SENSOR_TYPE::PAC, 2},
    {SENSOR_TYPE::WORK_MODE, 2},
    {SENSOR_TYPE::TEMPERATURE, 2},
    {SENSOR_TYPE::ERROR_CODES, 4},
    {SENSOR_TYPE::E_TOTAL, 4},
    {SENSOR_TYPE::H_TOTAL, 4},
    {SENSOR_TYPE::GFCI_FAULT_VALUE, 2},
    {SENSOR_TYPE::E_TODAY, 2},
    {SENSOR_TYPE::COUNTRY_CODE, 1},

    // Query ID info sensors
    {SENSOR_TYPE::MODEL, 10},
    {SENSOR_TYPE::SERIAL_NUMBER, 16}};

const std::unordered_map<SENSOR_TYPE, ENCODING_TYPE> MAP_SENSOR_ENCODING_TYPE = {
    // Query running info sensors
    {SENSOR_TYPE::VPV1, ENCODING_TYPE::INTEGER},
    {SENSOR_TYPE::VPV2, ENCODING_TYPE::INTEGER},
    {SENSOR_TYPE::IPV1, ENCODING_TYPE::INTEGER},
    {SENSOR_TYPE::IPV2, ENCODING_TYPE::INTEGER},
    {SENSOR_TYPE::VAC1, ENCODING_TYPE::INTEGER},
    {SENSOR_TYPE::IAC1, ENCODING_TYPE::INTEGER},
    {SENSOR_TYPE::FAC1, ENCODING_TYPE::INTEGER},
    {SENSOR_TYPE::PAC, ENCODING_TYPE::INTEGER},
    {SENSOR_TYPE::WORK_MODE, ENCODING_TYPE::INTEGER},
    {SENSOR_TYPE::TEMPERATURE, ENCODING_TYPE::INTEGER},
    {SENSOR_TYPE::ERROR_CODES, ENCODING_TYPE::INTEGER},
    {SENSOR_TYPE::E_TOTAL, ENCODING_TYPE::INTEGER},
    {SENSOR_TYPE::H_TOTAL, ENCODING_TYPE::INTEGER},
    {SENSOR_TYPE::GFCI_FAULT_VALUE, ENCODING_TYPE::INTEGER},
    {SENSOR_TYPE::E_TODAY, ENCODING_TYPE::INTEGER},
    {SENSOR_TYPE::COUNTRY_CODE, ENCODING_TYPE::INTEGER},

    // Query ID info sensors
    {SENSOR_TYPE::MODEL, ENCODING_TYPE::ASCII},
    {SENSOR_TYPE::SERIAL_NUMBER, ENCODING_TYPE::ASCII}};

const std::unordered_map<SENSOR_TYPE, FUNCTION_CODE> MAP_SENSOR_RESPONSE_SOURCE = {
    // Query running info sensors
    {SENSOR_TYPE::VPV1, FUNCTION_CODE::RUN_INFO_RESPONSE},
    {SENSOR_TYPE::VPV2, FUNCTION_CODE::RUN_INFO_RESPONSE},
    {SENSOR_TYPE::IPV1, FUNCTION_CODE::RUN_INFO_RESPONSE},
    {SENSOR_TYPE::IPV2, FUNCTION_CODE::RUN_INFO_RESPONSE},
    {SENSOR_TYPE::VAC1, FUNCTION_CODE::RUN_INFO_RESPONSE},
    {SENSOR_TYPE::IAC1, FUNCTION_CODE::RUN_INFO_RESPONSE},
    {SENSOR_TYPE::FAC1, FUNCTION_CODE::RUN_INFO_RESPONSE},
    {SENSOR_TYPE::PAC, FUNCTION_CODE::RUN_INFO_RESPONSE},
    {SENSOR_TYPE::WORK_MODE, FUNCTION_CODE::RUN_INFO_RESPONSE},
    {SENSOR_TYPE::TEMPERATURE, FUNCTION_CODE::RUN_INFO_RESPONSE},
    {SENSOR_TYPE::ERROR_CODES, FUNCTION_CODE::RUN_INFO_RESPONSE},
    {SENSOR_TYPE::E_TOTAL, FUNCTION_CODE::RUN_INFO_RESPONSE},
    {SENSOR_TYPE::H_TOTAL, FUNCTION_CODE::RUN_INFO_RESPONSE},
    {SENSOR_TYPE::GFCI_FAULT_VALUE, FUNCTION_CODE::RUN_INFO_RESPONSE},
    {SENSOR_TYPE::E_TODAY, FUNCTION_CODE::RUN_INFO_RESPONSE},

    // Query ID info sensors
    {SENSOR_TYPE::MODEL, FUNCTION_CODE::ID_INFO_RESPONSE},
    {SENSOR_TYPE::SERIAL_NUMBER, FUNCTION_CODE::ID_INFO_RESPONSE},
    {SENSOR_TYPE::COUNTRY_CODE, FUNCTION_CODE::ID_INFO_RESPONSE}};

const std::unordered_map<INPUT_TYPE, FUNCTION_CODE> MAP_INPUT_RESPONSE = {
    {INPUT_TYPE::START_STOP, FUNCTION_CODE::START_INVERTER_RESPONSE},
    {INPUT_TYPE::START_STOP, FUNCTION_CODE::STOP_INVERTER_RESPONSE},
    {INPUT_TYPE::RECONNECT_GRID, FUNCTION_CODE::RECONNECT_GRID_RESPONSE},
    {INPUT_TYPE::ADJUST_POWER, FUNCTION_CODE::ADJUST_POWER_RESPONSE}};
}  // namespace aa55_const
}  // namespace esphome
