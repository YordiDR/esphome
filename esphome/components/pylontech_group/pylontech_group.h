#pragma once
#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>
#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "const.h"

namespace esphome {
namespace pylontech_battery {
class PylontechBattery;  // Forward declaration of PylontechBattery class to avoid circular dependency
}
namespace pylontech_group {

// Fixed-capacity ring queue for outgoing commands (no dynamic allocation).
template<typename T, size_t CAPACITY> class RingQueue {
 public:
  bool push(T item) {
    if (full())
      return false;
    this->data_[this->head_] = item;
    this->head_ = (this->head_ + 1) % CAPACITY;
    this->size_++;
    return true;
  }

  T &front() { return this->data_[this->tail_]; }
  const T &front() const { return this->data_[this->tail_]; }

  void pop() {
    if (empty())
      return;
    this->tail_ = (this->tail_ + 1) % CAPACITY;
    this->size_--;
  }

  void clear() { this->head_ = this->tail_ = this->size_ = 0; }

  size_t size() const { return this->size_; }
  bool empty() const { return this->size_ == 0; }
  bool full() const { return this->size_ == CAPACITY; }

  template<typename Predicate> bool contains_if(Predicate pred) const {
    for (size_t i = 0; i < this->size_; i++) {
      if (pred(this->data_[(this->tail_ + i) % CAPACITY]))
        return true;
    }
    return false;
  }

 private:
  T data_[CAPACITY]{};
  size_t head_{0}, tail_{0}, size_{0};
};

class PylontechGroup : public uart::UARTDevice, public Component {
 public:
  PylontechGroup(std::string id);
  void setup() override;
  void dump_config() override;
  void loop() override;
  void add_battery(pylontech_battery::PylontechBattery *battery);
  /// Queue a command. Silently ignored if an identical command is already queued or in flight.
  void queue_command(const PylontechCommand &command);
  /// True if this exact command is queued or waiting for its response.
  bool has_pending(const PylontechCommand &command) const;
  const std::string &get_component_id();
  /// True once the session handshake is finished and queued commands are being sent.
  bool is_ready() const { return this->session_state_ == SESSION_STATE::READY; }
  PROMPT get_prompt() const { return this->prompt_; }

 protected:
  // Internal variables
  RingQueue<PylontechCommand, MAX_QUEUED_TX_COMMANDS> commands_to_send_;
  std::string id_;
  std::vector<pylontech_battery::PylontechBattery *> configured_batteries_;
  char line_[MAX_LINE_LENGTH]{};  // Line currently being assembled from UART bytes
  size_t line_len_{0};
  uint32_t last_send_time_{0};            // When the in-flight command was sent (for the response timeout)
  uint32_t last_done_time_{0};            // When the previous command finished (for COMMAND_DELAY)
  PylontechCommand last_sent_command_{};  // Command in flight, used to route response lines
  bool waiting_for_response_{false};
  bool response_started_{false};  // '@' seen

  // Console session (pylon> / pylon_debug>)
  PROMPT prompt_{PROMPT::UNKNOWN};
  SESSION_STATE session_state_{SESSION_STATE::PROBING};
  uint8_t probe_count_{0};
  uint8_t login_attempts_{0};
  uint32_t session_state_since_{0};
  bool login_failed_{false};
  uint32_t login_failed_time_{0};

  // Functions
  void send_command_(const PylontechCommand &command);
  void process_rx_();
  void push_char_(char c);
  void process_line_(char *line);
  void finish_response_(bool success);
  void update_session_(uint32_t now);  // Prompt probing / login state machine
  void start_login_(uint32_t now);
  void observe_prompt_(const char *text);  // Records which prompt (if any) `text` starts with
  void drain_rx_();
  pylontech_battery::PylontechBattery *find_battery_(uint8_t battery_number);
};

}  // namespace pylontech_group
}  // namespace esphome