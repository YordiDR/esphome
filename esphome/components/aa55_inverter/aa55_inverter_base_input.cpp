#include "aa55_inverter_base_input.h"

namespace esphome {
namespace aa55_inverter {
static const char *LOGGING_TAG = "aa55_input";

AA55InverterBaseInput::AA55InverterBaseInput(std::string id, aa55_inverter::INPUT_TYPE type,
                                             AA55Inverter *parent_inverter, bool offline_hold) {
  this->id_ = id;
  this->type_ = type;
  this->parent_inverter_ = parent_inverter;
  this->offline_hold_ = offline_hold;
  this->response_function_code_ = aa55_inverter::MAP_INPUT_RESPONSE.at(type);
}

aa55_inverter::INPUT_TYPE AA55InverterBaseInput::get_type() { return this->type_; }

std::string AA55InverterBaseInput::get_id() { return this->id_; }

aa55_bus::FUNCTION_CODE AA55InverterBaseInput::get_response_function_code() { return this->response_function_code_; }
}  // namespace aa55_inverter
}  // namespace esphome
