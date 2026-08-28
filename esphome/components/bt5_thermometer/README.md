# Configuration notes
- The id of the bt5_thermometer component must be configured since it's used to generate unique probe names.
- The update interval is 10s by default but it can optionally be configured. The device sends updates every 3 seconds when a probe is connected.

# Example configuration
```yaml
ble_client:
  - mac_address: AA:BB:CC:DD:EE:FF # Set the MAC address of your BT5 thermometer here
    id: bt5_bbq_thermometer
    auto_connect: true

bt5_thermometer:
  id: bt5
  ble_client_id: bt5_bbq_thermometer
  update_inverval: 30s # Optional
```