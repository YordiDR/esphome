#pragma once
#include <cstdint>
#include <cstdio>
#include <cstring>

namespace esphome {
namespace pylontech_group {
static constexpr size_t READ_BATCH_SIZE = 64;
static constexpr size_t MAX_QUEUED_TX_COMMANDS = 16;  // Max amount of commands that can be queued to send over UART
static constexpr uint16_t MAX_BUFFER_LENGTH = 1024;   // Max characters for serial buffer, ?? bytes is the length of the
                                                      // response to the longest command
static constexpr uint16_t COMMAND_DELAY = 500;        // Min time between RS232 commands in milliseconds
static constexpr const char *PWR_COMMAND = "pwr";
static constexpr const char *STAT_COMMAND = "stat";

enum class COMMAND : uint8_t { PWR, STAT, UNKNOWN };

inline const char *command_to_string(COMMAND command) {
  switch (command) {
    case COMMAND::PWR:
      return "pwr";
    case COMMAND::STAT:
      return "stat";
    default:
      return "unknown";
  }
};

struct PylontechCommand {
  uint8_t battery_number{1};
  COMMAND command{};
};

struct PylontechResponse {
  uint8_t battery_number{1};
  COMMAND command{};
  char payload[MAX_BUFFER_LENGTH + 1]{};
};

inline PylontechCommand make_pwr_command(uint8_t battery_number) {
  return PylontechCommand{battery_number, COMMAND::PWR};
}

inline PylontechCommand make_stat_command(uint8_t battery_number) {
  return PylontechCommand{battery_number, COMMAND::STAT};
}

inline void create_command_string(const PylontechCommand &command, char *buffer, size_t buffer_size) {
  snprintf(buffer, buffer_size, "%s %d", command_to_string(command.command), command.battery_number);
};

}  // namespace pylontech_group
}  // namespace esphome
