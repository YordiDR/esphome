#pragma once
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

namespace esphome {
namespace pylontech_group {

static constexpr size_t READ_BATCH_SIZE = 128;
static constexpr size_t MAX_QUEUED_TX_COMMANDS = 16;  // Max amount of commands that can be queued to send over UART
static constexpr size_t MAX_LINE_LENGTH = 192;        // Longest console line is the "Protect ENA" line (~140 chars)
static constexpr uint8_t MAX_COMMAND_LENGTH = 16;     // Max characters for stringified command
static constexpr uint16_t COMMAND_DELAY = 200;        // Min time between RS232 commands in milliseconds
static constexpr uint32_t RESPONSE_TIMEOUT = 3000;    // Give up waiting for "$$" after this many milliseconds
static constexpr uint32_t RX_TIME_BUDGET = 30;        // Max milliseconds spent reading UART per loop() call
static constexpr const char *PYLON_COMMAND_START = "@";
static constexpr const char *PYLON_COMMAND_END = "$$";
static constexpr const char *PYLON_PROMPT_PREFIX = "pylon";   // matches "pylon>" and "pylon_debug>" (prompt + echo)
static constexpr const char *PYLON_PROMPT_NORMAL = "pylon>";  // limited command set
static constexpr const char *PYLON_PROMPT_DEBUG = "pylon_debug>";  // full command set
static constexpr const char *PYLON_LOGIN_COMMAND = "login debug";  // elevates pylon> to pylon_debug>

// Console session handling
static constexpr uint32_t PROMPT_PROBE_INTERVAL = 1000;  // Send a bare newline this often until a prompt shows up
static constexpr uint8_t MAX_PROMPT_PROBES = 5;          // After this many unanswered probes, carry on regardless
static constexpr uint32_t LOGIN_TIMEOUT = 2000;          // Wait this long for the pylon_debug> prompt after login
static constexpr uint8_t MAX_LOGIN_ATTEMPTS = 3;
static constexpr uint32_t LOGIN_RETRY_DELAY = 60000;  // After failed logins, only try again after this long

enum class COMMAND : uint8_t { PWR, STAT, BAT, UNKNOWN };

// Which prompt the console showed last
enum class PROMPT : uint8_t { UNKNOWN, NORMAL, DEBUG };

enum class SESSION_STATE : uint8_t {
  PROBING,     // waiting for the first prompt
  LOGGING_IN,  // "login debug" sent, waiting for the pylon_debug> prompt
  READY        // commands may be sent
};

inline const char *command_to_string(COMMAND command) {
  switch (command) {
    case COMMAND::PWR:
      return "pwr";
    case COMMAND::STAT:
      return "stat";
    case COMMAND::BAT:
      return "bat";
    default:
      return "unknown";
  }
}

struct PylontechCommand {
  uint8_t battery_number{1};
  COMMAND command{COMMAND::UNKNOWN};

  bool operator==(const PylontechCommand &other) const {
    return this->battery_number == other.battery_number && this->command == other.command;
  }
};

inline PylontechCommand make_pwr_command(uint8_t battery_number) {
  return PylontechCommand{battery_number, COMMAND::PWR};
}

inline PylontechCommand make_stat_command(uint8_t battery_number) {
  return PylontechCommand{battery_number, COMMAND::STAT};
}

inline PylontechCommand make_bat_command(uint8_t battery_number) {
  return PylontechCommand{battery_number, COMMAND::BAT};
}

inline void create_command_string(const PylontechCommand &command, char *buffer, size_t buffer_size) {
  snprintf(buffer, buffer_size, "%s %u", command_to_string(command.command), (unsigned) command.battery_number);
}

}  // namespace pylontech_group
}  // namespace esphome