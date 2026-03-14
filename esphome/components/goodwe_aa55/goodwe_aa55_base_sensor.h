#pragma once
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/component.h"

namespace esphome {
namespace goodwe_aa55 {

class GoodweAA55BaseSensor {
 public:
  uint16_t get_skip_updates() { return this->skip_updates_; }

  void set_skip_updates(uint16_t skip_updates) { this->skip_updates_ = skip_updates; }

  uint16_t get_skipped_updates() { return this->skipped_updates_; }

  uint16_t increment_skipped_updates() { return ++this->skipped_updates_; }

  void reset_skipped_updates() { this->skipped_updates_ = 0; }

  bool time_to_update() { return this->skipped_updates_ == this->skip_updates_; }

 protected:
  uint16_t skip_updates_{0};
  uint16_t skipped_updates_{0};
};

}  // namespace goodwe_aa55
}  // namespace esphome
