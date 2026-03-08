#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace goodwe_aa55 {

static const uint8_t MAX_LINE_LENGTH = 150;  // Max characters for serial buffer, 150 bytes is the length of the
                                             // response to the "read running info list" command

class GoodweAA55 : public uart::UARTDevice, public PollingComponent {
 public:
  GoodweAA55(std::string serial_number, uint8_t slave_address, uint8_t master_address, uint32_t update_interval);
  void setup() override;
  void dump_config() override;
  void loop() override;
  void set_vpv1_sensor(sensor::Sensor *vpv1_sensor) { vpv1_sensor_ = vpv1_sensor; }
  void set_vpv2_sensor(sensor::Sensor *vpv2_sensor) { vpv2_sensor_ = vpv2_sensor; }
  void set_ipv1_sensor(sensor::Sensor *ipv1_sensor) { ipv1_sensor_ = ipv1_sensor; }
  void set_ipv2_sensor(sensor::Sensor *ipv2_sensor) { ipv2_sensor_ = ipv2_sensor; }
  void set_vac1_sensor(sensor::Sensor *vac1_sensor) { vac1_sensor_ = vac1_sensor; }
  void set_iac1_sensor(sensor::Sensor *iac1_sensor) { iac1_sensor_ = iac1_sensor; }
  void set_fac1_sensor(sensor::Sensor *fac1_sensor) { fac1_sensor_ = fac1_sensor; }
  void set_pac_sensor(sensor::Sensor *pac_sensor) { pac_sensor_ = pac_sensor; }
  void set_temperature_sensor(sensor::Sensor *temperature_sensor) { temperature_sensor_ = temperature_sensor; }
  void set_e_total_sensor(sensor::Sensor *e_total_sensor) { e_total_sensor_ = e_total_sensor; }
  void set_h_total_sensor(sensor::Sensor *h_total_sensor) { h_total_sensor_ = h_total_sensor; }
  void set_gfci_fault_value_sensor(sensor::Sensor *gfci_fault_value_sensor) {
    gfci_fault_value_sensor_ = gfci_fault_value_sensor;
  }
  void set_e_today_sensor(sensor::Sensor *e_today_sensor) { e_today_sensor_ = e_today_sensor; }

  void update() override {
    this->vpv1_sensor_->publish_state(vpv1_);
    this->vpv2_sensor_->publish_state(vpv2_);
    this->ipv1_sensor_->publish_state(ipv1_);
    this->ipv2_sensor_->publish_state(ipv2_);
    this->vac1_sensor_->publish_state(vac1_);
    this->iac1_sensor_->publish_state(iac1_);
    this->fac1_sensor_->publish_state(fac1_);
    this->pac_sensor_->publish_state(pac_);
    this->temperature_sensor_->publish_state(temperature_);
    this->e_total_sensor_->publish_state(e_total_);
    this->h_total_sensor_->publish_state(h_total_);
    this->gfci_fault_value_sensor_->publish_state(gfci_fault_value_);
    this->e_today_sensor_->publish_state(e_today_);
  }

 protected:
  // Internal variables
  std::string serial_number_;
  uint8_t slave_address_;
  uint8_t master_address_;
  uint8_t update_interval_;
  uint32_t loop_counter_ = 0;
  std::vector<uint8_t> receive_buffer_;
  bool inverter_registered_ = false;
  float vpv1_ = 0.0;               // PV string 1 Voltage
  float vpv2_ = 0.0;               // PV string 2 Voltage
  float ipv1_ = 0.0;               // PV string 1 current
  float ipv2_ = 0.0;               // PV string 2 current
  float vac1_ = 0.0;               // Phase 1 voltage
  float iac1_ = 0.0;               // Phase 1 current
  float fac1_ = 0.0;               // Phase 1 frequency
  uint16_t pac_ = 0;               // AC power output
  float temperature_ = 0.0;        // Inverter temperature
  uint32_t e_total_ = 0;           // Total generated energy
  uint32_t h_total_ = 0;           // Total inverter runtime
  uint16_t gfci_fault_value_ = 0;  // GFCI fault value
  float e_today_ = 0.0;            // Energy generated today

  // Sensors
  sensor::Sensor *vpv1_sensor_{nullptr};
  sensor::Sensor *vpv2_sensor_{nullptr};
  sensor::Sensor *ipv1_sensor_{nullptr};
  sensor::Sensor *ipv2_sensor_{nullptr};
  sensor::Sensor *vac1_sensor_{nullptr};
  sensor::Sensor *iac1_sensor_{nullptr};
  sensor::Sensor *fac1_sensor_{nullptr};
  sensor::Sensor *pac_sensor_{nullptr};
  sensor::Sensor *temperature_sensor_{nullptr};
  sensor::Sensor *e_total_sensor_{nullptr};
  sensor::Sensor *h_total_sensor_{nullptr};
  sensor::Sensor *gfci_fault_value_sensor_{nullptr};
  sensor::Sensor *e_today_sensor_{nullptr};

  // Functions
  void parse_data();  // A method to parse the data read from the sensor hardware
  std::vector<uint8_t> calculate_checksum(
      std::vector<uint8_t> message);                   // Method that calculates the CRC checksum for an AA55 message
  bool verify_checksum(std::vector<uint8_t> message);  // Method that verifies the AA55 CRC checksum
  std::string create_hex_string(std::vector<uint8_t> data);  // Method that converts an array of bytes to a hex string
  float parse_int(std::vector<uint8_t> message, uint8_t start, uint8_t bytes, uint8_t precision = 0);
};

}  // namespace goodwe_aa55
}  // namespace esphome
