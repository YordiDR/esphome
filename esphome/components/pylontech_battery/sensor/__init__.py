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


def _sensor_schema(unit, accuracy_decimals, device_class):
    return sensor.sensor_schema(
        class_=PylontechBatterySensor,
        unit_of_measurement=unit,
        accuracy_decimals=accuracy_decimals,
        state_class=STATE_CLASS_MEASUREMENT,
        device_class=device_class,
    ).extend(
        {
            cv.Optional(CONF_SKIP_UPDATES, default=0): cv.positive_int,
        }
    )


# key -> schema. The key (upper-cased) must match an entry of SENSOR_TYPE in const.h
SENSORS = {
    CONF_VOLTAGE: _sensor_schema(UNIT_VOLT, 3, DEVICE_CLASS_VOLTAGE),
    CONF_CURRENT: _sensor_schema(UNIT_AMPERE, 3, DEVICE_CLASS_CURRENT),
    CONF_POWER: _sensor_schema(UNIT_WATT, 0, DEVICE_CLASS_POWER),
    CONF_SOC: _sensor_schema(UNIT_PERCENT, 0, DEVICE_CLASS_BATTERY),
    CONF_SOH: _sensor_schema(UNIT_PERCENT, 0, DEVICE_CLASS_BATTERY),
    CONF_TEMPERATURE: _sensor_schema(UNIT_CELSIUS, 1, DEVICE_CLASS_TEMPERATURE),
    CONF_HIGHEST_CELL_VOLTAGE: _sensor_schema(UNIT_VOLT, 3, DEVICE_CLASS_VOLTAGE),
    CONF_LOWEST_CELL_VOLTAGE: _sensor_schema(UNIT_VOLT, 3, DEVICE_CLASS_VOLTAGE),
    CONF_CELL_VOLTAGE_DELTA: _sensor_schema(UNIT_MILLIVOLT, 0, DEVICE_CLASS_VOLTAGE),
}

CONFIG_SCHEMA = (
    cv.Schema({cv.Optional(key): schema for key, schema in SENSORS.items()})
    .extend(BATTERY_CHILD_SCHEMA)
    .extend(cv.COMPONENT_SCHEMA)
)


async def to_code(config):
    battery = await cg.get_variable(config[CONF_PYLONTECH_BATTERY_ID])

    # Iterate over the known keys instead of inspecting id types, which is fragile with custom classes
    for key in SENSORS:
        if key not in config:
            continue
        conf = config[key]
        var = cg.new_Pvariable(
            conf[CONF_ID],
            key,
            getattr(SensorType, key.upper()),
            conf[CONF_SKIP_UPDATES],
        )
        await cg.register_component(var, conf)
        await sensor.register_sensor(var, conf)
        cg.add(battery.add_sensor(var))
