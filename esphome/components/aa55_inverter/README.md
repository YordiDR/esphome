# Configuration notes
- only sensors/inputs which have at least an ID defined will be processed
- skip_updates can be used to alter the state publish interval, it is not applicable on sensors read from the AA55 "ID info" command (= model, serial number & country code) as these normally do not change when an inverter is online. skip_updates is an integer value which describes the amount of update intervals to skip before publishing a new value. The update interval is specified on the inverter. Example: "inverter update_interval: 5s, sensor skip_updates 1" means the sensor will skip 1 interval = update every 10s
- aa55_bus master_address is a hexadecimal value, you can choose this freely. The only restriction from the AA55 protocol is that the first bit must be 1 (= range 0x80 - 0xFF)
- aa55_inverter slave_address is a hexadecimal value, you can choose this freely. The AA55 protocol dictates that the first bit must be 0 (= range 0x00 - 0x7E). 0x7F is used as a broadcast address, you should avoid it. This address will be assigned to the inverter over AA55, it does not need to be configured in the inverter manually.

# Example configuration
```yaml
uart:
  - id: uart_aa55_01
    tx_pin: GPIO01
    rx_pin: GPIO02
    baud_rate: 9600
    data_bits: 8
    parity: NONE
    stop_bits: 1

aa55_bus:
  - id: aa55_bus01
    uart_id: uart_aa55_01
    master_address: 0xFF

aa55_inverter:
  - id: aa55_inverter01
    aa55_bus_id: aa55_bus01
    slave_address: 0x01
    serial_number: ABC
    update_interval: 5s
    setup_priority: -15

sensor:
  - platform: aa55_inverter
    aa55_inverter_id: aa55_inverter01
    vpv1:
      name: "Inverter 1 PV1 Voltage (Vpv1)"
      id: aa55_inverter01_vpv1
      skip_updates: 11
    ipv1:
      name: "Inverter 1 PV1 Current (Ipv1)"
      id: aa55_inverter01_ipv1
      skip_updates: 11
    vpv2:
      name: "Inverter 1 PV2 Voltage (Vpv2)"
      id: aa55_inverter01_vpv2
      skip_updates: 11
    ipv2:
      name: "Inverter 1 PV2 Current (Ipv2)"
      id: aa55_inverter01_ipv2
      skip_updates: 11
    vac1:
      name: "Inverter 1 Phase 1 Voltage (Vac1)"
      id: aa55_inverter01_vac1
      skip_updates: 11
    iac1:
      name: "Inverter 1 Phase 1 Current (Iac1)"
      id: aa55_inverter01_iac1
      skip_updates: 11
    fac1:
      name: "Inverter 1 Phase 1 Frequency (Fac1)"
      id: aa55_inverter01_fac1
      skip_updates: 11
    pac:
      name: "Inverter 1 AC Power (Pac)"
      id: aa55_inverter01_pac
    temperature:
      name: "Inverter 1 Temperature"
      id: aa55_inverter01_temperature
      skip_updates: 11
    e_total:
      name: "Inverter 1 Total Generated Energy (E-total)"
      id: aa55_inverter01_e_total
      skip_updates: 11
    h_total:
      name: "Inverter 1 Total runtime (H-total)"
      id: aa55_inverter01_h_total
      skip_updates: 11
    gfci_fault_value:
      name: "Inverter 1 GFCI Fault Value"
      id: aa55_inverter01_gfci_fault_value
      skip_updates: 719
    e_today:
      name: "Inverter 1 Generated Energy today (E-today)"
      id: aa55_inverter01_e_today
      skip_updates: 11
    country_code:
      name: "Inverter 1 Country code"
      id: aa55_inverter01_country_code

text_sensor:
  - platform: aa55_inverter
    aa55_inverter_id: aa55_inverter01
    work_mode:
      name: "Inverter 1 Work mode"
      id: aa55_inverter01_work_mode
    error_codes:
      name: "Inverter 1 Error codes"
      id: aa55_inverter01_error_codes
    model:
      name: "Inverter 1 model"
      id: aa55_inverter01_model
    serial_number:
      name: "Inverter 1 serial number"
      id: aa55_inverter01_serial_number

switch:
  - platform: aa55_inverter
    aa55_inverter_id: aa55_inverter01
    start_stop:
      name: "Inverter 1 Start-Stop"
      id: aa55_inverter01_start_stop

number:
  - platform: aa55_inverter
    aa55_inverter_id: aa55_inverter01
    adjust_power:
      name: "Inverter 1 power adjustment"
      id: aa55_inverter01_adjust_power

button:
  - platform: aa55_inverter
    aa55_inverter_id: aa55_inverter01
    reconnect_grid:
      name: "Inverter 1 reconnect grid"
      id: aa55_inverter01_reconnect_grid
```

# AA55 protocol documentation
An unofficial protocol documentation file can be found here: https://yamasun.com.tw/upload/F_20170313191367UrC8jo.PDF
