import esphome.codegen as cg
from esphome.components import text_sensor
import esphome.config_validation as cv
from esphome.const import CONF_ID

from .. import BATTERY_CHILD_SCHEMA, CONF_PYLONTECH_BATTERY_ID, pylontech_battery_ns

DEPENDENCIES = ["pylontech_battery"]

CONF_STATE = "state"
CONF_VOLTAGE_STATE = "voltage_state"
CONF_CURRENT_STATE = "current_state"
CONF_TEMPERATURE_STATE = "temperature_state"
CONF_SKIP_UPDATES = "skip_updates"

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


# The key (upper-cased) must match an entry of SENSOR_TYPE in const.h
TEXT_SENSORS = [
    CONF_STATE,
    CONF_VOLTAGE_STATE,
    CONF_CURRENT_STATE,
    CONF_TEMPERATURE_STATE,
]

CONFIG_SCHEMA = (
    cv.Schema({cv.Optional(key): _text_sensor_schema() for key in TEXT_SENSORS})
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
            conf[CONF_SKIP_UPDATES],
        )
        await cg.register_component(var, conf)
        await text_sensor.register_text_sensor(var, conf)
        cg.add(battery.add_sensor(var))
