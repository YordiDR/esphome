#include <cstdlib>
#include <cstring>
#include "esphome/core/log.h"
#include "esphome/core/hal.h"
#include "../pylontech_group/pylontech_group.h"
#include "pylontech_battery.h"

namespace esphome {
namespace pylontech_battery {

static constexpr const char *LOGGING_TAG = "pylontech_battery";

namespace {

// Splits "Key   :   value unit" into a trimmed key and a pointer to the first character of the value.
bool split_kv(const char *line, char *key, size_t key_size, const char **value) {
  const char *colon = strchr(line, ':');
  if (colon == nullptr)
    return false;
  while (*line == ' ' || *line == '\t')
    line++;
  const char *key_end = colon;
  while (key_end > line && (key_end[-1] == ' ' || key_end[-1] == '\t'))
    key_end--;
  // Copy the key with runs of whitespace collapsed to one space: "Soft  version" -> "Soft version"
  size_t n = 0;
  bool prev_space = false;
  for (const char *p = line; p < key_end; p++) {
    const bool is_space = (*p == ' ' || *p == '\t');
    if (is_space && prev_space)
      continue;
    if (n + 1 >= key_size)
      return false;
    key[n++] = is_space ? ' ' : *p;
    prev_space = is_space;
  }
  if (n == 0)
    return false;
  key[n] = '\0';
  const char *v = colon + 1;
  while (*v == ' ' || *v == '\t')
    v++;
  *value = v;
  return true;
}

bool parse_long(const char *value, long &out) {
  char *end = nullptr;
  out = strtol(value, &end, 10);
  return end != value;
}

// Copies the whole value (trailing whitespace removed) into dst
void copy_value(char *dst, size_t dst_size, const char *value) {
  size_t n = strlen(value);
  if (n > dst_size - 1)
    n = dst_size - 1;
  while (n > 0 && (value[n - 1] == ' ' || value[n - 1] == '\t'))
    n--;
  memcpy(dst, value, n);
  dst[n] = '\0';
}

// Copies the first whitespace-delimited token of value into dst
void copy_token(char *dst, size_t dst_size, const char *value) {
  size_t i = 0;
  while (value[i] != '\0' && value[i] != ' ' && value[i] != '\t' && i < dst_size - 1) {
    dst[i] = value[i];
    i++;
  }
  dst[i] = '\0';
}

}  // namespace

PylontechBattery::PylontechBattery(uint8_t battery_number) : PollingComponent() {
  this->battery_number_ = battery_number;
}

void PylontechBattery::dump_config() {
  ESP_LOGCONFIG(LOGGING_TAG, "Pylontech Battery component");
  ESP_LOGCONFIG(LOGGING_TAG, "  Battery Number: %d", this->battery_number_);
  ESP_LOGCONFIG(LOGGING_TAG, "  Battery group ID: %s", this->parent_group_->get_component_id().c_str());
  ESP_LOGCONFIG(LOGGING_TAG, "  Sensors: %u", (unsigned) this->sensors_.size());
}

void PylontechBattery::add_sensor(PylontechBatteryBaseSensor *sensor) { this->sensors_.push_back(sensor); }

uint8_t PylontechBattery::get_battery_number() { return this->battery_number_; }

void PylontechBattery::set_parent_group(pylontech_group::PylontechGroup *group) { this->parent_group_ = group; }

void PylontechBattery::update() {
  // Offline detection
  if (this->has_received_response_ && !this->offline_ &&
      millis() - this->last_response_received_ > BATTERY_OFFLINE_TIMEOUT) {
    ESP_LOGW(LOGGING_TAG, "Battery %d in group %s has not answered for %u s, marking sensors unavailable",
             this->battery_number_, this->parent_group_->get_component_id().c_str(),
             (unsigned) (BATTERY_OFFLINE_TIMEOUT / 1000));
    this->offline_ = true;
    for (auto *sensor : this->sensors_)
      sensor->on_offline();
  }

  // Only ask for the commands that at least one *due* sensor needs
  bool needed_pwr = false, needed_stat = false, needed_bat = false, needed_info = false;
  for (auto *sensor : this->sensors_) {
    if (!sensor->is_due())
      continue;
    switch (sensor->get_payload_source()) {
      case pylontech_group::COMMAND::PWR:
        needed_pwr = true;
        break;
      case pylontech_group::COMMAND::STAT:
        needed_stat = true;
        break;
      case pylontech_group::COMMAND::BAT:
        needed_bat = true;
        break;
      case pylontech_group::COMMAND::INFO:
        needed_info = true;
        break;
      default:
        break;
    }
  }

  if (needed_pwr) {
    ESP_LOGD(LOGGING_TAG, "Queueing pwr command to group %s for battery %d",
             this->parent_group_->get_component_id().c_str(), this->battery_number_);
    this->parent_group_->queue_command(pylontech_group::make_pwr_command(this->battery_number_));
  }
  if (needed_bat) {
    ESP_LOGD(LOGGING_TAG, "Queueing bat command to group %s for battery %d",
             this->parent_group_->get_component_id().c_str(), this->battery_number_);
    this->parent_group_->queue_command(pylontech_group::make_bat_command(this->battery_number_));
  }
  if (needed_stat) {
    ESP_LOGD(LOGGING_TAG, "Queueing stat command to group %s for battery %d",
             this->parent_group_->get_component_id().c_str(), this->battery_number_);
    this->parent_group_->queue_command(pylontech_group::make_stat_command(this->battery_number_));
  }

  if (needed_info) {
    ESP_LOGD(LOGGING_TAG, "Queueing info command to group %s for battery %d",
             this->parent_group_->get_component_id().c_str(), this->battery_number_);
    this->parent_group_->queue_command(pylontech_group::make_info_command(this->battery_number_));
  }

  // Count down skip_updates for sensors that are not due yet (due sensors wait for their data)
  for (auto *sensor : this->sensors_)
    sensor->tick();
}

void PylontechBattery::handle_line(pylontech_group::COMMAND command, const char *line) {
  switch (command) {
    case pylontech_group::COMMAND::PWR:
      this->parse_pwr_line_(line);
      break;
    case pylontech_group::COMMAND::STAT:
      this->parse_stat_line_(line);
      break;
    case pylontech_group::COMMAND::BAT:
      this->parse_bat_line_(line);
      break;
    case pylontech_group::COMMAND::INFO:
      this->parse_info_line_(line);
      break;
    default:
      break;
  }
}

void PylontechBattery::handle_response_end(pylontech_group::COMMAND command, bool success) {
  BatteryData data = this->scratch_;
  this->scratch_ = BatteryData{};  // always start the next response from a clean slate

  const bool has_data = command == pylontech_group::COMMAND::BAT ? data.cell_count > 0 : data.parsed_fields > 0;
  if (!success || !has_data) {
    ESP_LOGW(LOGGING_TAG, "No usable '%s' response for battery %d in group %s",
             pylontech_group::command_to_string(command), this->battery_number_,
             this->parent_group_->get_component_id().c_str());
    return;  // sensors stay due and are retried on the next update()
  }

  // Derived values
  if (command == pylontech_group::COMMAND::PWR && !std::isnan(data.voltage) && !std::isnan(data.current))
    data.power = data.voltage * data.current;
  if (command == pylontech_group::COMMAND::BAT) {
    data.highest_cell_voltage = data.cell_max_mv / 1000.0f;
    data.lowest_cell_voltage = data.cell_min_mv / 1000.0f;
    data.cell_voltage_delta = (float) (data.cell_max_mv - data.cell_min_mv);
  }

  this->last_response_received_ = millis();
  this->has_received_response_ = true;
  this->offline_ = false;

  ESP_LOGD(LOGGING_TAG, "Publishing '%s' data for battery %d", pylontech_group::command_to_string(command),
           this->battery_number_);
  for (auto *sensor : this->sensors_) {
    if (sensor->get_payload_source() == command && sensor->is_due()) {
      sensor->process_data(data);
      sensor->mark_published();
    }
  }
}

void PylontechBattery::parse_pwr_line_(const char *line) {
  char key[24];
  const char *value = nullptr;
  long n = 0;
  if (!split_kv(line, key, sizeof(key), &value))
    return;

  BatteryData &d = this->scratch_;
  if (strcmp(key, "Voltage") == 0 && parse_long(value, n)) {  // mV
    d.voltage = n / 1000.0f;
    d.parsed_fields++;
  } else if (strcmp(key, "Current") == 0 && parse_long(value, n)) {  // mA
    d.current = n / 1000.0f;
    d.parsed_fields++;
  } else if (strcmp(key, "Temperature") == 0 && parse_long(value, n)) {  // mC
    d.temperature = n / 1000.0f;
    d.parsed_fields++;
  } else if (strcmp(key, "Coulomb") == 0 && parse_long(value, n)) {  // the console calls the SoC "Coulomb", in %
    d.soc = (float) n;
    d.parsed_fields++;
  } else if (strcmp(key, "Basic Status") == 0) {
    copy_token(d.state, sizeof(d.state), value);
    d.parsed_fields++;
  } else if (strcmp(key, "Volt Status") == 0) {
    copy_token(d.voltage_state, sizeof(d.voltage_state), value);
    d.parsed_fields++;
  } else if (strcmp(key, "Current Status") == 0) {
    copy_token(d.current_state, sizeof(d.current_state), value);
    d.parsed_fields++;
  } else if (strcmp(key, "Tmpr. Status") == 0) {
    copy_token(d.temperature_state, sizeof(d.temperature_state), value);
    d.parsed_fields++;
  }
}

void PylontechBattery::parse_stat_line_(const char *line) {
  char key[24];
  const char *value = nullptr;
  long n = 0;
  if (!split_kv(line, key, sizeof(key), &value))
    return;
  if (strcmp(key, "SOH") == 0 && parse_long(value, n)) {  // exact match: "SOH Times" is a different key
    this->scratch_.soh = (float) n;
    this->scratch_.parsed_fields++;
  }
}

void PylontechBattery::parse_info_line_(const char *line) {
  char key[24];
  const char *value = nullptr;
  if (!split_kv(line, key, sizeof(key), &value))
    return;

  BatteryData &d = this->scratch_;
  char *target = nullptr;
  if (strcmp(key, "Device name") == 0)
    target = d.device_name;
  else if (strcmp(key, "Board version") == 0)
    target = d.board_version;
  else if (strcmp(key, "Board") == 0)
    target = d.board;
  else if (strcmp(key, "Main Soft version") == 0)
    target = d.main_software_version;
  else if (strcmp(key, "Soft version") == 0)
    target = d.software_version;
  else if (strcmp(key, "Boot version") == 0)
    target = d.boot_version;
  else if (strcmp(key, "Comm version") == 0)
    target = d.comm_version;
  else if (strcmp(key, "Barcode") == 0)  // exposed as "serial_number"
    target = d.serial_number;
  if (target == nullptr)
    return;

  copy_value(target, INFO_TEXT_LENGTH, value);
  d.parsed_fields++;
}

void PylontechBattery::parse_bat_line_(const char *line) {
  // The header starts with "Battery"; data rows start with the cell index followed by the cell voltage in mV.
  if (line[0] < '0' || line[0] > '9')
    return;
  long index = 0, mv = 0;
  if (sscanf(line, "%ld %ld", &index, &mv) != 2)
    return;

  BatteryData &d = this->scratch_;
  if (d.cell_count == 0) {
    d.cell_min_mv = d.cell_max_mv = (int32_t) mv;
  } else {
    d.cell_min_mv = std::min(d.cell_min_mv, (int32_t) mv);
    d.cell_max_mv = std::max(d.cell_max_mv, (int32_t) mv);
  }
  if (d.cell_count < 255)
    d.cell_count++;
  d.parsed_fields++;
}

}  // namespace pylontech_battery
}  // namespace esphome
