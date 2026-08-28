import esphome.codegen as cg
from esphome.components import binary_sensor, ble_client, sensor
import esphome.config_validation as cv
from esphome.const import (
    CONF_ID,
    CONF_NAME,
    DEVICE_CLASS_CONNECTIVITY,
    DEVICE_CLASS_TEMPERATURE,
    STATE_CLASS_MEASUREMENT,
    UNIT_CELSIUS,
)
from esphome.core import ID

DEPENDENCIES = ["ble_client"]
AUTO_LOAD = ["sensor", "binary_sensor"]

bt5_thermometer_ns = cg.esphome_ns.namespace("bt5_thermometer")
BT5Thermometer = bt5_thermometer_ns.class_(
    "BT5Thermometer", ble_client.BLEClientNode, cg.Component
)
BT5ProbeSensor = bt5_thermometer_ns.class_("BT5ProbeSensor", sensor.Sensor)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.Required(CONF_ID): cv.declare_id(BT5Thermometer),
        }
    )
    .extend(ble_client.BLE_CLIENT_SCHEMA)
    .extend(cv.COMPONENT_SCHEMA)
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID], config[CONF_ID].id)
    await cg.register_component(var, config)
    await ble_client.register_ble_node(var, config)

    # Extract the component ID string to use as the naming base (e.g., "my_thermometer")
    base_id = config[CONF_ID].id
    # Create a human-friendly title version for entity names (e.g., "My Thermometer")
    base_name = base_id.replace("_", " ").title()

    # Generate the connection status binary sensor
    conn_sens_id = ID(
        f"{base_id}_connected",
        is_declaration=True,
        type=binary_sensor.BinarySensor,
    )
    conn_sens = cg.new_Pvariable(conn_sens_id)
    conn_sens_config = binary_sensor.binary_sensor_schema(
        device_class=DEVICE_CLASS_CONNECTIVITY
    )(
        {
            CONF_ID: conn_sens_id,
            CONF_NAME: f"{base_name} Connected",
        }
    )
    await binary_sensor.register_binary_sensor(conn_sens, conn_sens_config)
    cg.add(var.set_connection_sensor(conn_sens))

    # Generate the probe sensors
    for i in range(1, 7):
        sens_id = ID(
            f"{base_id}_probe_{i}",
            is_declaration=True,
            type=BT5ProbeSensor,
        )
        sens = cg.new_Pvariable(sens_id, i)

        sens_config = sensor.sensor_schema(
            unit_of_measurement=UNIT_CELSIUS,
            accuracy_decimals=1,
            device_class=DEVICE_CLASS_TEMPERATURE,
            state_class=STATE_CLASS_MEASUREMENT,
        )(
            {
                CONF_ID: sens_id,
                CONF_NAME: f"{base_name} Probe {i}",
            }
        )
        await sensor.register_sensor(sens, sens_config)

        cg.add(var.register_probe(sens))
