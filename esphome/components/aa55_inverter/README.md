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
    master_address: 0xFF # first bit should be one (see AA55 doc)

aa55_inverter:
  - id: aa55_inverter01
    aa55_bus_id: aa55_bus01
    slave_address: 0x01 # first bit should be zero (see AA55 doc)
    serial_number: ABC
    update_interval: 5s

sensor:
  - platform: aa55_inverter
    aa55_inverter_id: aa55_inverter01
    pac:
      name: "Inverter 1 AC current (Pac)"
      id: inverter01_pac
      skip_updates: 11
    vpv1:
      name: "Inverter 1 PV1 Voltage (Vpv1)"
      id: inverter01_vpv1
      skip_updates: 11

text_sensor:
  - platform: aa55_inverter
    aa55_inverter_id: aa55_inverter01
    work_mode:
      name: "Inverter 1 work mode"
      id: inverter01_work_mode
```

# AA55 protocol documentation
An unofficial protocol documentation file can be found here: https://yamasun.com.tw/upload/F_20170313191367UrC8jo.PDF
