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

  // Register function codes
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
  E_TODAY
};

const std::unordered_map<SENSOR_TYPE, uint8_t> MAP_SENSOR_PAYLOAD_LOCATION = {
    {SENSOR_TYPE::VPV1, 0},         {SENSOR_TYPE::VPV2, 2},
    {SENSOR_TYPE::IPV1, 4},         {SENSOR_TYPE::IPV2, 6},
    {SENSOR_TYPE::VAC1, 8},         {SENSOR_TYPE::IAC1, 10},
    {SENSOR_TYPE::FAC1, 12},        {SENSOR_TYPE::PAC, 14},
    {SENSOR_TYPE::WORK_MODE, 16},   {SENSOR_TYPE::TEMPERATURE, 18},
    {SENSOR_TYPE::ERROR_CODES, 20}, {SENSOR_TYPE::E_TOTAL, 24},
    {SENSOR_TYPE::H_TOTAL, 28},     {SENSOR_TYPE::GFCI_FAULT_VALUE, 42},
    {SENSOR_TYPE::E_TODAY, 44}};

const std::unordered_map<SENSOR_TYPE, uint8_t> MAP_SENSOR_PAYLOAD_LENGTH = {{SENSOR_TYPE::VPV2, 2},
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
                                                                            {SENSOR_TYPE::E_TODAY, 2}};
}  // namespace goodwe_aa55
}  // namespace esphome
