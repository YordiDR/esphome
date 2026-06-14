import esphome.codegen as cg
from esphome.components import pylontech_group
import esphome.config_validation as cv
from esphome.const import CONF_ID

AUTO_LOAD = ["pylontech_group"]
MULTI_CONF = True

CONF_PYLONTECH_GROUP_ID = "pylontech_group_id"
CONF_PYLONTECH_BATTERY_ID = "pylontech_battery_id"
CONF_BATTERY_NUMBER = "battery_number"

pylontech_battery_ns = cg.esphome_ns.namespace("pylontech_battery")
pylontech_group_ns = cg.esphome_ns.namespace("pylontech_group")
PylontechBattery = pylontech_battery_ns.class_("PylontechBattery", cg.PollingComponent)

BATTERY_CHILD_SCHEMA = cv.Schema(
    {cv.GenerateID(CONF_PYLONTECH_BATTERY_ID): cv.use_id(PylontechBattery)}
)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(PylontechBattery),
            cv.Required(CONF_BATTERY_NUMBER): cv.int_range(min=1, max=16),
        }
    )
    .extend(cv.polling_component_schema("60s"))
    .extend(pylontech_group.pylontech_group_schema())
)


async def to_code(config):
    var = cg.new_Pvariable(
        config[CONF_ID],
        config[CONF_BATTERY_NUMBER],
    )
    await cg.register_component(var, config)
    await pylontech_group.add_pylontech_battery(var, config)
