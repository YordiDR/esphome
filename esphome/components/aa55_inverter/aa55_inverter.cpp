#include "esphome/core/log.h"
#include "aa55_inverter.h"
#include <iterator>
#include <cmath>
#include <algorithm>

namespace esphome {
namespace aa55_inverter {
AA55Inverter::AA55Inverter(std::string serial_number, uint8_t slave_address) {
  serial_number_ = serial_number;
  slave_address_ = slave_address;
}

void AA55Inverter::setup() {
  // Mark all sensors as unavailable
  for (AA55InverterSensor *sensor : this->sensors_) {
    sensor->publish_state(NAN);
  }
  for (AA55InverterTextSensor *sensor : this->text_sensors_) {
    sensor->publish_state("");
  }

  // Send deregister command to inverter at ESP startup so we can register it again
  const aa55_const::AA55Command remove_register = {this->parent_bus_->get_master_address(), this->slave_address_,
                                                   aa55_const::CONTROL_CODE::REGISTER,
                                                   aa55_const::FUNCTION_CODE::REMOVE_REG, aa55_const::EMPTY_VECTOR};
  this->parent_bus_->send_packet(remove_register);
}

void AA55Inverter::dump_config() {
  ESP_LOGCONFIG(LOGGING_TAG, "Goodwe AA55 Inverter component");
  ESP_LOGCONFIG(LOGGING_TAG, "  AA55 Bus ID: %s", this->parent_bus_->get_component_id().c_str());
  ESP_LOGCONFIG(LOGGING_TAG, "  Serial number: %s", this->serial_number_.c_str());
  ESP_LOGCONFIG(LOGGING_TAG, "  Slave address: %x", this->slave_address_);
  ESP_LOGCONFIG(LOGGING_TAG, "  Update interval: %d", this->update_interval_);
}

void AA55Inverter::loop() {
  // TODO If inverter is unregistered, send out offline requests to discover when it comes online, process registration
  // TODO send register command every 10s
}

void AA55Inverter::update() {
  // Get updated running info from inverter
  const aa55_const::AA55Command query_run_info = {this->parent_bus_->get_master_address(), aa55_const::DEFAULT_ADDRESS,
                                                  aa55_const::CONTROL_CODE::READ,
                                                  aa55_const::FUNCTION_CODE::QUERY_RUN_INFO, aa55_const::EMPTY_VECTOR};
  this->parent_bus_->drain_uart_rx_buffer();
  this->parent_bus_->send_packet(query_run_info);
  std::vector<uint8_t> response_payload = this->parent_bus_->await_response(query_run_info);

  if (response_payload == aa55_const::EMPTY_VECTOR) {
    if (this->inverter_online_) {
      this->inverter_offline_countdown_--;
      if (this->inverter_offline_countdown_ == 0) {
        ESP_LOGI(LOGGING_TAG, "Considering inverter offline due to countdown.");
        this->inverter_online_ = false;
      }
    }
    return;
  }

  // Mark inverter as online if it was previously online since we succesfully received a response
  if (!this->inverter_online_) {
    this->inverter_online_ = true;
    this->inverter_offline_countdown_ = aa55_const::INVERTER_OFFLINE_COUNTDOWN_RESET;
  }

  if (response_payload == aa55_const::EMPTY_VECTOR) {
    return;
  }

  // Parse updated sensor info
  this->parse_run_info_response(response_payload);

  // Get updated ID info from inverter
  const aa55_const::AA55Command query_id_info = {this->parent_bus_->get_master_address(), aa55_const::DEFAULT_ADDRESS,
                                                 aa55_const::CONTROL_CODE::READ,
                                                 aa55_const::FUNCTION_CODE::QUERY_ID_INFO, aa55_const::EMPTY_VECTOR};
  this->parent_bus_->drain_uart_rx_buffer();
  this->parent_bus_->send_packet(query_id_info);
  response_payload = this->parent_bus_->await_response(query_id_info);

  if (response_payload == aa55_const::EMPTY_VECTOR) {
    return;
  }

  // Parse updated sensor info
  this->parse_id_info_response(response_payload);

  if (!this->inverter_online_) {
    // Set all sensors to an unknown state
    for (AA55InverterSensor *sensor : this->sensors_) {
      sensor->publish_state(NAN);
    }

    // Set all text sensors to an empty string besides WORK_MODE if it is defined
    for (AA55InverterTextSensor *sensor : this->text_sensors_) {
      if (sensor->get_type() == aa55_const::SENSOR_TYPE::WORK_MODE) {
        sensor->publish_state("Offline");
      } else {
        sensor->publish_state("");
      }
    }
  }

  // Publish most recent sensor values if applicable
  for (AA55InverterSensor *sensor : this->sensors_) {
    if (sensor->time_to_update()) {
      sensor->publish_state(sensor->get_newest_value());
      if (sensor->get_skip_updates() != 0) {
        sensor->reset_skipped_updates();
      }
    } else {
      sensor->increment_skipped_updates();
    }
  }
  for (AA55InverterTextSensor *sensor : this->text_sensors_) {
    if (sensor->time_to_update()) {
      sensor->publish_state(sensor->get_newest_value());
      if (sensor->get_skip_updates() != 0) {
        sensor->reset_skipped_updates();
      }
    } else {
      sensor->increment_skipped_updates();
    }
  }
}

void AA55Inverter::add_sensor(AA55InverterSensor *sensor) { this->sensors_.push_back(sensor); }

void AA55Inverter::add_text_sensor(AA55InverterTextSensor *sensor) { this->text_sensors_.push_back(sensor); }

void AA55Inverter::parse_run_info_response(const std::vector<uint8_t> &payload) {
  ESP_LOGD(LOGGING_TAG, "Parsing run info response payload %s (%d bytes)", this->create_hex_string(payload).c_str(),
           payload.size());
  ESP_LOGD(LOGGING_TAG, "Parsing packet payload...");

  // During boot, sometimes the inverter returns an all 0 payload to the read command.
  bool all_zeroes = std::all_of(payload.begin(), payload.end(), [](int i) { return i == 0; });

  if (all_zeroes) {
    ESP_LOGI(LOGGING_TAG, "Received read response with all 0 payload. Discarding response...");
    return;
  }

  // Save received values in the sensor attributes
  for (AA55InverterSensor *sensor : this->sensors_) {
    if (aa55_const::MAP_SENSOR_PAYLOAD_SOURCE.at(sensor->get_type()) == aa55_const::FUNCTION_CODE::RUN_INFO_RESPONSE) {
      ESP_LOGV(LOGGING_TAG, "Parsing %s from payload[%d], length %d bytes.", sensor->get_id().c_str(),
               sensor->get_payload_location(), sensor->get_payload_length());
      sensor->parse_payload(payload);
      ESP_LOGV(LOGGING_TAG, "Parsed %s: %f", sensor->get_id().c_str(), sensor->get_newest_value());
    }
  }

  for (AA55InverterTextSensor *sensor : this->text_sensors_) {
    if (aa55_const::MAP_SENSOR_PAYLOAD_SOURCE.at(sensor->get_type()) == aa55_const::FUNCTION_CODE::RUN_INFO_RESPONSE) {
      ESP_LOGV(LOGGING_TAG, "Parsing %s from payload[%d], length %d bytes.", sensor->get_id().c_str(),
               sensor->get_payload_location(), sensor->get_payload_length());
      sensor->parse_payload(payload);
      ESP_LOGV(LOGGING_TAG, "Parsed %s: %s", sensor->get_id().c_str(), sensor->get_newest_value().c_str());
    }
  }
}

void AA55Inverter::parse_id_info_response(const std::vector<uint8_t> &payload) {
  ESP_LOGD(LOGGING_TAG, "Parsing ID info response payload %s (%d bytes)", this->create_hex_string(payload).c_str(),
           payload.size());
  ESP_LOGD(LOGGING_TAG, "Parsing packet payload...");

  // During boot, sometimes the inverter returns an all 0 payload to the read command.
  bool all_zeroes = std::all_of(payload.begin(), payload.end(), [](int i) { return i == 0; });

  if (all_zeroes) {
    ESP_LOGI(LOGGING_TAG, "Received read ID info response with all 0 payload. Discarding response...");
    return;
  }

  // Save received values in the sensor attributes
  for (AA55InverterSensor *sensor : this->sensors_) {
    if (aa55_const::MAP_SENSOR_PAYLOAD_SOURCE.at(sensor->get_type()) == aa55_const::FUNCTION_CODE::ID_INFO_RESPONSE) {
      ESP_LOGV(LOGGING_TAG, "Parsing %s from payload[%d], length %d bytes.", sensor->get_id().c_str(),
               sensor->get_payload_location(), sensor->get_payload_length());
      sensor->parse_payload(payload);
      ESP_LOGV(LOGGING_TAG, "Parsed %s: %f", sensor->get_id().c_str(), sensor->get_newest_value());
    }
  }

  for (AA55InverterTextSensor *sensor : this->text_sensors_) {
    if (aa55_const::MAP_SENSOR_PAYLOAD_SOURCE.at(sensor->get_type()) == aa55_const::FUNCTION_CODE::ID_INFO_RESPONSE) {
      ESP_LOGV(LOGGING_TAG, "Parsing %s from payload[%d], length %d bytes.", sensor->get_id().c_str(),
               sensor->get_payload_location(), sensor->get_payload_length());
      sensor->parse_payload(payload);
      ESP_LOGV(LOGGING_TAG, "Parsed %s: %s", sensor->get_id().c_str(), sensor->get_newest_value().c_str());
    }
  }
}
}  // namespace aa55_inverter
}  // namespace esphome
