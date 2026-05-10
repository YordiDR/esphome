#include "esphome/core/log.h"
#include "../aa55_bus/aa55_bus.h"
#include "aa55_inverter.h"
#include "switch/aa55_inverter_switch.h"
#include "number/aa55_inverter_number.h"

namespace esphome {
namespace aa55_inverter {

static constexpr const char *LOGGING_TAG = "aa55_inverter";

AA55Inverter::AA55Inverter(const std::string &serial_number, uint8_t device_address) : PollingComponent() {
  this->serial_number_ = serial_number;
  this->device_address_ = device_address;
}

void AA55Inverter::setup() {
  ESP_LOGD(LOGGING_TAG, "Invalidating all sensors as part of startup...");

  // Send deregister command to inverter at ESP startup so we can register it again
  ESP_LOGD(LOGGING_TAG, "Sending remove register command for inverter %x", this->device_address_);
  this->parent_bus_->queue_command(aa55_bus::make_remove_register_packet(this->device_address_));
}

void AA55Inverter::dump_config() {
  ESP_LOGCONFIG(LOGGING_TAG, "Goodwe AA55 Inverter component");
  ESP_LOGCONFIG(LOGGING_TAG, "  AA55 Bus ID: %s", this->parent_bus_->get_component_id().c_str());
  ESP_LOGCONFIG(LOGGING_TAG, "  Serial number: %s", this->serial_number_.c_str());
  ESP_LOGCONFIG(LOGGING_TAG, "  Device address: %x", this->device_address_);
  ESP_LOGCONFIG(LOGGING_TAG, "  Update interval: %d", this->update_interval_);
}

void AA55Inverter::loop() {
  // Check for inverter offline timeout — packet processing now happens directly via handle_packet()
  if (this->inverter_online_ && millis() - this->last_packet_received_ >= aa55_inverter::INVERTER_OFFLINE_TIMEOUT) {
    ESP_LOGI(LOGGING_TAG, "Marking inverter %x on bus %s offline due to no response.", this->device_address_,
             this->parent_bus_->get_component_id().c_str());
    this->inverter_online_ = false;
    // Remove device as registered from bus
    this->parent_bus_->remove_registered_inverter(this);

    // Override sensor & input values to match offline state as best as possible
    for (AA55InverterBaseSensor *sensor : this->sensors_)
      sensor->handle_inverter_offline();

    for (AA55InverterBaseInput *input : this->inputs_)
      input->handle_inverter_offline();
  }
}

void AA55Inverter::update() {
  // Get updated running info from inverter if it's online
  if (!this->inverter_online_) {
    ESP_LOGD(LOGGING_TAG, "Inverter %x is currently offline. Skipping sending query run info command...",
             this->device_address_);
    return;
  }

  ESP_LOGD(LOGGING_TAG, "Sending query run info command to bus for inverter %x", this->device_address_);
  this->parent_bus_->queue_command(aa55_bus::make_query_run_info_packet(this->device_address_));
}

void AA55Inverter::add_sensor(AA55InverterBaseSensor *sensor) { this->sensors_.push_back(sensor); }

void AA55Inverter::add_input(AA55InverterBaseInput *input) { this->inputs_.push_back(input); }

void AA55Inverter::handle_packet(const aa55_bus::AA55RXPacket &packet) {
  this->last_packet_received_ = millis();

  switch (packet.control_code) {
    case aa55_bus::CONTROL_CODE::REGISTER:
      switch (packet.function_code) {
        case aa55_bus::FUNCTION_CODE::REG_REQUEST:
          this->handle_registration_request(packet);
          break;
        case aa55_bus::FUNCTION_CODE::ADDR_CONFIRM:
          this->handle_address_confirm(packet);
          break;
        case aa55_bus::FUNCTION_CODE::REMOVE_CONFIRM:
          ESP_LOGD(LOGGING_TAG, "Received remove register confirmation for inverter %x.", this->device_address_);
          break;
        default:
          ESP_LOGW(LOGGING_TAG,
                   "Inverter %x received a response packet with control code %x and unknown function code %x. "
                   "Skipping...",
                   this->device_address_, packet.control_code, packet.function_code);
      }
      break;
    case aa55_bus::CONTROL_CODE::READ:
      switch (packet.function_code) {
        case aa55_bus::FUNCTION_CODE::RUN_INFO_RESPONSE:
          this->parse_run_info_response(packet);
          break;
        case aa55_bus::FUNCTION_CODE::ID_INFO_RESPONSE:
          this->parse_id_info_response(packet);
          break;
        default:
          ESP_LOGW(LOGGING_TAG,
                   "Inverter %x received a response packet with control code %x and unknown function code %x. "
                   "Skipping...",
                   this->device_address_, packet.control_code, packet.function_code);
      }
      break;
    case aa55_bus::CONTROL_CODE::EXECUTE:
      this->parse_execute_response(packet);
      break;
    default:
      ESP_LOGW(LOGGING_TAG,
               "Inverter %x received a response packet with control code %x and unknown function code %x. Skipping...",
               this->device_address_, packet.control_code, packet.function_code);
  }
}

void AA55Inverter::parse_run_info_response(const aa55_bus::AA55RXPacket &packet) {
  ESP_LOGD(LOGGING_TAG, "Parsing run info response payload %s (%d bytes)",
           this->create_hex_string(packet.payload, packet.payload_length).c_str(), packet.payload_length);

  // During boot, sometimes the inverter returns an all 0 payload to the read command
  bool all_zeroes = true;
  for (uint8_t i = 0; i < packet.payload_length; i++)
    if (packet.payload[i] != 0) {
      all_zeroes = false;
      break;
    }

  if (all_zeroes) {
    ESP_LOGI(LOGGING_TAG, "Received read response with all 0 payload. Discarding response...");
    return;
  }

  // Save received values in the sensor attributes + publish state if applicable
  for (AA55InverterBaseSensor *sensor : this->sensors_) {
    if (sensor->get_payload_source() == aa55_bus::FUNCTION_CODE::RUN_INFO_RESPONSE)
      sensor->process_response(packet.payload, packet.payload_length);
  }
}

void AA55Inverter::parse_id_info_response(const aa55_bus::AA55RXPacket &packet) {
  ESP_LOGD(LOGGING_TAG, "Parsing ID info response payload %s (%d bytes)",
           this->create_hex_string(packet.payload, packet.payload_length).c_str(), packet.payload_length);

  // During boot, sometimes the inverter returns an all 0 payload to the read command
  bool all_zeroes = true;
  for (uint8_t i = 0; i < packet.payload_length; i++)
    if (packet.payload[i] != 0) {
      all_zeroes = false;
      break;
    }

  if (all_zeroes) {
    ESP_LOGI(LOGGING_TAG, "Received read ID info response with all 0 payload. Discarding response...");
    return;
  }

  // Save received values in the sensor attributes
  for (AA55InverterBaseSensor *sensor : this->sensors_) {
    if (sensor->get_payload_source() == aa55_bus::FUNCTION_CODE::ID_INFO_RESPONSE)
      sensor->process_response(packet.payload, packet.payload_length);
  }
}

void AA55Inverter::parse_execute_response(const aa55_bus::AA55RXPacket &packet) {
  for (AA55InverterBaseInput *input : this->inputs_) {
    if (input->handles_response(packet.function_code)) {
      ESP_LOGV(LOGGING_TAG, "Passing execute command response %x (payload %d) from inverter %x to input %s",
               (uint8_t) packet.function_code, packet.payload[0], this->device_address_, input->get_id().c_str());
      input->handle_response(packet.function_code, packet.payload[0]);
    }
  }
}

void AA55Inverter::send_execute_command(aa55_bus::FUNCTION_CODE function_code, uint8_t payload) {
  ESP_LOGD(LOGGING_TAG, "Sending execute command %x with payload %d to inverter %x", function_code, payload,
           this->device_address_);
  this->parent_bus_->queue_command(aa55_bus::make_execute_packet(this->device_address_, function_code, payload));
}

void AA55Inverter::handle_registration_request(const aa55_bus::AA55RXPacket &packet) {
  ESP_LOGD(LOGGING_TAG, "Assigning address %x to inverter with serial number %s", this->device_address_,
           this->serial_number_.c_str());

  // Send address confirm command to inverter
  this->parent_bus_->queue_command(
      aa55_bus::make_alloc_reg_addr_packet(packet.payload, packet.payload_length, this->device_address_));
}

uint8_t AA55Inverter::get_device_address() { return this->device_address_; }

void AA55Inverter::set_parent_bus(aa55_bus::AA55Bus *bus) { this->parent_bus_ = bus; }

const std::string &AA55Inverter::get_serial_number() { return this->serial_number_; }

// Function which is triggered when the inverter comes online by confirming its address.
void AA55Inverter::handle_address_confirm(const aa55_bus::AA55RXPacket &packet) {
  ESP_LOGD(LOGGING_TAG, "Received address confirm from inverter %x", this->device_address_);
  ESP_LOGI(LOGGING_TAG, "Inverter %x on bus %s came online.", this->device_address_,
           this->parent_bus_->get_component_id().c_str());
  this->inverter_online_ = true;
  this->parent_bus_->add_registered_inverter(this);

  for (AA55InverterBaseSensor *sensor : this->sensors_) {
    if (sensor->get_payload_source() == aa55_bus::FUNCTION_CODE::RUN_INFO_RESPONSE)
      sensor->force_next_update();  // Force update so sensors refresh immediately on the next received response
  }

  // Get serial & model info
  ESP_LOGD(LOGGING_TAG, "Sending query id info command to bus for inverter %x", this->device_address_);
  this->parent_bus_->queue_command(aa55_bus::make_query_id_info_packet(this->device_address_));

  // Initialize inputs
  for (AA55InverterBaseInput *input : this->inputs_)
    input->handle_inverter_online();
}
}  // namespace aa55_inverter
}  // namespace esphome
