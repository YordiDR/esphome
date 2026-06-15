#include <algorithm>
#include <climits>
#include "esphome/core/log.h"
#include "pylontech_group.h"
#include "../pylontech_battery/pylontech_battery.h"

namespace esphome {
namespace pylontech_group {
static constexpr const char *LOGGING_TAG = "pylontech_group";

bool RingBuffer::push(char x) {
  if (this->full())
    return false;
  this->data_[this->head_] = x;
  this->head_ = (this->head_ + 1) % this->CAPACITY;
  this->size_++;
  return true;
}

char RingBuffer::peek(size_t offset) const { return this->data_[(this->tail_ + offset) % this->CAPACITY]; }

void RingBuffer::consume(size_t chars_to_remove) {
  chars_to_remove = std::min(chars_to_remove, this->size_);
  this->tail_ = (this->tail_ + chars_to_remove) % this->CAPACITY;
  this->size_ -= chars_to_remove;
}

void RingBuffer::clear() {
  this->head_ = 0;
  this->tail_ = 0;
  this->size_ = 0;
}

size_t RingBuffer::size() const { return this->size_; }

bool RingBuffer::empty() const { return this->size_ == 0; }

bool RingBuffer::full() const { return this->size_ == this->CAPACITY; }

void RingBuffer::to_string(char *buf) const {
  for (size_t i = 0; i < this->size_; i++) {
    buf[i] = this->peek(i);
  }
  buf[this->size_] = '\0';
}

PylontechGroup::PylontechGroup(std::string id) : uart::UARTDevice(), Component() { this->id_ = id; }

void PylontechGroup::setup() {}

void PylontechGroup::add_battery(pylontech_battery::PylontechBattery *battery) {
  this->configured_batteries_.push_back(battery);
}

void PylontechGroup::queue_command(const PylontechCommand &command) { this->commands_to_send_.push(command); }

const std::string &PylontechGroup::get_component_id() { return this->id_; }

void PylontechGroup::dump_config() {
  ESP_LOGCONFIG(LOGGING_TAG, "Pylontech Group component");
  ESP_LOGCONFIG(LOGGING_TAG, "  Group ID: %s", this->id_.c_str());
}

void PylontechGroup::loop() {
  const uint32_t loop_start_time = millis();

  // Process received data if applicable
  if (this->available()) {
    this->process_rx(loop_start_time);
  }

  // Send first queued packet if applicable, take into account minimum time between commands & awaiting response for
  // previously sent command
  if (!this->commands_to_send_.empty() && loop_start_time - this->last_send_time_ >= COMMAND_DELAY &&
      !this->waiting_for_response_) {
    this->send_command(this->commands_to_send_.front());
    this->last_send_time_ = loop_start_time;

    this->commands_to_send_.pop();
    ESP_LOGV(LOGGING_TAG, "Remaining commands in queue for group %s: %d", this->id_.c_str(),
             this->commands_to_send_.size());
  }
}

void PylontechGroup::send_command(const PylontechCommand &command) {
  this->last_sent_command_ = command;
  char buf[16];  // Buffer to hold the generated command string, max length is 8 (e.g. "stat 15") + null terminator
  snprintf(buf, sizeof(buf), "%s %d\n", command_to_string(command.command), command.battery_number);
  ESP_LOGD(LOGGING_TAG, "Sending command %s", buf);
  this->waiting_for_response_ = true;
  this->write_str(buf);  // Send command over UART
}

void PylontechGroup::process_rx(const uint32_t &loop_start_time) {
  // Drop all RX buffer contents on buffer overload
  if (this->receive_buffer_.full()) {
#if ESPHOME_LOG_LEVEL >= ESPHOME_LOG_LEVEL_VERBOSE
    char rx_buffer[this->receive_buffer_.size() + 1];
    this->receive_buffer_.to_string(rx_buffer);
    ESP_LOGV(LOGGING_TAG, "UART RX buffer contents: %s", rx_buffer);
#endif
    ESP_LOGW(LOGGING_TAG, "UART RX buffer for group %s has filled up. Clearing buffer...", this->id_.c_str());
    this->receive_buffer_.clear();

    // Drain UART hardware buffer
    uint8_t buf[64];
    while (this->available() > 0 && millis() - loop_start_time < 30) {
      const size_t to_drain = std::min((size_t) this->available(), sizeof(buf));
      if (!this->read_array(buf, to_drain))
        break;
    }
    return;
  }

  uint8_t read_buffer[READ_BATCH_SIZE];

  if (this->waiting_for_response_) {
    ESP_LOGD(LOGGING_TAG, "Started receiving response for command %s on group %s battery %d",
             this->last_sent_command_.command, this->id_.c_str(), this->last_sent_command_.battery_number);
  } else {
#if ESPHOME_LOG_LEVEL >= ESPHOME_LOG_LEVEL_WARNING
    char rx_buffer[this->receive_buffer_.size() + 1];
    this->receive_buffer_.to_string(rx_buffer);
    ESP_LOGW(LOGGING_TAG,
             "Received unexpected data on UART for group %s while not waiting for a response. Received data: %s",
             this->id_.c_str(), rx_buffer);
#endif
    this->receive_buffer_.clear();
    return;
  }

  while (this->available() && !this->receive_buffer_.full() &&
         millis() - loop_start_time < 30) {  // Avoid blocking the thread for 30ms+
    // Read available bytes in fixed-size batches to reduce UART call overhead
    const size_t avail = (size_t) this->available();
    const size_t to_read = std::min(avail, READ_BATCH_SIZE);
    ESP_LOGD(LOGGING_TAG, "%d bytes are available from UART, max buffer size is %d, reading %d bytes...", avail,
             READ_BATCH_SIZE, to_read);
    if (!this->read_array(read_buffer, to_read))
      break;

    for (size_t i = 0; i < to_read; i++)
      this->receive_buffer_.push((char) read_buffer[i]);

#if ESPHOME_LOG_LEVEL >= ESPHOME_LOG_LEVEL_VERBOSE
    char rx_buffer[this->receive_buffer_.size() + 1];
    this->receive_buffer_.to_string(rx_buffer);
    ESP_LOGV(LOGGING_TAG, "Updated receive_buffer_ contents: %s", rx_buffer);
#endif

    if (!this->available()) {
      ESP_LOGD(LOGGING_TAG, "Finished receiving response for command %s on group %s battery %d",
               this->last_sent_command_.command, this->id_.c_str(), this->last_sent_command_.battery_number);
      this->waiting_for_response_ = false;

      PylontechResponse response = PylontechResponse{};
      response.battery_number = this->last_sent_command_.battery_number;
      response.command = this->last_sent_command_.command;
      this->receive_buffer_.to_string(response.payload);
      this->receive_buffer_.clear();

      std::vector<pylontech_battery::PylontechBattery *>::iterator find_battery_it =
          std::find_if(this->configured_batteries_.begin(), this->configured_batteries_.end(),
                       [response](pylontech_battery::PylontechBattery *battery) {
                         return battery->get_battery_number() == response.battery_number;
                       });
      if (find_battery_it == this->configured_batteries_.end()) {
        ESP_LOGD(LOGGING_TAG, "Received unexpected response. Discarding...");
        break;
      }

      ESP_LOGD(LOGGING_TAG, "Received response for battery (%d).", response.battery_number);

      // Dispatch to battery
      (*find_battery_it)->handle_response(response);
      break;
    }
  }
}
}  // namespace pylontech_group
}  // namespace esphome
