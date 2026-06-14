import esphome.codegen as cg
from esphome.components import uart
import esphome.config_validation as cv
from esphome.const import CONF_ID

DEPENDENCIES = ["uart"]
MULTI_CONF = True

CONF_PYLONTECH_GROUP_ID = "pylontech_group_id"

pylontech_group_ns = cg.esphome_ns.namespace("pylontech_group")
pylontech_battery_ns = cg.esphome_ns.namespace("pylontech_battery")
PylontechGroup = pylontech_group_ns.class_(
    "PylontechGroup", cg.Component, uart.UARTDevice
)
PylontechBattery = pylontech_battery_ns.class_("PylontechBattery")

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(PylontechGroup),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(uart.UART_DEVICE_SCHEMA)
)


def pylontech_group_schema():
    schema = {
        cv.GenerateID(CONF_PYLONTECH_GROUP_ID): cv.use_id(PylontechGroup),
    }
    return cv.Schema(schema)


async def add_pylontech_battery(var, config):
    pylontech_group = await cg.get_variable(config[CONF_PYLONTECH_GROUP_ID])
    cg.add(pylontech_group.add_battery(var))
    cg.add(var.set_parent_group(pylontech_group))


async def to_code(config):
    var = cg.new_Pvariable(
        config[CONF_ID],
        config[CONF_ID].id,
    )
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)
