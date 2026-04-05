#include "esphome/core/log.h"
#include "aa55_inverter.h"
#include "switch/aa55_inverter_switch.h"
#include "number/aa55_inverter_number.h"
#include "../aa55_bus/aa55_bus.h"
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
  ESP_LOGD(LOGGING_TAG, "Invalidating all sensors as part of startup...");
  // Mark all sensors as unavailable
  for (AA55InverterSensor *sensor : this->sensors_) {
    sensor->publish_state(NAN);
  }
  for (AA55InverterTextSensor *sensor : this->text_sensors_) {
    sensor->publish_state("");
  }

  // Send deregister command to inverter at ESP startup so we can register it again
  ESP_LOGD(LOGGING_TAG, "Sending remove register command to bus for inverter %x", this->slave_address_);
  const aa55_const::AA55Packet remove_register_command = {
      this->parent_bus_->get_master_address(), this->slave_address_, aa55_const::CONTROL_CODE::REGISTER,
      aa55_const::FUNCTION_CODE::REMOVE_REG, aa55_const::EMPTY_VECTOR};
  this->parent_bus_->queue_command(remove_register_command);
}

void AA55Inverter::dump_config() {
  ESP_LOGCONFIG(LOGGING_TAG, "Goodwe AA55 Inverter component");
  ESP_LOGCONFIG(LOGGING_TAG, "  AA55 Bus ID: %s", this->parent_bus_->get_component_id().c_str());
  ESP_LOGCONFIG(LOGGING_TAG, "  Serial number: %s", this->serial_number_.c_str());
  ESP_LOGCONFIG(LOGGING_TAG, "  Slave address: %x", this->slave_address_);
  ESP_LOGCONFIG(LOGGING_TAG, "  Update interval: %d", this->update_interval_);
}

void AA55Inverter::loop() {
  uint32_t start_time = millis();
  while (!this->response_packets_buffer_.empty() &&
         millis() < start_time + 30) {  // Avoid blocking the thread for 30ms+
    this->last_packet_received_ = millis();

    if (!this->inverter_online_) {
      ESP_LOGI(LOGGING_TAG, "Inverter %x on bus %s came online.", this->slave_address_,
               this->parent_bus_->get_component_id().c_str());
      this->inverter_online_ = true;
      this->packet_brought_inverter_online = true;

      // Get serial & model info
      ESP_LOGD(LOGGING_TAG, "Sending query id info command to bus for inverter %x", this->slave_address_);
      const aa55_const::AA55Packet query_id_info_command = {
          this->parent_bus_->get_master_address(), aa55_const::DEFAULT_ADDRESS, aa55_const::CONTROL_CODE::READ,
          aa55_const::FUNCTION_CODE::QUERY_ID_INFO, aa55_const::EMPTY_VECTOR};
      this->parent_bus_->queue_command(query_id_info_command);

      // Initialize inputs
      for (AA55InverterBaseInput *input : this->inputs_) {
        switch (input->get_type()) {
          case aa55_const::INPUT_TYPE::START_STOP:
            static_cast<AA55InverterSwitch *>(input)->publish_state(true);
            break;
          case aa55_const::INPUT_TYPE::ADJUST_POWER:
            static_cast<AA55InverterNumber *>(input)->publish_state(100);
            break;
        }
      }
    }

    // Parse packet
    aa55_const::AA55Packet *packet = &this->response_packets_buffer_.front();
    switch (packet->control_code) {
      case aa55_const::CONTROL_CODE::READ:
        switch (packet->function_code) {
          case aa55_const::FUNCTION_CODE::RUN_INFO_RESPONSE:
            this->parse_run_info_response(packet->payload);
            break;
          case aa55_const::FUNCTION_CODE::ID_INFO_RESPONSE:
            this->parse_id_info_response(packet->payload);
            break;
          default:
            ESP_LOGW(
                LOGGING_TAG,
                "Inverter %d received a response packet with control code %x and unknown function code %x. Skipping...",
                this->slave_address_, packet->control_code, packet->function_code);
        }
        break;
      case aa55_const::CONTROL_CODE::EXECUTE:
        this->parse_execute_response(packet->function_code, packet->payload.at(0));
        break;
      default:
        ESP_LOGW(
            LOGGING_TAG,
            "Inverter %d received a response packet with control code %x and unknown function code %x. Skipping...",
            this->slave_address_, packet->control_code, packet->function_code);
    }

    this->response_packets_buffer_.pop();
  }

  if (this->inverter_online_ && millis() > this->last_packet_received_ + 30000) {
    ESP_LOGI(LOGGING_TAG, "Marking inverter %x on bus %s offline due to no response.", this->slave_address_,
             this->parent_bus_->get_component_id().c_str());
    this->inverter_online_ = false;

    // Override sensor & input values to match offline state as best as possible
    for (AA55InverterSensor *sensor : this->sensors_) {
      switch (sensor->get_type()) {
        case aa55_const::SENSOR_TYPE::VPV1:
        case aa55_const::SENSOR_TYPE::IPV1:
        case aa55_const::SENSOR_TYPE::VPV2:
        case aa55_const::SENSOR_TYPE::IPV2:
        case aa55_const::SENSOR_TYPE::VAC1:
        case aa55_const::SENSOR_TYPE::IAC1:
        case aa55_const::SENSOR_TYPE::FAC1:
        case aa55_const::SENSOR_TYPE::TEMPERATURE:
          sensor->publish_state(NAN);
          break;
        case aa55_const::SENSOR_TYPE::PAC:
          sensor->publish_state(0);
      }
    }

    for (AA55InverterTextSensor *sensor : this->text_sensors_) {
      switch (sensor->get_type()) {
        case aa55_const::SENSOR_TYPE::WORK_MODE:
          sensor->publish_state("Offline");
          break;
        case aa55_const::SENSOR_TYPE::ERROR_CODES:
          sensor->publish_state("");
      }
    }

    for (AA55InverterBaseInput *input : this->inputs_) {
      switch (input->get_type()) {
        case aa55_const::INPUT_TYPE::START_STOP:
          static_cast<AA55InverterSwitch *>(input)->publish_state(false);
          break;
        case aa55_const::INPUT_TYPE::ADJUST_POWER:
          static_cast<AA55InverterNumber *>(input)->publish_state(NAN);
      }
    }
  }
}

void AA55Inverter::update() {
  // Get updated running info from inverter
  ESP_LOGD(LOGGING_TAG, "Sending query run info command to bus for inverter %x", this->slave_address_);
  const aa55_const::AA55Packet query_run_info_command = {
      this->parent_bus_->get_master_address(), aa55_const::DEFAULT_ADDRESS, aa55_const::CONTROL_CODE::READ,
      aa55_const::FUNCTION_CODE::QUERY_RUN_INFO, aa55_const::EMPTY_VECTOR};
  this->parent_bus_->queue_command(query_run_info_command);
}

void AA55Inverter::add_sensor(AA55InverterSensor *sensor) { this->sensors_.push_back(sensor); }

void AA55Inverter::add_text_sensor(AA55InverterTextSensor *sensor) { this->text_sensors_.push_back(sensor); }

void AA55Inverter::add_input(AA55InverterBaseInput *input) { this->inputs_.push_back(input); }

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

  // Save received values in the sensor attributes + publish state if applicable
  for (AA55InverterSensor *sensor : this->sensors_) {
    if (aa55_const::MAP_SENSOR_RESPONSE_SOURCE.at(sensor->get_type()) == aa55_const::FUNCTION_CODE::RUN_INFO_RESPONSE) {
      ESP_LOGV(LOGGING_TAG, "Parsing %s from payload[%d], length %d bytes.", sensor->get_id().c_str(),
               sensor->get_payload_location(), sensor->get_payload_length());
      sensor->parse_payload(payload);
      ESP_LOGV(LOGGING_TAG, "Parsed %s: %f", sensor->get_id().c_str(), sensor->get_newest_value());
      ESP_LOGV(LOGGING_TAG, "Checking if it's time to update sensor %s: %s", sensor->get_id().c_str(),
               sensor->time_to_update() ? "yes" : "no");

      if (sensor->time_to_update() ||
          this->packet_brought_inverter_online) {  // Publish state if it matches sensor config or if it is the first
                                                   // state we received after the inverter came online
        sensor->publish_state(sensor->get_newest_value());
        if (sensor->get_skip_updates() != 0) {
          sensor->reset_skipped_updates();
        }
      } else {
        sensor->increment_skipped_updates();
      }
    }
  }

  for (AA55InverterTextSensor *sensor : this->text_sensors_) {
    if (aa55_const::MAP_SENSOR_RESPONSE_SOURCE.at(sensor->get_type()) == aa55_const::FUNCTION_CODE::RUN_INFO_RESPONSE) {
      ESP_LOGV(LOGGING_TAG, "Parsing %s from payload[%d], length %d bytes.", sensor->get_id().c_str(),
               sensor->get_payload_location(), sensor->get_payload_length());
      sensor->parse_payload(payload);
      ESP_LOGV(LOGGING_TAG, "Parsed %s: %s", sensor->get_id().c_str(), sensor->get_newest_value().c_str());
      ESP_LOGV(LOGGING_TAG, "Checking if it's time to update sensor %s: %s", sensor->get_id().c_str(),
               sensor->time_to_update() ? "yes" : "no");

      if (sensor->time_to_update() ||
          this->packet_brought_inverter_online) {  // Publish state if it matches sensor config or if it is the first
                                                   // state we received after the inverter came online
        sensor->publish_state(sensor->get_newest_value());
        if (sensor->get_skip_updates() != 0) {
          sensor->reset_skipped_updates();
        }
      } else {
        sensor->increment_skipped_updates();
      }
    }
  }

  if (this->packet_brought_inverter_online) {
    this->packet_brought_inverter_online = false;
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
  for (AA55InverterTextSensor *sensor : this->text_sensors_) {
    if (aa55_const::MAP_SENSOR_RESPONSE_SOURCE.at(sensor->get_type()) == aa55_const::FUNCTION_CODE::ID_INFO_RESPONSE) {
      ESP_LOGV(LOGGING_TAG, "Parsing %s from payload[%d], length %d bytes.", sensor->get_id().c_str(),
               sensor->get_payload_location(), sensor->get_payload_length());
      sensor->parse_payload(payload);
      ESP_LOGV(LOGGING_TAG, "Parsed %s: %s", sensor->get_id().c_str(), sensor->get_newest_value().c_str());
      sensor->publish_state(sensor->get_newest_value());
    }
  }
}

void AA55Inverter::parse_execute_response(aa55_const::FUNCTION_CODE function_code, uint8_t response) {
  for (AA55InverterBaseInput *input : this->inputs_) {
    if (input->get_type() == aa55_const::MAP_RESPONSE_INPUT.at(function_code)) {
      ESP_LOGV(LOGGING_TAG, "Passing execute command response %x (payload %d) from inverter %x to input %s",
               (uint8_t) function_code, response, this->get_slave_address(), input->get_id().c_str());
      input->handle_response(function_code, response);
    }
  }
}

void AA55Inverter::send_execute_command(aa55_const::FUNCTION_CODE function_code, uint8_t payload) {
  ESP_LOGD(LOGGING_TAG, "Sending execute command %x with payload %d to inverter %x", function_code, payload,
           this->slave_address_);
  std::vector<uint8_t> payload_vector;
  if (function_code == aa55_const::FUNCTION_CODE::ADJUST_POWER) {
    payload_vector.push_back(payload);
  }

  const aa55_const::AA55Packet execute_command = {this->parent_bus_->get_master_address(), aa55_const::DEFAULT_ADDRESS,
                                                  aa55_const::CONTROL_CODE::EXECUTE, function_code, payload_vector};
  this->parent_bus_->queue_command(execute_command);
}

}  // namespace aa55_inverter
}  // namespace esphome
