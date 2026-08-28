#include "bt5_thermometer.h"
#include "esphome/core/log.h"
#include "const.h"

namespace esphome {
namespace bt5_thermometer {

static const char *const TAG = "bt5_thermometer";

void BT5Thermometer::setup() {
  // Initialize connection status sensor as disconnected
  this->connection_sensor_->publish_state(false);
  this->invalidate_probes_();
}

void BT5Thermometer::loop() {
  // Invalidate probes if we are connected to the thermometer and no data has been received for a while
  if (this->connection_sensor_->state && !this->is_stale_ &&
      millis() - this->last_notify_packet_received_timestamp_ > STALE_TIMEOUT_MS) {
    this->is_stale_ = true;
    ESP_LOGI(TAG, "%s stopped sending data updates. Marking probes as unknown.", this->thermometer_id_.c_str());
    this->invalidate_probes_();
  }
}

void BT5Thermometer::invalidate_probes_() {
  for (BT5ProbeSensor *probe : this->probes_) {
    if (probe->is_connected()) {
      probe->set_connected(false);
      probe->publish_state(NAN);
    }
  }
}

void BT5Thermometer::register_probe(BT5ProbeSensor *probe) { this->probes_.push_back(probe); }

void BT5Thermometer::gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
                                         esp_ble_gattc_cb_param_t *param) {
  switch (event) {
    case ESP_GATTC_SEARCH_CMPL_EVT: {
      ESP_LOGD(TAG, "Service discovery complete for %s", this->thermometer_id_.c_str());
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
      ESP_LOGI(TAG, "Connected to %s", this->thermometer_id_.c_str());
      this->connection_sensor_->publish_state(true);
      break;
    }
    case ESP_GATTC_DISCONNECT_EVT: {
      ESP_LOGI(TAG, "Disconnected from %s", this->thermometer_id_.c_str());
      this->connection_sensor_->publish_state(false);
      this->invalidate_probes_();
      break;
    }
    case ESP_GATTC_NOTIFY_EVT: {
      ESP_LOGD(TAG, "BLE Notify received on handle '%d' from %s", param->notify.handle, this->thermometer_id_.c_str());
      if (param->notify.handle == this->char_handle_) {
        this->last_notify_packet_received_timestamp_ = millis();
        if (this->is_stale_) {
          this->is_stale_ = false;
          ESP_LOGI(TAG, "%s resumed sending data updates.", this->thermometer_id_.c_str());
        }
        ESP_LOGD(TAG, "BLE Notify received for correct handle from %s, parsing response...",
                 this->thermometer_id_.c_str());
        this->parse_data_(param->notify.value, param->notify.value_len);
      }
      break;
    }
    default:
      break;
  }
}

void BT5Thermometer::parse_data_(const uint8_t *data, uint16_t length) {
  for (BT5ProbeSensor *probe : this->probes_) {
    uint16_t high_byte_id = probe->get_probe_number() * 2;
    uint16_t low_byte_id = high_byte_id + 1;

    if (low_byte_id < length) {
      uint16_t raw_temp = (data[high_byte_id] * 256) + data[low_byte_id];
      ESP_LOGD(TAG, "Parsed %s Probe %d raw temperature: %d", this->thermometer_id_.c_str(), probe->get_probe_number(),
               raw_temp);

      // Check if probe just disconnected and ignore already disconnected probe values
      if (raw_temp == INVALID_RAW_TEMP) {
        if (probe->is_connected()) {
          ESP_LOGI(TAG, "%s Probe %d was disconnected", this->thermometer_id_.c_str(), probe->get_probe_number());
          probe->set_connected(false);
          probe->publish_state(NAN);
        }
        continue;
      }

      // Check if probe just connected
      if (!probe->is_connected()) {
        ESP_LOGI(TAG, "%s Probe %d was connected", this->thermometer_id_.c_str(), probe->get_probe_number());
        probe->set_connected(true);
      }

      probe->publish_state((raw_temp / 10.0));
    }
  }
}
}  // namespace bt5_thermometer
}  // namespace esphome