#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <algorithm>
#include "esphome/core/component.h"
#include "../uart/uart.h"
#include "const.h"

namespace esphome {
namespace pylontech_battery {
class PylontechBattery;  // Forward declaration of PylontechBattery class to avoid circular dependency
}
namespace pylontech_group {

// Ring buffer implementation for UART RX data. This allows us to efficiently parse incoming data without worrying about
// fragmentation or overflow as much.
class RingBuffer {
 public:
  bool push(char byte);
  char peek(size_t offset) const;        // Read char at offset from the tail without consuming it
  void consume(size_t bytes_to_remove);  // Discard n bytes from the front
  void clear();
  size_t size() const;
  bool empty() const;
  bool full() const;
  void to_string(char *buf) const;

 private:
  static constexpr size_t CAPACITY = MAX_BUFFER_LENGTH;
  char data_[CAPACITY]{};
  size_t head_{0}, tail_{0}, size_{0};
};

// Ring queue implementation for outgoing commands. This allows us to queue commands to be sent without worrying about
// dynamic memory allocation or overflow as much.
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
  void queue_command(const PylontechCommand &command);
  const std::string &get_component_id();

 protected:
  // Internal variables
  RingBuffer receive_buffer_;
  RingQueue<PylontechCommand, MAX_QUEUED_TX_COMMANDS> commands_to_send_;
  std::string id_;
  std::vector<pylontech_battery::PylontechBattery *> configured_batteries_;
  std::uint32_t last_send_time_{UINT32_MAX -
                                COMMAND_DELAY};  // Set to max - interval to allow sending immediately on startup
  PylontechCommand last_sent_command_{};  // Last command sent to batteries, used to correlate responses to commands
  bool waiting_for_response_{false};

  // Functions
  void send_command(const PylontechCommand &command);  // Function that generates the command and sends it via UART
  void process_rx(const uint32_t &loop_start_time);    // Function that parses incoming data from UART and dispatches
                                                       // directly to battery objects
};
}  // namespace pylontech_group
}  // namespace esphome
