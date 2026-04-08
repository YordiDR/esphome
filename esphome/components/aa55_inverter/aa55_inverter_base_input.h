#pragma once
#include "const.h"

namespace esphome {
namespace aa55_inverter {
class AA55Inverter;  // Forward declaration of AA55Inverter class to avoid circular dependency

class AA55InverterBaseInput {
 public:
  AA55InverterBaseInput(std::string id, aa55_const::INPUT_TYPE type, AA55Inverter *parent_inverter) {
    this->id_ = id;
    this->type_ = type;
    this->parent_inverter_ = parent_inverter;
    this->response_function_code_ = aa55_const::MAP_INPUT_RESPONSE.at(type);
  }

  aa55_const::INPUT_TYPE get_type() { return this->type_; }

  std::string get_id() { return this->id_; }

  virtual void handle_response(aa55_const::FUNCTION_CODE function_code, uint8_t response) = 0;

  aa55_const::FUNCTION_CODE get_response_function_code() { return this->response_function_code_; }

 protected:
  aa55_const::INPUT_TYPE type_{};
  std::string id_{};
  aa55_const::FUNCTION_CODE response_function_code_{};
  AA55Inverter *parent_inverter_{nullptr};
};

}  // namespace aa55_inverter
}  // namespace esphome
