import esphome.codegen as cg
from esphome.components import uart
import esphome.config_validation as cv
from esphome.const import CONF_ID

DEPENDENCIES = ["uart"]
AUTO_LOAD = ["sensor"]
MULTI_CONF = True

CONF_GOODWE_AA55_ID = "goodwe_aa55_id"
CONF_SERIAL_NUMBER = "serial_number"
CONF_SLAVE_ADDRESS = "slave_address"
CONF_MASTER_ADDRESS = "master_address"
CONF_UPDATE_INTERVAL = "update_interval"

goodwe_aa55_ns = cg.esphome_ns.namespace("goodwe_aa55")
GoodweAA55 = goodwe_aa55_ns.class_("GoodweAA55", cg.PollingComponent, uart.UARTDevice)

HUB_CHILD_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_GOODWE_AA55_ID): cv.use_id(GoodweAA55),
    }
)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(GoodweAA55),
            cv.Required(CONF_SERIAL_NUMBER): cv.string,
            cv.Required(CONF_SLAVE_ADDRESS): cv.hex_uint8_t,
            cv.Optional(CONF_MASTER_ADDRESS, default=0xFF): cv.hex_uint8_t,
            cv.Optional(CONF_UPDATE_INTERVAL, default=30): cv.positive_int,
        }
    )
    .extend(cv.polling_component_schema("30s"))
    .extend(uart.UART_DEVICE_SCHEMA)
)


async def to_code(config):
    var = cg.new_Pvariable(
        config[CONF_ID],
        config[CONF_SERIAL_NUMBER],
        config[CONF_SLAVE_ADDRESS],
        config[CONF_MASTER_ADDRESS],
        config[CONF_UPDATE_INTERVAL],
    )
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)
