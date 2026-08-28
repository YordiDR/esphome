#pragma once

#include <vector>
#include "esphome/core/component.h"
#include "esphome/components/ble_client/ble_client.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace bt5_thermometer {

class BT5ProbeSensor : public sensor::Sensor {
 public:
  explicit BT5ProbeSensor(uint16_t probe_number) : probe_number_(probe_number) {}
  uint8_t get_probe_number() const { return probe_number_; }

 protected:
  uint8_t probe_number_{0};
};

class BT5Thermometer : public ble_client::BLEClientNode, public Component {
 public:
  void setup() override;
  void loop() override;

  void register_probe(BT5ProbeSensor *probe);
  void gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
                           esp_ble_gattc_cb_param_t *param) override;

 protected:
  uint16_t char_handle_{0};
  std::vector<BT5ProbeSensor *> probes_;

  void parse_data_(const uint8_t *data, uint16_t length);
};

}  // namespace bt5_thermometer
}  // namespace esphome