import esphome.codegen as cg
from esphome.components import uart
import esphome.config_validation as cv
from esphome.const import CONF_ID

DEPENDENCIES = ["uart"]
MULTI_CONF = True

CONF_AA55_BUS_ID = "aa55_bus_id"
CONF_SERIAL_NUMBER = "serial_number"
CONF_SLAVE_ADDRESS = "slave_address"
CONF_MASTER_ADDRESS = "master_address"

aa55_bus_ns = cg.esphome_ns.namespace("aa55_bus")
AA55Bus = aa55_bus_ns.class_("AA55Bus", cg.Component, uart.UARTDevice)
AA55Inverter = aa55_bus_ns.class_("AA55Inverter")

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(AA55Bus),
            cv.Optional(CONF_MASTER_ADDRESS, default=0xFF): cv.hex_uint8_t,
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(uart.UART_DEVICE_SCHEMA)
)


def aa55_inverter_schema():
    schema = {
        cv.GenerateID(CONF_AA55_BUS_ID): cv.use_id(AA55Bus),
        cv.Required(CONF_SERIAL_NUMBER): cv.string,
        cv.Required(CONF_SLAVE_ADDRESS): cv.hex_uint8_t,
    }
    return cv.Schema(schema)


async def add_aa55_inverter(var, config):
    aa55_bus = await cg.get_variable(config[CONF_AA55_BUS_ID])
    cg.add(aa55_bus.add_inverter(var))
    cg.add(var.set_parent_bus(aa55_bus))


async def to_code(config):
    var = cg.new_Pvariable(
        config[CONF_ID],
        config[CONF_ID].id,
        config[CONF_MASTER_ADDRESS],
    )
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)
