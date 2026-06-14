import esphome.codegen as cg
from esphome.components import sensor
import esphome.config_validation as cv
from esphome.const import (
    CONF_ID,
    CONF_TEMPERATURE,
    DEVICE_CLASS_BATTERY,
    DEVICE_CLASS_CURRENT,
    DEVICE_CLASS_POWER,
    DEVICE_CLASS_TEMPERATURE,
    DEVICE_CLASS_VOLTAGE,
    STATE_CLASS_MEASUREMENT,
    UNIT_AMPERE,
    UNIT_CELSIUS,
    UNIT_MILLIVOLT,
    UNIT_PERCENT,
    UNIT_VOLT,
    UNIT_WATT,
)

from .. import BATTERY_CHILD_SCHEMA, CONF_PYLONTECH_BATTERY_ID, pylontech_battery_ns

DEPENDENCIES = ["pylontech_battery"]

CONF_VOLTAGE = "voltage"
CONF_CURRENT = "current"
CONF_POWER = "power"
CONF_SOC = "soc"
CONF_SOH = "soh"
CONF_HIGHEST_CELL_VOLTAGE = "highest_cell_voltage"
CONF_LOWEST_CELL_VOLTAGE = "lowest_cell_voltage"
CONF_CELL_VOLTAGE_DELTA = "cell_voltage_delta"
CONF_SKIP_UPDATES = "skip_updates"

PylontechBatterySensor = pylontech_battery_ns.class_(
    "PylontechBatterySensor", sensor.Sensor, cg.Component
)
SensorType = pylontech_battery_ns.enum("SENSOR_TYPE", is_class=True)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.Optional(
                CONF_VOLTAGE,
            ): sensor.sensor_schema(
                class_=PylontechBatterySensor,
                unit_of_measurement=UNIT_VOLT,
                accuracy_decimals=3,
                state_class=STATE_CLASS_MEASUREMENT,
                device_class=DEVICE_CLASS_VOLTAGE,
            ).extend(
                {
                    cv.Optional(CONF_SKIP_UPDATES, default=0): cv.positive_int,
                }
            ),
            cv.Optional(
                CONF_CURRENT,
            ): sensor.sensor_schema(
                class_=PylontechBatterySensor,
                unit_of_measurement=UNIT_AMPERE,
                accuracy_decimals=3,
                state_class=STATE_CLASS_MEASUREMENT,
                device_class=DEVICE_CLASS_CURRENT,
            ).extend(
                {
                    cv.Optional(CONF_SKIP_UPDATES, default=0): cv.positive_int,
                }
            ),
            cv.Optional(
                CONF_POWER,
            ): sensor.sensor_schema(
                class_=PylontechBatterySensor,
                unit_of_measurement=UNIT_WATT,
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
                device_class=DEVICE_CLASS_POWER,
            ).extend(
                {
                    cv.Optional(CONF_SKIP_UPDATES, default=0): cv.positive_int,
                }
            ),
            cv.Optional(
                CONF_SOC,
            ): sensor.sensor_schema(
                class_=PylontechBatterySensor,
                unit_of_measurement=UNIT_PERCENT,
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
                device_class=DEVICE_CLASS_BATTERY,
            ).extend(
                {
                    cv.Optional(CONF_SKIP_UPDATES, default=0): cv.positive_int,
                }
            ),
            cv.Optional(
                CONF_SOH,
            ): sensor.sensor_schema(
                class_=PylontechBatterySensor,
                unit_of_measurement=UNIT_PERCENT,
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
                device_class=DEVICE_CLASS_BATTERY,
            ).extend(
                {
                    cv.Optional(CONF_SKIP_UPDATES, default=0): cv.positive_int,
                }
            ),
            cv.Optional(
                CONF_TEMPERATURE,
            ): sensor.sensor_schema(
                class_=PylontechBatterySensor,
                unit_of_measurement=UNIT_CELSIUS,
                accuracy_decimals=1,
                state_class=STATE_CLASS_MEASUREMENT,
                device_class=DEVICE_CLASS_TEMPERATURE,
            ).extend(
                {
                    cv.Optional(CONF_SKIP_UPDATES, default=0): cv.positive_int,
                }
            ),
            cv.Optional(
                CONF_HIGHEST_CELL_VOLTAGE,
            ): sensor.sensor_schema(
                class_=PylontechBatterySensor,
                unit_of_measurement=UNIT_VOLT,
                accuracy_decimals=3,
                state_class=STATE_CLASS_MEASUREMENT,
                device_class=DEVICE_CLASS_VOLTAGE,
            ).extend(
                {
                    cv.Optional(CONF_SKIP_UPDATES, default=0): cv.positive_int,
                }
            ),
            cv.Optional(
                CONF_LOWEST_CELL_VOLTAGE,
            ): sensor.sensor_schema(
                class_=PylontechBatterySensor,
                unit_of_measurement=UNIT_VOLT,
                accuracy_decimals=3,
                state_class=STATE_CLASS_MEASUREMENT,
                device_class=DEVICE_CLASS_VOLTAGE,
            ).extend(
                {
                    cv.Optional(CONF_SKIP_UPDATES, default=0): cv.positive_int,
                }
            ),
            cv.Optional(
                CONF_CELL_VOLTAGE_DELTA,
            ): sensor.sensor_schema(
                class_=PylontechBatterySensor,
                unit_of_measurement=UNIT_MILLIVOLT,
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
                device_class=DEVICE_CLASS_VOLTAGE,
            ).extend(
                {
                    cv.Optional(CONF_SKIP_UPDATES, default=0): cv.positive_int,
                }
            ),
        }
    )
    .extend(BATTERY_CHILD_SCHEMA)
    .extend(cv.COMPONENT_SCHEMA)
)


async def to_code(config):
    battery = await cg.get_variable(config[CONF_PYLONTECH_BATTERY_ID])

    for key, conf in config.items():
        if not isinstance(conf, dict):
            continue
        id = conf[CONF_ID]
        if id and id.type == sensor.Sensor:
            skip_updates = 0
            if CONF_SKIP_UPDATES in conf:
                skip_updates = conf[CONF_SKIP_UPDATES]

            var = cg.new_Pvariable(
                id, key, getattr(SensorType, key.upper()), skip_updates
            )
            await cg.register_component(var, conf)
            await sensor.register_sensor(var, conf)
            cg.add(battery.add_sensor(var))
