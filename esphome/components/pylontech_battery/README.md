# Configuration notes
- only sensors/inputs which have at least an ID defined will be processed
- skip_updates can be used to alter the state publish interval. skip_updates is an integer value which describes the amount of update intervals to skip before publishing a new value. The update interval is specified on the inverter. Example: "battery update_interval: 5s, sensor skip_updates 1" means the sensor will skip 1 interval = update every 10s

# Example configuration
```yaml
pylontech_group:
  - id: pylontech_group01
    uart_id: pylontech_uart

pylontech_battery:
  - pylontech_group_id: pylontech_group01
    battery_number: 1
    name: "pylontech_group01_battery1"
    update_interval: 60s
    setup_priority: -15
  - pylontech_group_id: pylontech_group01
    battery_number: 2
    name: "pylontech_group01_battery2"
    update_interval: 60s
    setup_priority: -15

sensor:
  - platform: pylontech_battery
    battery_id: pylontech_group01_battery1
    name: "group01_battery1_voltage"
    unit_of_measurement: "V"
    accuracy_decimals: 3
    icon: "mdi:battery"
  - platform: pylontech_battery
    battery_id: pylontech_group01_battery2
    name: "group01_battery2_voltage"
    unit_of_measurement: "V"
    accuracy_decimals: 3
    icon: "mdi:battery"
```
