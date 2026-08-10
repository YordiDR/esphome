#include "esphome/core/log.h"
#include "../pylontech_group/pylontech_group.h"
#include "pylontech_battery.h"

namespace esphome {
namespace pylontech_battery {

static constexpr const char *LOGGING_TAG = "pylontech_battery";

PylontechBattery::PylontechBattery(uint8_t battery_number) : PollingComponent() {
  this->battery_number_ = battery_number;
}

void PylontechBattery::setup() {
  // Check if a sensor with a payload source of PWR or STAT is configured, and if so, mark the corresponding command as
  // necessary to send
  for (PylontechBatteryBaseSensor *sensor : this->sensors_) {
    if (!this->pwr_command_necessary_ && sensor->get_payload_source() == pylontech_group::COMMAND::PWR) {
      this->pwr_command_necessary_ = true;
    } else if (!this->stat_command_necessary_ && sensor->get_payload_source() == pylontech_group::COMMAND::STAT) {
      this->stat_command_necessary_ = true;
    }
    if (this->pwr_command_necessary_ && this->stat_command_necessary_) {
      break;
    }
  }
}

void PylontechBattery::dump_config() {
  ESP_LOGCONFIG(LOGGING_TAG, "Pylontech Battery component");
  ESP_LOGCONFIG(LOGGING_TAG, "  Battery Number: %d", this->battery_number_);
  ESP_LOGCONFIG(LOGGING_TAG, "  Battery group ID: %s", this->parent_group_->get_component_id().c_str());
}

void PylontechBattery::loop() {}

void PylontechBattery::update() {
  if (this->pwr_command_necessary_) {
    ESP_LOGD(LOGGING_TAG, "Queueing pwr command to group %s for battery %d",
             this->parent_group_->get_component_id().c_str(), this->battery_number_);
    this->parent_group_->queue_command(pylontech_group::make_pwr_command(this->battery_number_));
  }
  if (this->stat_command_necessary_) {
    ESP_LOGD(LOGGING_TAG, "Queueing stat command to group %s for battery %d",
             this->parent_group_->get_component_id().c_str(), this->battery_number_);
    this->parent_group_->queue_command(pylontech_group::make_stat_command(this->battery_number_));
  }
}

void PylontechBattery::add_sensor(PylontechBatteryBaseSensor *sensor) { this->sensors_.push_back(sensor); }

void PylontechBattery::handle_response(const pylontech_group::PylontechResponse &response) {
  this->last_response_received_ = millis();

  return;  // Temp return for debugging

  if (response.command == pylontech_group::COMMAND::UNKNOWN) {
    ESP_LOGW(LOGGING_TAG, "Received response '%s' for battery %d in group %s with unknown command. Skipping...",
             response.payload, this->battery_number_, this->parent_group_->get_component_id().c_str());
    return;
  }

  ESP_LOGD(LOGGING_TAG, "Parsing %s response payload %s", pylontech_group::command_to_string(response.command),
           response.payload);

  // Save received values in the sensor attributes + publish state if applicable
  for (PylontechBatteryBaseSensor *sensor : this->sensors_) {
    if (sensor->get_payload_source() == response.command) {
      sensor->process_response(response.payload);
    }
  }
}

uint8_t PylontechBattery::get_battery_number() { return this->battery_number_; }

void PylontechBattery::set_parent_group(pylontech_group::PylontechGroup *group) { this->parent_group_ = group; }
}  // namespace pylontech_battery
}  // namespace esphome
