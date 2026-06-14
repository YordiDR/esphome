# Configuration notes
- only sensors/inputs which have at least an ID defined will be processed
- skip_updates can be used to alter the state publish interval. skip_updates is an integer value which describes the amount of update intervals to skip before publishing a new value. The update interval is specified on the inverter. Example: "battery update_interval: 5s, sensor skip_updates 1" means the sensor will skip 1 interval = update every 10s

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
      id: pylontech_group01_batter0y1_lowest_cell_voltage
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