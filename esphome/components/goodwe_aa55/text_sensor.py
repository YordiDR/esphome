import esphome.codegen as cg
from esphome.components import text_sensor
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_NAME

from . import CONF_GOODWE_AA55_ID, HUB_CHILD_SCHEMA

DEPENDENCIES = ["goodwe_aa55"]

CONF_WORK_MODE = "work_mode"
CONF_ERROR_CODES = "error_codes"

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.Optional(
                CONF_WORK_MODE, default={CONF_ID: "work_mode", CONF_NAME: "Work mode"}
            ): text_sensor.text_sensor_schema(),
            cv.Optional(
                CONF_ERROR_CODES,
                default={CONF_ID: "error_codes", CONF_NAME: "Error codes"},
            ): text_sensor.text_sensor_schema(),
        }
    )
    .extend(HUB_CHILD_SCHEMA)
    .extend(cv.COMPONENT_SCHEMA)
)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_GOODWE_AA55_ID])

    for key, conf in config.items():
        if not isinstance(conf, dict):
            continue
        id = conf[CONF_ID]
        if id and id.type == text_sensor.TextSensor:
            sens = await text_sensor.new_text_sensor(conf)
            cg.add(getattr(hub, f"set_{key}_sensor")(sens))
