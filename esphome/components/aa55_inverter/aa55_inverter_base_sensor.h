#pragma once
#include "const.h"
#include <string>

namespace esphome {
namespace aa55_inverter {

class AA55InverterBaseSensor {
 public:
  AA55InverterBaseSensor(const std::string &id, aa55_inverter::SENSOR_TYPE type, uint16_t skip_updates,
                         bool offline_hold);
  const std::string &get_id();
  aa55_inverter::SENSOR_TYPE get_type();
  aa55_bus::FUNCTION_CODE get_payload_source();
  virtual void process_response(const uint8_t *payload, uint8_t payload_length) = 0;
  virtual void handle_inverter_offline() = 0;
  void force_next_update();

 protected:
  uint16_t skip_updates_{0};
  uint16_t skipped_updates_{0};
  aa55_inverter::SENSOR_TYPE type_{};
  std::string id_{};
  uint8_t payload_location_{};
  uint8_t payload_length_{};
  aa55_bus::FUNCTION_CODE payload_source_{};
  bool offline_hold_{}, force_next_update_{false};

  bool time_to_update();
  uint32_t parse_int(const uint8_t *payload, uint8_t payload_length);
};

}  // namespace aa55_inverter
}  // namespace esphome
