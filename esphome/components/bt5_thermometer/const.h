#pragma once

namespace esphome {
namespace bt5_thermometer {
static constexpr uint16_t INVALID_RAW_TEMP = 0xFFFF;  // uint16 max value means that no probe is connected
static constexpr uint32_t STALE_TIMEOUT_MS =
    10000;  // Normally we recieve a GATT event every 3s. If we miss 3, we will assume all probes are disconnected
            // causing the device to go silent
}  // namespace bt5_thermometer
}  // namespace esphome