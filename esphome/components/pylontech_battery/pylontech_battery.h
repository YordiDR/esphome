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
  void setup() override;
  void dump_config() override;
  void loop() override;
  void update() override;
  void add_sensor(PylontechBatteryBaseSensor *sensor);
  uint8_t get_battery_number();
  void set_parent_group(pylontech_group::PylontechGroup *group);
  void handle_response(const pylontech_group::PylontechResponse &response);
  bool is_online() const;

 protected:
  // Internal variables
  uint8_t battery_number_;
  std::vector<PylontechBatteryBaseSensor *> sensors_;
  bool battery_online_{false};
  pylontech_group::PylontechGroup *parent_group_{nullptr};
  // Initialised to max-timeout so the offline check doesn't fire before the inverter has registered
  uint32_t last_response_received_{UINT32_MAX - BATTERY_OFFLINE_TIMEOUT};
  bool pwr_command_necessary_{false};   // Whether we need to send a pwr command to this battery to get updated info
                                        // (toggled based on applicable sensor configuration)
  bool stat_command_necessary_{false};  // Whether we need to send a stat command to this battery to get updated info
                                        // (toggled based on applicable sensor configuration)

  // Functions
  void parse_stat_response(const char *response);
  void parse_pwr_response(const char *response);
};

}  // namespace pylontech_battery
}  // namespace esphome
