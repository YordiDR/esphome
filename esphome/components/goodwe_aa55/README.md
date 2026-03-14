# Example configuration
uart:
  - id: uart_aa55
    tx_pin: GPIO01
    rx_pin: GPIO02
    baud_rate: 9600
    data_bits: 8
    parity: NONE
    stop_bits: 1

goodwe_aa55:
  - id: aa55_inverter01
    uart_id: uart_aa55
    serial_number: ABCDEFGHIJKLMNOP
    slave_address: 0x01
    master_address: 0xFF
    update_interval: 5s

sensor:
  - platform: goodwe_aa55
    goodwe_aa55_id: aa55_inverter01
    pac:
      name: "Inverter01 AC current (Pac)"
      id: inverter01_pac
      skip_updates: 11
    vpv1:
      name: "Inverter01 PV1 Voltage (Vpv1)"
      id: inverter01_vpv1
      skip_updates: 11

text_sensor:
  - platform: goodwe_aa55
    goodwe_aa55_id: aa55_adrinv02
    work_mode:
      name: "ADRINV02 work mode"
      id: adrinv02_work_mode
