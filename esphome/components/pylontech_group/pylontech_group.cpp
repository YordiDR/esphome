#include <algorithm>
#include "esphome/core/log.h"
#include "esphome/core/hal.h"
#include "pylontech_group.h"
#include "../pylontech_battery/pylontech_battery.h"

namespace esphome {
namespace pylontech_group {
static constexpr const char *LOGGING_TAG = "pylontech_group";

PylontechGroup::PylontechGroup(std::string id) : uart::UARTDevice(), Component() { this->id_ = id; }

void PylontechGroup::setup() {
  // Wake up the console. Its answer is just a prompt; update_session_() uses it to find out whether we are in
  // pylon> (limited) or pylon_debug> (full) mode before any command is sent.
  this->write_str("\n");
  this->probe_count_ = 1;
  this->session_state_ = SESSION_STATE::PROBING;
  this->session_state_since_ = this->last_done_time_ = millis();
}

void PylontechGroup::dump_config() {
  ESP_LOGCONFIG(LOGGING_TAG, "Pylontech Group component");
  ESP_LOGCONFIG(LOGGING_TAG, "  Group ID: %s", this->id_.c_str());
  ESP_LOGCONFIG(LOGGING_TAG, "  Batteries: %u", (unsigned) this->configured_batteries_.size());
}

void PylontechGroup::add_battery(pylontech_battery::PylontechBattery *battery) {
  this->configured_batteries_.push_back(battery);
}

bool PylontechGroup::has_pending(const PylontechCommand &command) const {
  if (this->waiting_for_response_ && this->last_sent_command_ == command)
    return true;
  return this->commands_to_send_.contains_if([&command](const PylontechCommand &c) { return c == command; });
}

void PylontechGroup::queue_command(const PylontechCommand &command) {
  if (this->has_pending(command)) {
    ESP_LOGV(LOGGING_TAG, "Command %s %u already pending, not queueing again", command_to_string(command.command),
             (unsigned) command.battery_number);
    return;
  }
  if (!this->commands_to_send_.push(command)) {
    ESP_LOGW(LOGGING_TAG, "Command queue of group %s is full, dropping '%s %u'", this->id_.c_str(),
             command_to_string(command.command), (unsigned) command.battery_number);
  }
}

const std::string &PylontechGroup::get_component_id() { return this->id_; }

pylontech_battery::PylontechBattery *PylontechGroup::find_battery_(uint8_t battery_number) {
  for (auto *battery : this->configured_batteries_) {
    if (battery->get_battery_number() == battery_number)
      return battery;
  }
  return nullptr;
}

void PylontechGroup::loop() {
  if (this->available()) {
    this->process_rx_();
  }

  const uint32_t now = millis();

  // Give up on a response that never completes, otherwise the whole group would stall forever.
  if (this->waiting_for_response_ && now - this->last_send_time_ > RESPONSE_TIMEOUT) {
    ESP_LOGW(LOGGING_TAG, "Timeout waiting for response to '%s %u' in group %s",
             command_to_string(this->last_sent_command_.command), (unsigned) this->last_sent_command_.battery_number,
             this->id_.c_str());
    this->finish_response_(false);
  }

  // Make sure the console is in pylon_debug> mode before talking to it
  this->update_session_(millis());
  if (this->session_state_ != SESSION_STATE::READY)
    return;

  // Send the next queued command (strict FIFO), respecting the minimum delay between commands
  if (!this->waiting_for_response_ && !this->commands_to_send_.empty() &&
      millis() - this->last_done_time_ >= COMMAND_DELAY) {
    this->send_command_(this->commands_to_send_.front());
    this->commands_to_send_.pop();
    ESP_LOGV(LOGGING_TAG, "Remaining commands in queue for group %s: %u", this->id_.c_str(),
             (unsigned) this->commands_to_send_.size());
  }
}

void PylontechGroup::drain_rx_() {
  uint8_t drain[32];
  while (this->available() > 0) {
    const size_t n = std::min((size_t) this->available(), sizeof(drain));
    if (!this->read_array(drain, n))
      break;
  }
  this->line_len_ = 0;
}

void PylontechGroup::observe_prompt_(const char *text) {
  if (strncmp(text, PYLON_PROMPT_DEBUG, strlen(PYLON_PROMPT_DEBUG)) == 0)
    this->prompt_ = PROMPT::DEBUG;
  else if (strncmp(text, PYLON_PROMPT_NORMAL, strlen(PYLON_PROMPT_NORMAL)) == 0)
    this->prompt_ = PROMPT::NORMAL;
}

void PylontechGroup::start_login_(uint32_t now) {
  this->drain_rx_();
  this->prompt_ = PROMPT::UNKNOWN;  // the prompt printed after the login tells us whether it worked
  this->login_attempts_++;
  this->session_state_ = SESSION_STATE::LOGGING_IN;
  this->session_state_since_ = now;
  ESP_LOGD(LOGGING_TAG, "Sending '%s' (attempt %u/%u) in group %s", PYLON_LOGIN_COMMAND,
           (unsigned) this->login_attempts_, (unsigned) MAX_LOGIN_ATTEMPTS, this->id_.c_str());
  this->write_str(PYLON_LOGIN_COMMAND);
  this->write_str("\n");
}

void PylontechGroup::update_session_(uint32_t now) {
  switch (this->session_state_) {
    case SESSION_STATE::PROBING:
      if (this->prompt_ == PROMPT::DEBUG) {
        ESP_LOGI(LOGGING_TAG, "Console of group %s is already at the %s prompt", this->id_.c_str(), PYLON_PROMPT_DEBUG);
        this->session_state_ = SESSION_STATE::READY;
      } else if (this->prompt_ == PROMPT::NORMAL) {
        ESP_LOGD(LOGGING_TAG, "Console of group %s is at the %s prompt, logging in", this->id_.c_str(),
                 PYLON_PROMPT_NORMAL);
        this->login_attempts_ = 0;
        this->start_login_(now);
      } else if (now - this->session_state_since_ >= PROMPT_PROBE_INTERVAL) {
        if (this->probe_count_ >= MAX_PROMPT_PROBES) {
          // Carry on: commands will time out individually and show up in the log. If a prompt appears later it
          // is picked up by the READY branch below.
          ESP_LOGW(LOGGING_TAG, "No prompt received from console of group %s, continuing without handshake",
                   this->id_.c_str());
          this->session_state_ = SESSION_STATE::READY;
        } else {
          this->probe_count_++;
          this->session_state_since_ = now;
          this->write_str("\n");
        }
      }
      break;

    case SESSION_STATE::LOGGING_IN:
      if (this->prompt_ == PROMPT::DEBUG) {
        ESP_LOGI(LOGGING_TAG, "Console of group %s elevated to the %s prompt", this->id_.c_str(), PYLON_PROMPT_DEBUG);
        this->session_state_ = SESSION_STATE::READY;
        this->login_failed_ = false;
        this->last_done_time_ = now;  // keep COMMAND_DELAY before the first real command
      } else if (now - this->session_state_since_ >= LOGIN_TIMEOUT) {
        if (this->login_attempts_ >= MAX_LOGIN_ATTEMPTS) {
          ESP_LOGW(LOGGING_TAG,
                   "Could not log in to console of group %s, continuing in %s mode (commands like 'stat' may fail)",
                   this->id_.c_str(), PYLON_PROMPT_NORMAL);
          this->session_state_ = SESSION_STATE::READY;
          this->login_failed_ = true;
          this->login_failed_time_ = now;
          this->last_done_time_ = now;
        } else {
          this->start_login_(now);
        }
      }
      break;

    case SESSION_STATE::READY:
      // The console can drop back to pylon> (reset, idle logout, ...). Notice it at the next idle moment.
      if (!this->waiting_for_response_ && this->prompt_ == PROMPT::NORMAL &&
          now - this->last_done_time_ >= COMMAND_DELAY &&
          (!this->login_failed_ || now - this->login_failed_time_ >= LOGIN_RETRY_DELAY)) {
        ESP_LOGW(LOGGING_TAG, "Console of group %s is back at the %s prompt, logging in again", this->id_.c_str(),
                 PYLON_PROMPT_NORMAL);
        this->login_attempts_ = 0;
        this->start_login_(now);
      }
      break;
  }
}

void PylontechGroup::send_command_(const PylontechCommand &command) {
  // Drop stale bytes (typically the trailing prompt of the previous command)
  this->drain_rx_();

  this->last_sent_command_ = command;
  this->waiting_for_response_ = true;
  this->response_started_ = false;
  this->last_send_time_ = millis();

  char buf[MAX_COMMAND_LENGTH + 1];
  snprintf(buf, sizeof(buf), "%s %u\n", command_to_string(command.command), (unsigned) command.battery_number);
  ESP_LOGD(LOGGING_TAG, "Sending command %s %u", command_to_string(command.command), (unsigned) command.battery_number);
  this->write_str(buf);
}

void PylontechGroup::process_rx_() {
  const uint32_t start = millis();
  uint8_t read_buffer[READ_BATCH_SIZE];

  // Bounded so a chatty console can't block the main loop for long
  while (this->available() > 0 && millis() - start < RX_TIME_BUDGET) {
    const size_t to_read = std::min((size_t) this->available(), READ_BATCH_SIZE);
    if (!this->read_array(read_buffer, to_read))
      break;
    for (size_t i = 0; i < to_read; i++) {
      this->push_char_((char) read_buffer[i]);
    }
  }

  // The prompt ("pylon>" / "pylon_debug>") is not followed by a newline, so it would never complete a line.
  // Recognise it while it sits alone in the line buffer.
  this->line_[this->line_len_] = '\0';
  if (this->line_len_ == strlen(PYLON_PROMPT_NORMAL) || this->line_len_ == strlen(PYLON_PROMPT_DEBUG))
    this->observe_prompt_(this->line_);
}

void PylontechGroup::push_char_(char c) {
  if (c == '\n') {
    this->line_[this->line_len_] = '\0';
    this->process_line_(this->line_);
    this->line_len_ = 0;
  } else if (c != '\r') {
    // Characters beyond the line buffer are dropped; no console line we care about is that long.
    if (this->line_len_ < MAX_LINE_LENGTH - 1)
      this->line_[this->line_len_++] = c;
  }
}

void PylontechGroup::process_line_(char *line) {
  // Trim
  while (*line == ' ' || *line == '\t')
    line++;
  size_t n = strlen(line);
  while (n > 0 && (line[n - 1] == ' ' || line[n - 1] == '\t'))
    line[--n] = '\0';
  if (n == 0)
    return;

  ESP_LOGVV(LOGGING_TAG, "RX: %s", line);

  this->observe_prompt_(line);

  if (!this->waiting_for_response_) {
    ESP_LOGV(LOGGING_TAG, "Ignoring line while idle: %s", line);
    return;
  }

  if (strcmp(line, PYLON_COMMAND_END) == 0) {
    this->finish_response_(this->response_started_);
    return;
  }
  if (strcmp(line, PYLON_COMMAND_START) == 0) {
    this->response_started_ = true;
    return;
  }
  // Prompt and command echo ("pylon>pwr 1")
  if (strncmp(line, PYLON_PROMPT_PREFIX, strlen(PYLON_PROMPT_PREFIX)) == 0)
    return;
  if (!this->response_started_)
    return;

  auto *battery = this->find_battery_(this->last_sent_command_.battery_number);
  if (battery != nullptr)
    battery->handle_line(this->last_sent_command_.command, line);
}

void PylontechGroup::finish_response_(bool success) {
  const PylontechCommand command = this->last_sent_command_;
  this->waiting_for_response_ = false;
  this->response_started_ = false;
  this->last_done_time_ = millis();

  auto *battery = this->find_battery_(command.battery_number);
  if (battery == nullptr) {
    ESP_LOGD(LOGGING_TAG, "Response for unconfigured battery %u discarded", (unsigned) command.battery_number);
    return;
  }
  ESP_LOGD(LOGGING_TAG, "Response to '%s %u' %s", command_to_string(command.command), (unsigned) command.battery_number,
           success ? "complete" : "FAILED");
  battery->handle_response_end(command.command, success);
}

}  // namespace pylontech_group
}  // namespace esphome
