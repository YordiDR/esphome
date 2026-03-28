import esphome.codegen as cg
from esphome.components import text_sensor
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_NAME

from .. import CONF_GOODWE_AA55_ID, HUB_CHILD_SCHEMA, goodwe_aa55_ns

DEPENDENCIES = ["goodwe_aa55"]

CONF_WORK_MODE = "work_mode"
CONF_ERROR_CODES = "error_codes"
CONF_FIRMWARE_VERSION = "firmware_version"
CONF_MODEL = "model"
CONF_SERIAL_NUMBER = "serial_number"
CONF_INTERNAL_VERSION = "internal_version"
CONF_SKIP_UPDATES = "skip_updates"

GoodweAA55TextSensor = goodwe_aa55_ns.class_(
    "GoodweAA55TextSensor", text_sensor.TextSensor, cg.Component
)
SensorType = goodwe_aa55_ns.enum("SENSOR_TYPE", is_class=True)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.Optional(
                CONF_WORK_MODE, default={CONF_ID: "work_mode", CONF_NAME: "Work mode"}
            ): text_sensor.text_sensor_schema(class_=GoodweAA55TextSensor).extend(
                {cv.Optional(CONF_SKIP_UPDATES, default=0): cv.positive_int}
            ),
            cv.Optional(
                CONF_ERROR_CODES,
                default={CONF_ID: "error_codes", CONF_NAME: "Error codes"},
            ): text_sensor.text_sensor_schema(class_=GoodweAA55TextSensor).extend(
                {cv.Optional(CONF_SKIP_UPDATES, default=0): cv.positive_int}
            ),
            cv.Optional(
                CONF_FIRMWARE_VERSION,
                default={CONF_ID: "firmware_version", CONF_NAME: "Firmware version"},
            ): text_sensor.text_sensor_schema(class_=GoodweAA55TextSensor).extend(
                {cv.Optional(CONF_SKIP_UPDATES, default=720): cv.positive_int}
            ),
            cv.Optional(
                CONF_MODEL, default={CONF_ID: "model", CONF_NAME: "Model"}
            ): text_sensor.text_sensor_schema(class_=GoodweAA55TextSensor).extend(
                {cv.Optional(CONF_SKIP_UPDATES, default=720): cv.positive_int}
            ),
            cv.Optional(
                CONF_SERIAL_NUMBER,
                default={CONF_ID: "serial_number", CONF_NAME: "Serial number"},
            ): text_sensor.text_sensor_schema(class_=GoodweAA55TextSensor).extend(
                {cv.Optional(CONF_SKIP_UPDATES, default=720): cv.positive_int}
            ),
            cv.Optional(
                CONF_INTERNAL_VERSION,
                default={CONF_ID: "internal_version", CONF_NAME: "Internal version"},
            ): text_sensor.text_sensor_schema(class_=GoodweAA55TextSensor).extend(
                {cv.Optional(CONF_SKIP_UPDATES, default=720): cv.positive_int}
            ),
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
            cg.add(sens.set_skip_updates(conf[CONF_SKIP_UPDATES]))
            cg.add(sens.set_id(key))
            cg.add(sens.set_type(getattr(SensorType, key.upper())))
            cg.add(hub.add_text_sensor(sens))
