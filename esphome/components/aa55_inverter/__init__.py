import esphome.codegen as cg
from esphome.components import aa55_bus
import esphome.config_validation as cv
from esphome.const import CONF_ID

AUTO_LOAD = ["aa55_bus"]
MULTI_CONF = True

CONF_INVERTER_ID = "aa55_inverter_id"
CONF_SERIAL_NUMBER = "serial_number"
CONF_SLAVE_ADDRESS = "slave_address"

aa55_inverter_ns = cg.esphome_ns.namespace("aa55_inverter")
aa55_bus_ns = cg.esphome_ns.namespace("aa55_bus")
aa55_const_ns = cg.esphome_ns.namespace("aa55_const")
AA55Inverter = aa55_inverter_ns.class_(
    "AA55Inverter", cg.PollingComponent, aa55_bus.AA55Inverter
)

INVERTER_CHILD_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_INVERTER_ID): cv.use_id(AA55Inverter),
    }
)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(AA55Inverter),
            cv.Required(CONF_SERIAL_NUMBER): cv.string,
            cv.Required(CONF_SLAVE_ADDRESS): cv.hex_uint8_t,
        }
    )
    .extend(cv.polling_component_schema("60s"))
    .extend(aa55_bus.aa55_inverter_schema())
)


async def to_code(config):
    var = cg.new_Pvariable(
        config[CONF_ID],
        config[CONF_SERIAL_NUMBER],
        config[CONF_SLAVE_ADDRESS],
    )
    await cg.register_component(var, config)
    await aa55_bus.register_aa55_inverter(var, config)
