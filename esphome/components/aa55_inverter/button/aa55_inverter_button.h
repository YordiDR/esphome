#pragma once
#include <string>
#include "esphome/components/button/button.h"
#include "esphome/core/component.h"
#include "esphome/components/aa55_inverter/aa55_inverter_base_input.h"

namespace esphome {
namespace aa55_inverter {

class AA55InverterButton : public AA55InverterBaseInput, public button::Button, public Component {
 public:
  AA55InverterButton(std::string id, aa55_const::INPUT_TYPE type, AA55Inverter *parent_inverter);
  void dump_config() override;
  void handle_response(aa55_const::FUNCTION_CODE function_code, uint8_t response) override;
  void handle_inverter_offline() override;
  void handle_inverter_online() override;

 protected:
  void press_action() override;
};

}  // namespace aa55_inverter
}  // namespace esphome
