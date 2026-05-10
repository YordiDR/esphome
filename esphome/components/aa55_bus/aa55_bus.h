#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <algorithm>
#include "esphome/core/component.h"
#include "../uart/uart.h"
#include "const.h"

namespace esphome {
namespace aa55_inverter {
class AA55Inverter;  // Forward declaration of AA55Inverter class to avoid circular dependency
}
namespace aa55_bus {

// Ring buffer implementation for UART RX data. This allows us to efficiently parse incoming data without worrying about
// fragmentation or overflow as much.
class RingBuffer {
 public:
  bool push(uint8_t byte);
  uint8_t peek(size_t offset) const;     // Read byte at offset from the tail without consuming it
  void consume(size_t bytes_to_remove);  // Discard n bytes from the front
  void clear();
  size_t size() const;
  bool empty() const;
  bool full() const;

 private:
  static constexpr size_t CAPACITY = aa55_bus::MAX_BUFFER_LENGTH;
  uint8_t data_[CAPACITY]{};
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

class AA55Bus : public uart::UARTDevice, public Component {
 public:
  AA55Bus(std::string id, uint8_t controller_address);
  void setup() override;
  void dump_config() override;
  void loop() override;
  void add_inverter(aa55_inverter::AA55Inverter *inverter);
  void queue_command(aa55_bus::AA55TXPacket command);
  std::string get_component_id();
  void add_registered_inverter(aa55_inverter::AA55Inverter *inverter);
  void remove_registered_inverter(aa55_inverter::AA55Inverter *inverter);

 protected:
  // Internal variables
  uint8_t controller_address_;
  RingBuffer receive_buffer_;
  RingQueue<aa55_bus::AA55TXPacket, aa55_bus::MAX_QUEUED_TX_COMMANDS> commands_to_send_;
  std::string id_;
  std::vector<aa55_inverter::AA55Inverter *> configured_inverters_;
  std::vector<aa55_inverter::AA55Inverter *>
      registered_inverters_;  // Subset of configured inverters that have been registered on the bus
  std::uint32_t last_send_time_{
      UINT32_MAX - aa55_bus::COMMAND_DELAY};  // Set to max - interval to allow sending immediately on startup
  std::uint32_t last_offline_request_send_time_{
      UINT32_MAX - aa55_bus::OFFLINE_QUERY_INTERVAL};  // Set to max - interval to allow sending immediately on startup

  // Functions
  void send_packet(const aa55_bus::AA55TXPacket &command);  // Function that generates the packet and sends it via UART
  void process_rx(const uint32_t &loop_start_time);  // Function that parses incoming data from UART and dispatches
                                                     // directly to inverter objects
  std::string create_hex_string(const uint8_t *data, size_t length);
  std::string create_hex_string(const RingBuffer &buffer);
  template<typename T> std::string create_hex_string(const T &data) {
    std::string result;
    result.reserve(data.size() * 3);

    const char *hex = "0123456789ABCDEF";

    for (uint8_t byte : data) {
      result.push_back(hex[(byte >> 4) & 0xF]);
      result.push_back(hex[byte & 0xF]);
      result.push_back(' ');
    }

    if (!result.empty())
      result.pop_back();  // Remove trailing space
    return result;
  }
};

}  // namespace aa55_bus
}  // namespace esphome
