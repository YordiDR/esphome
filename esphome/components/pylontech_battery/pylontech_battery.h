#pragma once
#include <string>
#include <vector>
#include "esphome/core/component.h"
#include "../pylontech_group/pylontech_group.h"
#include "pylontech_battery_base_sensor.h"
#include "const.h"

namespace esphome {
namespace pylontech_battery {

class PylontechBattery : public PollingComponent {
 public:
  PylontechBattery(uint8_t battery_number);
  void dump_config() override;
  void update() override;
  void add_sensor(PylontechBatteryBaseSensor *sensor);
  uint8_t get_battery_number();
  void set_parent_group(pylontech_group::PylontechGroup *group);

  /// Called by the group for every payload line of the response to `command` (trimmed, no prompt/echo/@/$$).
  void handle_line(pylontech_group::COMMAND command, const char *line);
  /// Called by the group when the response to `command` is complete (success) or timed out (failure).
  void handle_response_end(pylontech_group::COMMAND command, bool success);

 protected:
  // Internal variables
  uint8_t battery_number_;
  std::vector<PylontechBatteryBaseSensor *> sensors_;
  pylontech_group::PylontechGroup *parent_group_{nullptr};
  uint32_t last_response_received_{0};
  bool has_received_response_{false};
  bool offline_{false};
  BatteryData scratch_{};  // Filled while a response is being received, published when it completes

  // Functions
  void parse_pwr_line_(const char *line);
  void parse_stat_line_(const char *line);
  void parse_bat_line_(const char *line);
};

}  // namespace pylontech_battery
}  // namespace esphome
