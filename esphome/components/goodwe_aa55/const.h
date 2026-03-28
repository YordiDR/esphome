#pragma once
#include <cstdint>
#include <unordered_map>

namespace esphome {
namespace goodwe_aa55 {

static const char *LOGGING_TAG = "goodwe_aa55";
static const uint8_t MAX_BUFFER_LENGTH = 160;  // Max characters for serial buffer, 150 bytes is the length of the
                                               // response to the longest command (read running info list)
static const uint8_t INVERTER_OFFLINE_COUNTDOWN_RESET = 5;
static const uint8_t DEFAULT_ADDRESS = 0x7f;

const std::vector<uint8_t> EMPTY_VECTOR = {};
const std::vector<uint8_t> HEADERS = {0xaa, 0x55};
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

enum class CONTROL_CODE : uint8_t { REGISTER = 0x00, READ = 0x01, EXECUTE = 0x03 };
enum class FUNCTION_CODE : uint8_t {
  // Register function codes
  OFFLINE_QUERY = 0x00,
  ALLOC_REG_ADDR = 0x01,
  REMOVE_REG = 0x02,
  REG_REQUEST = 0x80,
  ADDR_CONFIRM = 0x81,
  REMOVE_CONFIRM = 0x82,

  // Query info function codes
  QUERY_RUN_INFO = 0x01,
  QUERY_ID_INFO = 0x02,
  QUERY_SET_INFO = 0x03,
  RUN_INFO_RESPONSE = 0x81,
  ID_INFO_RESPONSE = 0x82,
  SET_INFO_RESPONSE = 0x83
};
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
  SAFETY_COUNTRY_CODE
};
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
    {SENSOR_TYPE::SAFETY_COUNTRY_CODE, 63}};

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

    // Query ID info sensors
    {SENSOR_TYPE::FIRMWARE_VERSION, 5},
    {SENSOR_TYPE::MODEL, 10},
    {SENSOR_TYPE::SERIAL_NUMBER, 16},
    {SENSOR_TYPE::NOM_VPV, 4},
    {SENSOR_TYPE::INTERNAL_VERSION, 12},
    {SENSOR_TYPE::SAFETY_COUNTRY_CODE, 1}};

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

    // Query ID info sensors
    {SENSOR_TYPE::FIRMWARE_VERSION, ENCODING_TYPE::ASCII},
    {SENSOR_TYPE::MODEL, ENCODING_TYPE::ASCII},
    {SENSOR_TYPE::SERIAL_NUMBER, ENCODING_TYPE::ASCII},
    {SENSOR_TYPE::NOM_VPV, ENCODING_TYPE::ASCII},
    {SENSOR_TYPE::INTERNAL_VERSION, ENCODING_TYPE::ASCII},
    {SENSOR_TYPE::SAFETY_COUNTRY_CODE, ENCODING_TYPE::INTEGER}};
}  // namespace goodwe_aa55
}  // namespace esphome
