#pragma once
#include <string>
#include "esphome/components/switch/switch.h"
#include "esphome/core/component.h"
#include "esphome/components/aa55_inverter/aa55_inverter_base_input.h"

namespace esphome {
namespace aa55_inverter {

class AA55InverterSwitch : public AA55InverterBaseInput, public switch_::Switch, public Component {
 public:
  AA55InverterSwitch(std::string id, aa55_const::INPUT_TYPE type, AA55Inverter *parent_inverter, bool offline_hold,
                     aa55_const::ON_OFF offline_value, aa55_const::ON_OFF online_initial_value);
  void setup() override;
  void dump_config() override;
  void write_state(bool state);
  void handle_response(aa55_const::FUNCTION_CODE function_code, uint8_t response) override;
  void handle_inverter_offline() override;
  void handle_inverter_online() override;

 protected:
  bool last_sent_value_{};
  aa55_const::ON_OFF offline_value_{}, online_initial_value_{};
};
}  // namespace aa55_inverter
}  // namespace esphome
