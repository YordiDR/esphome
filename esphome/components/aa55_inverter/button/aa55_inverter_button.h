#pragma once
#include <string>
#include "esphome/core/component.h"
#include "../../button/button.h"
#include "../aa55_inverter_base_input.h"

namespace esphome {
namespace aa55_inverter {

class AA55InverterButton : public AA55InverterBaseInput, public button::Button, public Component {
 public:
  AA55InverterButton(const std::string &id, aa55_inverter::INPUT_TYPE type, AA55Inverter *parent_inverter);
  void dump_config() override;
  bool handles_response(aa55_bus::FUNCTION_CODE function_code) override;
  void handle_response(aa55_bus::FUNCTION_CODE function_code, uint8_t response) override;
  void handle_inverter_offline() override;
  void handle_inverter_online() override;

 protected:
  void press_action() override;
};

}  // namespace aa55_inverter
}  // namespace esphome
