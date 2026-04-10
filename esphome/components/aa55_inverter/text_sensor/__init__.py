import esphome.codegen as cg
from esphome.components import text_sensor
import esphome.config_validation as cv
from esphome.const import CONF_ID

from .. import (
    CONF_INVERTER_ID,
    CONF_OFFLINE_HOLD,
    CONF_OFFLINE_VALUE,
    INVERTER_CHILD_SCHEMA,
    aa55_const_ns,
    aa55_inverter_ns,
)

DEPENDENCIES = ["aa55_inverter"]

CONF_WORK_MODE = "work_mode"
CONF_ERROR_CODES = "error_codes"
CONF_FIRMWARE_VERSION = "firmware_version"
CONF_MODEL = "model"
CONF_SERIAL_NUMBER = "serial_number"
CONF_INTERNAL_VERSION = "internal_version"
CONF_SKIP_UPDATES = "skip_updates"

AA55InverterTextSensor = aa55_inverter_ns.class_(
    "AA55InverterTextSensor", text_sensor.TextSensor, cg.Component
)
SensorType = aa55_const_ns.enum("SENSOR_TYPE", is_class=True)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.Optional(
                CONF_WORK_MODE,
            ): text_sensor.text_sensor_schema(class_=AA55InverterTextSensor).extend(
                {
                    cv.Optional(CONF_SKIP_UPDATES, default=0): cv.positive_int,
                    cv.Optional(CONF_OFFLINE_HOLD, default=False): cv.boolean,
                    cv.Optional(CONF_OFFLINE_VALUE, default="Offline"): cv.string,
                },
            ),
            cv.Optional(
                CONF_ERROR_CODES,
            ): text_sensor.text_sensor_schema(class_=AA55InverterTextSensor).extend(
                {
                    cv.Optional(CONF_SKIP_UPDATES, default=0): cv.positive_int,
                    cv.Optional(CONF_OFFLINE_HOLD, default=False): cv.boolean,
                    cv.Optional(CONF_OFFLINE_VALUE, default=""): cv.string,
                }
            ),
            cv.Optional(
                CONF_MODEL,
            ): text_sensor.text_sensor_schema(class_=AA55InverterTextSensor).extend(
                {
                    cv.Optional(CONF_OFFLINE_HOLD, default=True): cv.boolean,
                    cv.Optional(CONF_OFFLINE_VALUE, default=""): cv.string,
                }
            ),
            cv.Optional(
                CONF_SERIAL_NUMBER,
            ): text_sensor.text_sensor_schema(class_=AA55InverterTextSensor).extend(
                {
                    cv.Optional(CONF_OFFLINE_HOLD, default=True): cv.boolean,
                    cv.Optional(CONF_OFFLINE_VALUE, default=""): cv.string,
                }
            ),
        }
    )
    .extend(INVERTER_CHILD_SCHEMA)
    .extend(cv.COMPONENT_SCHEMA)
)


async def to_code(config):
    inverter = await cg.get_variable(config[CONF_INVERTER_ID])

    for key, conf in config.items():
        if not isinstance(conf, dict):
            continue
        id = conf[CONF_ID]
        if id and id.type == text_sensor.TextSensor:
            skip_updates = conf.get(CONF_SKIP_UPDATES, 0)
            var = cg.new_Pvariable(
                id,
                key,
                getattr(SensorType, key.upper()),
                skip_updates,
                conf.get(CONF_OFFLINE_HOLD, False),
                conf.get(CONF_OFFLINE_VALUE, ""),
            )
            await cg.register_component(var, conf)
            await text_sensor.register_text_sensor(var, conf)
            cg.add(inverter.add_sensor(var))
