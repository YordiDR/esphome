# Configuration notes
- Sensors are only polled when they are configured. The battery works out which console commands (`pwr`, `bat`, `stat`) are needed from the sensors you define: `pwr` for voltage/current/power/SoC/temperature/states, `bat` for the cell voltage sensors, `stat` for SoH.
- `skip_updates` alters the publish interval of a single sensor. It is an integer: the number of battery update intervals to skip between two publishes. Example: "battery update_interval: 5s, sensor skip_updates: 1" publishes every 10s. A command is only sent to the battery on the update ticks where at least one of its sensors is due, so `skip_updates` on a slow-changing sensor such as SoH (`soh: skip_updates: 11` with a 5s interval = once a minute) also reduces serial traffic.
- If a command gets no valid answer (timeout or garbage), nothing is published and the sensors are retried on the next update.
- If a battery has not answered for 5 minutes (BATTERY_OFFLINE_TIMEOUT in const.h), its numeric sensors publish NaN (unavailable in Home Assistant) until it answers again.
- Current and power keep the console's sign: negative = discharging. Use a `multiply: -1` filter if you want the opposite.
- The cell voltage delta is in mV, the highest/lowest cell voltage in V.

# Example configuration
```yaml
pylontech_group:
  - id: pylontech_group01
    uart_id: pylontech_uart_rs232
    setup_priority: -10

pylontech_battery:
  - pylontech_group_id: pylontech_group01
    battery_number: 1
    id: "group01_battery01"
    update_interval: 5s
    setup_priority: -15
  - pylontech_group_id: pylontech_group01
    battery_number: 2
    id: "group01_battery02"
    update_interval: 5s
    setup_priority: -15

sensor:
  - platform: pylontech_battery
    pylontech_battery_id: group01_battery01
    voltage:
      name: Pylontech Battery 1 Voltage
      id: pylontech_group01_battery01_voltage
    current:
      name: Pylontech Battery 1 Current
      id: pylontech_group01_battery01_current
    power:
      name: Pylontech Battery 1 Power
      id: pylontech_group01_battery01_power
    soc:
      name: Pylontech Battery 1 SoC
      id: pylontech_group01_battery01_soc
    temperature:
      name: Pylontech Battery 1 Temperature
      id: pylontech_group01_battery01_temperature
    soh:
      name: Pylontech Battery 1 SoH
      id: pylontech_group01_battery01_soh
    highest_cell_voltage:
      name: Pylontech Battery 1 Highest Cell Voltage
      id: pylontech_group01_battery01_highest_cell_voltage
    lowest_cell_voltage:
      name: Pylontech Battery 1 Lowest Cell Voltage
      id: pylontech_group01_battery01_lowest_cell_voltage
    cell_voltage_delta:
      name: Pylontech Battery 1 Cell Voltage Delta
      id: pylontech_group01_battery01_cell_voltage_delta

  - platform: pylontech_battery
    pylontech_battery_id: group01_battery02
    voltage:
      name: Pylontech Battery 2 Voltage
      id: pylontech_group01_battery02_voltage

text_sensor:
  - platform: pylontech_battery
    pylontech_battery_id: group01_battery01
    state:
      name: Pylontech Battery 1 Status
      id: pylontech_group01_battery01_status
    voltage_state:
      name: Pylontech Battery 1 Voltage Status
      id: pylontech_group01_battery01_voltage_status
    current_state:
      name: Pylontech Battery 1 Current Status
      id: pylontech_group01_battery01_current_status
    temperature_state:
      name: Pylontech Battery 1 Temperature Status
      id: pylontech_group01_battery01_temperature_status