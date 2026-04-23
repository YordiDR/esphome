#pragma once
#include <string>
#include <vector>
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/core/component.h"
#include "esphome/components/aa55_inverter/aa55_inverter_base_sensor.h"

namespace esphome {
namespace aa55_inverter {
class AA55InverterTextSensor : public AA55InverterBaseSensor, public text_sensor::TextSensor, public Component {
 public:
  AA55InverterTextSensor(std::string id, aa55_const::SENSOR_TYPE type, uint16_t skip_updates, bool offline_hold,
                         std::string offline_value);
  void process_response(const std::vector<uint8_t> &payload) override;
  void handle_inverter_offline() override;
  void dump_config() override;

 protected:
  std::string offline_value_{};

  void parse_ascii_payload(const std::vector<uint8_t> &payload);
  void parse_work_mode_payload(const std::vector<uint8_t> &payload);
  void parse_error_codes_payload(const std::vector<uint8_t> &payload);
};
}  // namespace aa55_inverter
}  // namespace esphome
