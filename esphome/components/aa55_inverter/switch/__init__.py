import esphome.codegen as cg
from esphome.components import switch
import esphome.config_validation as cv
from esphome.const import CONF_ID

from .. import (
    CONF_INVERTER_ID,
    CONF_OFFLINE_HOLD,
    CONF_OFFLINE_VALUE,
    CONF_ONLINE_INTIAL_VALUE,
    INVERTER_CHILD_SCHEMA,
    aa55_const_ns,
    aa55_inverter_ns,
)

DEPENDENCIES = ["aa55_inverter"]

CONF_START_STOP = "start_stop"

ON_OFF_OPTIONS = {
    "OFF": 0,
    "ON": 1,
}

AA55InverterSwitch = aa55_inverter_ns.class_(
    "AA55InverterSwitch", switch.Switch, cg.Component
)

InputType = aa55_const_ns.enum("INPUT_TYPE", is_class=True)
OnOff = aa55_const_ns.enum("ON_OFF", is_class=True)


CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.Optional(
                CONF_START_STOP,
            ): switch.switch_schema(class_=AA55InverterSwitch).extend(
                {
                    cv.Optional(CONF_OFFLINE_HOLD, default=False): cv.boolean,
                    cv.Optional(CONF_OFFLINE_VALUE, default="OFF"): cv.enum(
                        ON_OFF_OPTIONS, upper=True
                    ),
                    cv.Optional(CONF_ONLINE_INTIAL_VALUE, default="ON"): cv.enum(
                        ON_OFF_OPTIONS, upper=True
                    ),
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
        if id and id.type == switch.Switch:
            var = cg.new_Pvariable(
                id,
                key,
                getattr(InputType, key.upper()),
                inverter,
                conf.get(CONF_OFFLINE_HOLD, False),
                OnOff(conf.get(CONF_OFFLINE_VALUE, "OFF")),
                OnOff(conf.get(CONF_ONLINE_INTIAL_VALUE, "ON")),
            )
            await cg.register_component(var, conf)
            await switch.register_switch(var, conf)
            cg.add(inverter.add_input(var))
