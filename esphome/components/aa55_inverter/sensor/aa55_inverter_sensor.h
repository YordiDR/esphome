#pragma once
#include <string>
#include "esphome/core/component.h"
#include "../../sensor/sensor.h"
#include "../aa55_inverter_base_sensor.h"

namespace esphome {
namespace aa55_inverter {

class AA55InverterSensor : public AA55InverterBaseSensor, public sensor::Sensor, public Component {
 public:
  AA55InverterSensor(std::string id, aa55_inverter::SENSOR_TYPE type, uint16_t skip_updates, bool offline_hold,
                     float offline_value);
  void process_response(const uint8_t *payload, uint8_t payload_length) override;
  void handle_inverter_offline() override;
  void dump_config() override;

 protected:
  float offline_value_{};
};

}  // namespace aa55_inverter
}  // namespace esphome
