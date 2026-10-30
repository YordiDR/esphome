import esphome.codegen as cg
from esphome.components import text_sensor
import esphome.config_validation as cv
from esphome.const import CONF_ID, ENTITY_CATEGORY_DIAGNOSTIC

from .. import BATTERY_CHILD_SCHEMA, CONF_PYLONTECH_BATTERY_ID, pylontech_battery_ns

DEPENDENCIES = ["pylontech_battery"]

CONF_STATE = "state"
CONF_VOLTAGE_STATE = "voltage_state"
CONF_CURRENT_STATE = "current_state"
CONF_TEMPERATURE_STATE = "temperature_state"
CONF_SKIP_UPDATES = "skip_updates"

# Static device information (from the `info` console command)
CONF_DEVICE_NAME = "device_name"
CONF_BOARD_VERSION = "board_version"
CONF_BOARD = "board"
CONF_MAIN_SOFTWARE_VERSION = "main_software_version"
CONF_SOFTWARE_VERSION = "software_version"
CONF_BOOT_VERSION = "boot_version"
CONF_COMM_VERSION = "comm_version"
CONF_SERIAL_NUMBER = "serial_number"

PylontechBatteryTextSensor = pylontech_battery_ns.class_(
    "PylontechBatteryTextSensor", text_sensor.TextSensor, cg.Component
)
SensorType = pylontech_battery_ns.enum("SENSOR_TYPE", is_class=True)


def _text_sensor_schema():
    return text_sensor.text_sensor_schema(class_=PylontechBatteryTextSensor).extend(
        {
            cv.Optional(CONF_SKIP_UPDATES, default=0): cv.positive_int,
        }
    )


def _info_sensor_schema():
    # Device information never changes: it is requested once, so there is no skip_updates option.
    return text_sensor.text_sensor_schema(
        class_=PylontechBatteryTextSensor,
        entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
    )


# The key (upper-cased) must match an entry of SENSOR_TYPE in const.h
TEXT_SENSORS = {
    CONF_STATE: _text_sensor_schema,
    CONF_VOLTAGE_STATE: _text_sensor_schema,
    CONF_CURRENT_STATE: _text_sensor_schema,
    CONF_TEMPERATURE_STATE: _text_sensor_schema,
    CONF_DEVICE_NAME: _info_sensor_schema,
    CONF_BOARD_VERSION: _info_sensor_schema,
    CONF_BOARD: _info_sensor_schema,
    CONF_MAIN_SOFTWARE_VERSION: _info_sensor_schema,
    CONF_SOFTWARE_VERSION: _info_sensor_schema,
    CONF_BOOT_VERSION: _info_sensor_schema,
    CONF_COMM_VERSION: _info_sensor_schema,
    CONF_SERIAL_NUMBER: _info_sensor_schema,
}

CONFIG_SCHEMA = (
    cv.Schema({cv.Optional(key): make() for key, make in TEXT_SENSORS.items()})
    .extend(BATTERY_CHILD_SCHEMA)
    .extend(cv.COMPONENT_SCHEMA)
)


async def to_code(config):
    battery = await cg.get_variable(config[CONF_PYLONTECH_BATTERY_ID])

    for key in TEXT_SENSORS:
        if key not in config:
            continue
        conf = config[key]
        var = cg.new_Pvariable(
            conf[CONF_ID],
            key,
            getattr(SensorType, key.upper()),
            conf.get(CONF_SKIP_UPDATES, 0),
        )
        await cg.register_component(var, conf)
        await text_sensor.register_text_sensor(var, conf)
        cg.add(battery.add_sensor(var))
