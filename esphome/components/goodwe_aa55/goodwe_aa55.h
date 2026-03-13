#pragma once

#include "esphome/core/component.h"
#include "esphome/core/defines.h"
#include "esphome/components/uart/uart.h"
#ifdef USE_SENSOR
#include "esphome/components/sensor/sensor.h"
#endif
#ifdef USE_TEXT_SENSOR
#include "esphome/components/text_sensor/text_sensor.h"
#endif
#include <string>

#ifndef GOODWE_AA55_SENSOR_LIST
#define GOODWE_AA55_SENSOR_LIST(F, SEP)
#endif

#ifndef GOODWE_AA55_TEXT_SENSOR_LIST
#define GOODWE_AA55_TEXT_SENSOR_LIST(F, SEP)
#endif

namespace esphome {
namespace goodwe_aa55 {

static const uint8_t MAX_LINE_LENGTH = 150;  // Max characters for serial buffer, 150 bytes is the length of the
                                             // response to the "read running info list" command
static const uint8_t INVERTER_OFFLINE_COUNTDOWN_RESET = 5;

class GoodweAA55 : public uart::UARTDevice, public PollingComponent {
 public:
  GoodweAA55(std::string serial_number, uint8_t slave_address, uint8_t master_address, uint32_t update_interval);
  void setup() override;
  void dump_config() override;
  void loop() override;

// Sensor setters
#define GOODWE_AA55_SET_SENSOR(s) \
  void set_##s(sensor::Sensor *sensor) { s_##s##_ = sensor; }
  GOODWE_AA55_SENSOR_LIST(GOODWE_AA55_SET_SENSOR, )

#define GOODWE_AA55_SET_TEXT_SENSOR(s) \
  void set_##s(text_sensor::TextSensor *sensor) { s_##s##_ = sensor; }
  GOODWE_AA55_TEXT_SENSOR_LIST(GOODWE_AA55_SET_TEXT_SENSOR, )

  void update() override {
    if (inverter_online_) {
#define GOODWE_AA55_PUBLISH_SENSOR_STATE(s) this->s_##s##_->publish_state(v_##s##_);
      GOODWE_AA55_SENSOR_LIST(GOODWE_AA55_PUBLISH_SENSOR_STATE, )
#define GOODWE_AA55_PUBLISH_TEXT_SENSOR_STATE(s) this->s_##s##_->publish_state(v_##s##_);
      GOODWE_AA55_TEXT_SENSOR_LIST(GOODWE_AA55_PUBLISH_TEXT_SENSOR_STATE, )
    } else {
#define GOODWE_AA55_SET_SENSOR_UNAVAILABLE(s) this->s_##s##_->publish_state(NAN);
      GOODWE_AA55_SENSOR_LIST(GOODWE_AA55_SET_SENSOR_UNAVAILABLE, )
      this->s_work_mode_->publish_state("Offline");
      this->s_error_codes_->publish_state("");
    }
  }

 protected:
  // Internal variables
  std::string serial_number_;
  uint8_t slave_address_;
  uint8_t master_address_;
  uint8_t update_interval_;
  uint32_t loop_counter_ = 0;
  std::vector<uint8_t> receive_buffer_;
  bool inverter_online_ = false;
  uint8_t inverter_offline_countdown_ = INVERTER_OFFLINE_COUNTDOWN_RESET;
  float v_vpv1_;                 // PV string 1 Voltage
  float v_vpv2_;                 // PV string 2 Voltage
  float v_ipv1_;                 // PV string 1 current
  float v_ipv2_;                 // PV string 2 current
  float v_vac1_;                 // Phase 1 voltage
  float v_iac1_;                 // Phase 1 current
  float v_fac1_;                 // Phase 1 frequency
  uint16_t v_pac_;               // AC power output
  std::string v_work_mode_;      // Inverter work mode (stringified)
  uint16_t v_work_mode_code_;    // Inverter work mode (Numeric, as received from inverter)
  float v_temperature_;          // Inverter temperature
  std::string v_error_codes_;    // Inverter error codes (stringified)
  uint32_t v_error_codes_code_;  // Inverter error codes (Numeric, as received from inverter)
  float v_e_total_;              // Total generated energy
  uint32_t v_h_total_;           // Total inverter runtime
  uint16_t v_gfci_fault_value_;  // GFCI fault value
  float v_e_today_;              // Energy generated today

// Sensor member pointers
#define GOODWE_AA55_DECLARE_SENSOR(s) sensor::Sensor *s_##s##_{nullptr};
  GOODWE_AA55_SENSOR_LIST(GOODWE_AA55_DECLARE_SENSOR, )

#define GOODWE_AA55_DECLARE_TEXT_SENSOR(s) text_sensor::TextSensor *s_##s##_{nullptr};
  GOODWE_AA55_TEXT_SENSOR_LIST(GOODWE_AA55_DECLARE_TEXT_SENSOR, )

  // Functions
  void parse_data();  // A method to parse the data read from the sensor hardware
  std::vector<uint8_t> calculate_checksum(
      std::vector<uint8_t> message);                   // Method that calculates the CRC checksum for an AA55 message
  bool verify_checksum(std::vector<uint8_t> message);  // Method that verifies the AA55 CRC checksum
  std::string create_hex_string(std::vector<uint8_t> data);  // Method that converts an array of bytes to a hex string
  float parse_int(std::vector<uint8_t> message, uint8_t start, uint8_t bytes, uint8_t precision);
};

}  // namespace goodwe_aa55
}  // namespace esphome
