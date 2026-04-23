#pragma once
#include "const.h"

namespace esphome {
namespace aa55_inverter {

class AA55Inverter;  // Forward declaration of AA55Inverter class to avoid circular dependency

class AA55InverterBaseInput {
 public:
  AA55InverterBaseInput(std::string id, aa55_inverter::INPUT_TYPE type, AA55Inverter *parent_inverter,
                        bool offline_hold);
  aa55_inverter::INPUT_TYPE get_type();
  std::string get_id();
  aa55_bus::FUNCTION_CODE get_response_function_code();
  virtual void handle_response(aa55_bus::FUNCTION_CODE function_code, uint8_t response) = 0;
  virtual void handle_inverter_offline() = 0;
  virtual void handle_inverter_online() = 0;

 protected:
  aa55_inverter::INPUT_TYPE type_{};
  std::string id_{};
  aa55_bus::FUNCTION_CODE response_function_code_{};
  AA55Inverter *parent_inverter_{nullptr};
  bool offline_hold_{};
};

}  // namespace aa55_inverter
}  // namespace esphome
