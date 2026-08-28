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

CONF_NAME_PREFIX = "name_prefix"
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
            cv.GenerateID(): cv.declare_id(BT5Thermometer),
            cv.Optional(CONF_NAME_PREFIX, default="BT5"): cv.string,
        }
    )
    .extend(ble_client.BLE_CLIENT_SCHEMA)
    .extend(cv.COMPONENT_SCHEMA)
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await ble_client.register_ble_node(var, config)

    # Generate the connection status binary sensor
    conn_sens_id = ID(
        f"{config[CONF_NAME_PREFIX]}_connected",
        is_declaration=True,
        type=binary_sensor.BinarySensor,
    )
    conn_sens = cg.new_Pvariable(conn_sens_id)
    conn_sens_config = binary_sensor.binary_sensor_schema(
        device_class=DEVICE_CLASS_CONNECTIVITY
    )(
        {
            CONF_ID: conn_sens_id,
            CONF_NAME: f"{config[CONF_NAME_PREFIX]} Connected",
        }
    )
    await binary_sensor.register_binary_sensor(conn_sens, conn_sens_config)
    cg.add(var.set_connection_sensor(conn_sens))

    # Generate the probe sensors
    for i in range(1, 7):
        sens_id = ID(
            f"{config[CONF_NAME_PREFIX]}_probe_{i}",
            is_declaration=True,
            type=BT5ProbeSensor,
        )
        # Create the C++ object and pass 'i' directly to the constructor: BBQProbeSensor(i)
        sens = cg.new_Pvariable(sens_id, i)

        # Register standard sensor configuration properties
        sens_config = sensor.sensor_schema(
            unit_of_measurement=UNIT_CELSIUS,
            accuracy_decimals=0,
            device_class=DEVICE_CLASS_TEMPERATURE,
            state_class=STATE_CLASS_MEASUREMENT,
        )(
            {
                CONF_ID: sens_id,
                CONF_NAME: f"{config[CONF_NAME_PREFIX]} Probe {i}",
            }
        )
        await sensor.register_sensor(sens, sens_config)

        cg.add(var.register_probe(sens))
