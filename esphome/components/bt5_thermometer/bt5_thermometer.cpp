#include "bt5_thermometer.h"
#include "esphome/core/log.h"

namespace esphome {
namespace bt5_thermometer {

static const char *const TAG = "bt5_thermometer";

void BT5Thermometer::setup() {
  // Initialize connection status sensor as disconnected
  if (this->connection_sensor_ != nullptr) {
    this->connection_sensor_->publish_state(false);
  }
}

void BT5Thermometer::loop() {}

void BT5Thermometer::register_probe(BT5ProbeSensor *probe) { this->probes_.push_back(probe); }

void BT5Thermometer::gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
                                         esp_ble_gattc_cb_param_t *param) {
  switch (event) {
    case ESP_GATTC_SEARCH_CMPL_EVT: {
      ESP_LOGD(TAG, "Service discovery complete for BT5 Thermometer with ID: %s", this->thermometer_id_.c_str());
      esp32_ble_client::BLECharacteristic *char_desc = this->parent()->get_characteristic(
          esp32_ble_tracker::ESPBTUUID::from_uint16(0xFFB0), esp32_ble_tracker::ESPBTUUID::from_uint16(0xFFB2));

      if (char_desc != nullptr) {
        this->char_handle_ = char_desc->handle;
        esp_ble_gattc_register_for_notify(this->parent()->get_gattc_if(), this->parent()->get_remote_bda(),
                                          char_desc->handle);
      }
      break;
    }
    case ESP_GATTC_CONNECT_EVT: {
      ESP_LOGI(TAG, "Connected to BT5 Thermometer with ID: %s", this->thermometer_id_.c_str());
      if (this->connection_sensor_ != nullptr) {
        this->connection_sensor_->publish_state(true);
      }
      break;
    }
    case ESP_GATTC_DISCONNECT_EVT: {
      ESP_LOGI(TAG, "Disconnected from BT5 Thermometer with ID: %s", this->thermometer_id_.c_str());
      if (this->connection_sensor_ != nullptr) {
        this->connection_sensor_->publish_state(false);
      }
      break;
    }
    case ESP_GATTC_NOTIFY_EVT: {
      ESP_LOGD(TAG, "BLE Notify received on handle '%d' from BT5 Thermometer with ID: %s", param->notify.handle,
               this->thermometer_id_.c_str());
      if (param->notify.handle == this->char_handle_) {
        ESP_LOGD(TAG, "BLE Notify received for correct handle from BT5 Thermometer with ID: %s, parsing response...",
                 this->thermometer_id_.c_str());
        this->parse_data_(param->notify.value, param->notify.value_len);
      }
      break;
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
      ESP_LOGD(TAG, "Parsed BT5 thermometer '%s Probe %d' raw temperature: %d", this->thermometer_id_.c_str(),
               probe_num, raw_temp);
      if (raw_temp != 65535) {
        probe->publish_state((uint16_t) (raw_temp / 10.0));
      }
    }
  }
}
}  // namespace bt5_thermometer
}  // namespace esphome