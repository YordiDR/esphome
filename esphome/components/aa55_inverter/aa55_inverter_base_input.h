#pragma once
#include "const.h"

namespace esphome {
namespace aa55_inverter {
class AA55Inverter;  // Forward declaration of AA55Inverter class to avoid circular dependency

class AA55InverterBaseInput {
 public:
  aa55_const::INPUT_TYPE get_type() { return this->type_; }

  void set_type(aa55_const::INPUT_TYPE type) { this->type_ = type; }

  std::string get_id() { return this->id_; }

  void set_id(std::string id) { this->id_ = id; }

  void set_parent_inverter(AA55Inverter *inverter) { this->parent_inverter_ = inverter; }

  virtual void handle_response(aa55_const::FUNCTION_CODE function_code, uint8_t response) = 0;

 protected:
  aa55_const::INPUT_TYPE type_{};
  std::string id_{};
  AA55Inverter *parent_inverter_{nullptr};
};

}  // namespace aa55_inverter
}  // namespace esphome
