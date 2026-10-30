#pragma once
#include <string>
#include "const.h"

namespace esphome {
namespace pylontech_battery {

class PylontechBatteryBaseSensor {
 public:
  PylontechBatteryBaseSensor(const std::string &id, pylontech_battery::SENSOR_TYPE type, uint16_t skip_updates);
  virtual ~PylontechBatteryBaseSensor() = default;
  const std::string &get_id();
  SENSOR_TYPE get_type();
  pylontech_group::COMMAND get_payload_source();

  /// Called with fresh data when this sensor's command completed and the sensor is due. Publishes the value.
  virtual void process_data(const BatteryData &data) = 0;
  /// Called when the battery stopped answering.
  virtual void on_offline() {}

  // skip_updates bookkeeping. A sensor is "due" when its countdown reached zero; the battery only queues
  // commands for due sensors and calls mark_published() once the value has been delivered.
  bool is_due() const { return this->countdown_ == 0; }
  void tick() {
    if (this->countdown_ > 0)
      this->countdown_--;
  }
  // Device information from `info` never changes, so once published it is not requested again for a very long time
  // (65535 update intervals, i.e. days). A failed request leaves the sensor due, so it is retried on the next update.
  void mark_published() {
    this->countdown_ = this->payload_source_ == pylontech_group::COMMAND::INFO ? UINT16_MAX : this->skip_updates_;
  }

 protected:
  uint16_t skip_updates_{0};
  uint16_t countdown_{0};
  SENSOR_TYPE type_{};
  std::string id_{};
  pylontech_group::COMMAND payload_source_{};
};

}  // namespace pylontech_battery
}  // namespace esphome
