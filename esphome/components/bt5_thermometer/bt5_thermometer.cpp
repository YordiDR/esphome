#include "bt5_thermometer.h"
#include "esphome/core/log.h"

namespace esphome {
namespace bt5_thermometer {

static const char *const TAG = "bt5_thermometer";

void BT5Thermometer::setup() {}
void BT5Thermometer::loop() {}

void BT5Thermometer::register_probe(BT5ProbeSensor *probe) { this->probes_.push_back(probe); }

void BT5Thermometer::gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
                                         esp_ble_gattc_cb_param_t *param) {
  if (event == ESP_GATTC_SEARCH_CMPL_EVT) {
    esp32_ble_tracker::BLECharacteristic *char_desc = this->parent()->get_characteristic(
        esp32_ble_tracker::ESPBTUUID::from_uuid16(0xFFB0), esp32_ble_tracker::ESPBTUUID::from_uuid16(0xFFB2));

    if (char_desc != nullptr) {
      this->char_handle_ = char_desc->handle;
      esp_ble_gattc_register_for_notify(this->parent()->get_gattc_if(), this->parent()->get_remote_bda(),
                                        char_desc->handle);
    }
  } else if (event == ESP_GATTC_NOTIFY_EVT) {
    if (param->notify.handle == this->char_handle_) {
      this->parse_data_(param->notify.value, param->notify.value_len);
    }
  }
}

void BT5Thermometer::parse_data_(const uint8_t *data, uint16_t length) {
  for (BT5ProbeSensor *probe : this->probes_) {
    uint8_t probe_num = probe->get_probe_number();

    uint16_t high_byte_id = probe_num * 2;
    uint16_t low_byte_id = (probe_num * 2) + 1;

    if (low_byte_id < length) {
      uint16_t raw_temp = (data[high_byte_id] * 256) + data[low_byte_id];
      if (raw_temp != 65535) {
        probe->publish_state((uint16_t) (raw_temp / 10.0));
      }
    }
  }
}
}  // namespace bt5_thermometer
}  // namespace esphome