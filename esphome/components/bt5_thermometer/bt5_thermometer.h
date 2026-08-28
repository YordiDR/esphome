#pragma once

#include <vector>
#include "esphome/core/component.h"
#include "esphome/components/ble_client/ble_client.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/binary_sensor/binary_sensor.h"

namespace esphome {
namespace bt5_thermometer {

class BT5ProbeSensor : public sensor::Sensor {
 public:
  explicit BT5ProbeSensor(uint16_t probe_number) : probe_number_(probe_number) {}
  uint8_t get_probe_number() const { return probe_number_; }

  bool is_connected() const { return connected_; }
  void set_connected(bool connected) { connected_ = connected; }

 protected:
  uint8_t probe_number_{0};
  bool connected_{true};
};

class BT5Thermometer : public ble_client::BLEClientNode, public Component {
 public:
  explicit BT5Thermometer(std::string thermometer_id) : thermometer_id_(thermometer_id) {}
  void setup() override;
  void loop() override;

  void register_probe(BT5ProbeSensor *probe);
  void set_connection_sensor(binary_sensor::BinarySensor *connection_sensor) {
    this->connection_sensor_ = connection_sensor;
  }

  void gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
                           esp_ble_gattc_cb_param_t *param) override;

  void set_update_interval(uint32_t update_interval) { this->update_interval_ = update_interval; }

 protected:
  uint16_t char_handle_{0};
  uint32_t update_interval_;
  uint32_t last_update_timestamp_{0};
  std::vector<BT5ProbeSensor *> probes_;
  binary_sensor::BinarySensor *connection_sensor_{nullptr};
  std::string thermometer_id_;
  uint32_t last_notify_packet_received_timestamp_{0};
  bool is_stale_{false};

  void parse_data_(const uint8_t *data, uint16_t length);
  void invalidate_probes_();
};

}  // namespace bt5_thermometer
}  // namespace esphome