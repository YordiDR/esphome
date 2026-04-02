import esphome.codegen as cg
from esphome.components import switch
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_NAME

from .. import CONF_INVERTER_ID, INVERTER_CHILD_SCHEMA, aa55_const_ns, aa55_inverter_ns

DEPENDENCIES = ["aa55_inverter"]

CONF_START_STOP = "start_stop"

AA55InverterSwitch = aa55_inverter_ns.class_(
    "AA55InverterSwitch", switch.Switch, cg.Component
)

InputType = aa55_const_ns.enum("INPUT_TYPE", is_class=True)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.Optional(
                CONF_START_STOP,
                default={CONF_ID: "start_stop", CONF_NAME: "Start-Stop"},
            ): switch.switch_schema(class_=AA55InverterSwitch)
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
        if id and id.type == switch.Switch:
            sw = await switch.new_switch(conf)
            cg.add(sw.set_id(key))
            cg.add(sw.set_type(getattr(InputType, key.upper())))
            cg.add(sw.set_parent_inverter(inverter))
            cg.add(inverter.add_input(sw))
