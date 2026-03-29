#pragma once
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/component.h"
#include "const.h"

namespace esphome {
namespace aa55_inverter {

static const char *LOGGING_TAG = "aa55_inverter";

class AA55InverterBaseSensor {
 public:
  uint16_t get_skip_updates() { return this->skip_updates_; }

  void set_skip_updates(uint16_t skip_updates) { this->skip_updates_ = skip_updates; }

  uint16_t get_skipped_updates() { return this->skipped_updates_; }

  uint16_t increment_skipped_updates() { return ++this->skipped_updates_; }

  void reset_skipped_updates() { this->skipped_updates_ = 0; }

  bool time_to_update() { return this->skipped_updates_ == this->skip_updates_; }

  aa55_const::SENSOR_TYPE get_type() { return this->type_; }

  void set_type(aa55_const::SENSOR_TYPE type) { this->type_ = type; }

  std::string get_id() { return this->id_; }

  void set_id(std::string id) { this->id_ = id; }

  uint8_t get_payload_location() { return aa55_const::MAP_SENSOR_PAYLOAD_LOCATION.at(this->type_); }

  uint8_t get_payload_length() { return aa55_const::MAP_SENSOR_PAYLOAD_LENGTH.at(this->type_); }

 protected:
  uint16_t skip_updates_{0};
  uint16_t skipped_updates_{0};
  aa55_const::SENSOR_TYPE type_{};
  std::string id_{};

  uint32_t parse_int(const std::vector<uint8_t> &payload) {
    uint32_t response = 0;

    // Safety check to prevent out-of-bounds crash
    if (this->get_payload_location() + this->get_payload_length() > payload.size()) {
      ESP_LOGE(LOGGING_TAG, "Buffer overflow in parse_int at index %d", this->get_payload_location());
      return 0;
    }

    for (size_t i = 0; i < this->get_payload_length(); i++) {
      // Shift left 8 bits for each byte to maintain Big-Endian order
      response = (response << 8) | payload.at(this->get_payload_location() + i);
    }

    return response;
  }
};

}  // namespace aa55_inverter
}  // namespace esphome
