#pragma once
#include "esphome/core/log.h"
#include "const.h"

namespace esphome {
namespace aa55_inverter {

static const char *LOGGING_TAG = "aa55_inverter";

class AA55InverterBaseSensor {
 public:
  AA55InverterBaseSensor(std::string id, aa55_const::SENSOR_TYPE type, uint16_t skip_updates) {
    this->id_ = id;
    this->type_ = type;
    this->skip_updates_ = skip_updates;
    this->payload_location_ = aa55_const::MAP_SENSOR_PAYLOAD_LOCATION.at(type);
    this->payload_length_ = aa55_const::MAP_SENSOR_PAYLOAD_LENGTH.at(type);
    this->payload_source_ = aa55_const::MAP_SENSOR_RESPONSE_SOURCE.at(type);
  }

  std::string get_id() { return this->id_; }

  aa55_const::SENSOR_TYPE get_type() { return this->type_; }

  uint16_t get_skip_updates() { return this->skip_updates_; }

  aa55_const::FUNCTION_CODE get_payload_source() { return this->payload_source_; }

  uint16_t get_skipped_updates() { return this->skipped_updates_; }

  uint16_t increment_skipped_updates() { return ++this->skipped_updates_; }

  void reset_skipped_updates() { this->skipped_updates_ = 0; }

  bool time_to_update() { return this->skipped_updates_ == this->skip_updates_; }

 protected:
  uint16_t skip_updates_{0};
  uint16_t skipped_updates_{0};
  aa55_const::SENSOR_TYPE type_{};
  std::string id_{};
  uint8_t payload_location_{};
  uint8_t payload_length_{};
  aa55_const::FUNCTION_CODE payload_source_{};

  uint32_t parse_int(const std::vector<uint8_t> &payload) {
    uint32_t response = 0;

    // Safety check to prevent out-of-bounds crash
    if (this->payload_location_ + this->payload_length_ > payload.size()) {
      ESP_LOGE(LOGGING_TAG, "Buffer overflow in parse_int at index %d", this->payload_location_);
      return 0;
    }

    for (size_t i = 0; i < this->payload_length_; i++) {
      // Shift left 8 bits for each byte to maintain Big-Endian order
      response = (response << 8) | payload.at(this->payload_location_ + i);
    }

    return response;
  }
};

}  // namespace aa55_inverter
}  // namespace esphome
