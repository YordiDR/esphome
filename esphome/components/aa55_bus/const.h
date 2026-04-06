#pragma once
#include <cstdint>

namespace esphome {
namespace aa55_const {

static const uint16_t MAX_BUFFER_LENGTH = 512;  // Max characters for serial buffer, 150 bytes is the length of the
                                                // response to the longest command (read running info list)
static const uint32_t OFFLINE_QUERY_INTERVAL =
    60000;  // Time interval in ms for sending offline query commands for unregistered inverters
static const uint8_t DEFAULT_INVERTER_ADDRESS = 0x7F;
static const uint16_t COMMAND_DELAY = 500;  // Min time between AA55 commands is 500 ms according to AA55 documentation

static const std::vector<uint8_t> HEADERS = {0xAA, 0x55};
static const std::vector<uint8_t> EMPTY_VECTOR = {};

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
  SET_INFO_RESPONSE = 0x83,

  // Execute function codes
  START_INVERTER = 0x1B,
  START_INVERTER_RESPONSE = 0x9B,
  STOP_INVERTER = 0x1C,
  STOP_INVERTER_RESPONSE = 0x9C,
  RECONNECT_GRID = 0x1D,
  RECONNECT_GRID_RESPONSE = 0x9D,
  ADJUST_POWER = 0x1E,
  ADJUST_POWER_RESPONSE = 0x9E
};

struct AA55Packet {
  const uint8_t source_address;
  const uint8_t destination_address;
  const CONTROL_CODE control_code;
  const FUNCTION_CODE function_code;
  const std::vector<uint8_t> payload;
};
}  // namespace aa55_const
}  // namespace esphome
