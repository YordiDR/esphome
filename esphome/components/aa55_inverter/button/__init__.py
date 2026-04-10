import esphome.codegen as cg
from esphome.components import button
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_NAME

from .. import CONF_INVERTER_ID, INVERTER_CHILD_SCHEMA, aa55_const_ns, aa55_inverter_ns

DEPENDENCIES = ["aa55_inverter"]

CONF_RECONNECT_GRID = "reconnect_grid"

AA55InverterButton = aa55_inverter_ns.class_(
    "AA55InverterButton", button.Button, cg.Component
)
InputType = aa55_const_ns.enum("INPUT_TYPE", is_class=True)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.Optional(
                CONF_RECONNECT_GRID,
                default={CONF_ID: "reconnect_grid", CONF_NAME: "Reconnect Grid"},
            ): button.button_schema(class_=AA55InverterButton)
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
        if id and id.type == button.Button:
            var = cg.new_Pvariable(id, key, getattr(InputType, key.upper()), inverter)
            await cg.register_component(var, conf)
            await button.register_button(var, conf)
            cg.add(inverter.add_input(var))
