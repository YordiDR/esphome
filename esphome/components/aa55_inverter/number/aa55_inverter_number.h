#pragma once
#include <string>
#include "esphome/core/component.h"
#include "../../number/number.h"
#include "../aa55_inverter_base_input.h"

namespace esphome {
namespace aa55_inverter {

class AA55InverterNumber : public AA55InverterBaseInput, public number::Number, public Component {
 public:
  AA55InverterNumber(std::string id, aa55_inverter::INPUT_TYPE type, AA55Inverter *parent_inverter, bool offline_hold,
                     float offline_value, float online_initial_value);
  void setup() override;
  void dump_config() override;
  void control(float value);
  bool handles_response(aa55_bus::FUNCTION_CODE function_code) override;
  void handle_response(aa55_bus::FUNCTION_CODE function_code, uint8_t response) override;
  void handle_inverter_offline() override;
  void handle_inverter_online() override;

 protected:
  float last_sent_value_{}, offline_value_{}, online_initial_value_{};
};
}  // namespace aa55_inverter
}  // namespace esphome
