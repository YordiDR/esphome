#pragma once

#include "esphome/core/component.h"
#include "esphome/core/defines.h"
#include "esphome/components/uart/uart.h"
#include "sensor/goodwe_aa55_sensor.h"
#include "text_sensor/goodwe_aa55_text_sensor.h"
#include <string>
#include <vector>
#include <deque>

namespace esphome {
namespace goodwe_aa55 {

class GoodweAA55 : public uart::UARTDevice, public PollingComponent {
 public:
  GoodweAA55(std::string serial_number, uint8_t slave_address, uint8_t master_address, uint32_t update_interval);
  void setup() override;
  void dump_config() override;
  void loop() override;
  void update() override;
  void add_sensor(GoodweAA55Sensor *sensor);
  void add_text_sensor(GoodweAA55TextSensor *sensor);

 protected:
  // Internal variables
  std::string serial_number_;
  uint8_t slave_address_, master_address_, inverter_offline_countdown_ = INVERTER_OFFLINE_COUNTDOWN_RESET;
  uint16_t update_interval_;
  uint32_t loop_counter_ = 0;
  std::vector<uint8_t> receive_buffer_;
  std::vector<GoodweAA55Sensor *> sensors_;
  std::vector<GoodweAA55TextSensor *> text_sensors_;
  bool inverter_online_ = false;

  // Functions
  void parse_data(const std::vector<uint8_t> &payload);  // A method to parse the data read from the sensor hardware
  uint32_t parse_int(const std::vector<uint8_t> &message, uint8_t start, uint8_t bytes);
  template<typename T> std::vector<uint8_t> calculate_checksum(const T &packet) {
    uint16_t crc = 0;
    ESP_LOGD(LOGGING_TAG, "Calculating CRC for packet '%s'...", this->create_hex_string(packet).c_str());
    for (uint8_t byte : packet) {
      ESP_LOGV(LOGGING_TAG, "Checksum calculation: adding value %x to current CRC value (%d)", byte, crc);
      crc += byte;
    }

    ESP_LOGD(LOGGING_TAG, "Calculated CRC value: %d, {%x, %x}", crc, (uint8_t) (crc >> 8), (uint8_t) crc);
    const std::vector<uint8_t> crc_bytes{(uint8_t) (crc >> 8), (uint8_t) crc};
    return crc_bytes;
  }
  template<typename T> std::string create_hex_string(const T &data) {
    std::string result;
    result.reserve(data.size() * 3);

    const char *hex = "0123456789ABCDEF";

    for (uint8_t byte : data) {
      result.push_back(hex[(byte >> 4) & 0xF]);
      result.push_back(hex[byte & 0xF]);
      result.push_back(' ');
    }

    if (!result.empty())
      result.pop_back();  // Remove trailing space
    return result;
  }
};

}  // namespace goodwe_aa55
}  // namespace esphome
