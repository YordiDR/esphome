import esphome.codegen as cg
from esphome.components import sensor
import esphome.config_validation as cv
from esphome.const import (
    CONF_ID,
    CONF_NAME,
    DEVICE_CLASS_POWER,
    DEVICE_CLASS_VOLTAGE,
    STATE_CLASS_MEASUREMENT,
    UNIT_VOLT,
    UNIT_WATT,
)

from . import CONF_GOODWE_AA55_ID, HUB_CHILD_SCHEMA

DEPENDENCIES = ["goodwe_aa55"]

CONF_PAC = "pac"
CONF_VPV1 = "vpv1"

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.Optional(
                CONF_PAC, default={CONF_ID: "pac", CONF_NAME: "AC Power (Pac)"}
            ): sensor.sensor_schema(
                unit_of_measurement=UNIT_WATT,
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
                device_class=DEVICE_CLASS_POWER,
            ),
            cv.Optional(
                CONF_VPV1,
                default={CONF_ID: "vpv1", CONF_NAME: "PV1 Voltage (Vpv1)"},
            ): sensor.sensor_schema(
                unit_of_measurement=UNIT_VOLT,
                accuracy_decimals=1,
                state_class=STATE_CLASS_MEASUREMENT,
                device_class=DEVICE_CLASS_VOLTAGE,
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
        if id and id.type == sensor.Sensor:
            sens = await sensor.new_sensor(conf)
            cg.add(getattr(hub, f"set_{key}_sensor")(sens))
