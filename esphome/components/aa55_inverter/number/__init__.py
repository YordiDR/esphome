import esphome.codegen as cg
from esphome.components import number
import esphome.config_validation as cv
from esphome.const import (
    CONF_ID,
    CONF_MAX_VALUE,
    CONF_MIN_VALUE,
    CONF_NAME,
    CONF_STEP,
    DEVICE_CLASS_POWER_FACTOR,
    UNIT_PERCENT,
)

from .. import CONF_INVERTER_ID, INVERTER_CHILD_SCHEMA, aa55_const_ns, aa55_inverter_ns

DEPENDENCIES = ["aa55_inverter"]

CONF_ADJUST_POWER = "adjust_power"

AA55InverterNumber = aa55_inverter_ns.class_(
    "AA55InverterNumber", number.Number, cg.Component
)

InputType = aa55_const_ns.enum("INPUT_TYPE", is_class=True)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.Optional(
                CONF_ADJUST_POWER,
                default={CONF_ID: "adjust_power", CONF_NAME: "Adjust power"},
            ): number.number_schema(
                class_=AA55InverterNumber,
                device_class=DEVICE_CLASS_POWER_FACTOR,
                unit_of_measurement=UNIT_PERCENT,
            )
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
        if id and id.type == number.Number:
            if key == CONF_ADJUST_POWER:
                num = await number.new_number(conf, min_value=0, max_value=100, step=1)
            else:
                num = await number.new_number(
                    conf,
                    min_value=conf[CONF_MIN_VALUE],
                    max_value=conf[CONF_MAX_VALUE],
                    step=conf[CONF_STEP],
                )
            cg.add(num.set_id(key))
            cg.add(num.set_type(getattr(InputType, key.upper())))
            cg.add(num.set_parent_inverter(inverter))
            cg.add(inverter.add_input(num))
