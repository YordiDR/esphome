import esphome.codegen as cg
from esphome.components import sensor
import esphome.config_validation as cv
from esphome.const import (
    CONF_ID,
    CONF_NAME,
    DEVICE_CLASS_CURRENT,
    DEVICE_CLASS_DURATION,
    DEVICE_CLASS_ENERGY,
    DEVICE_CLASS_FREQUENCY,
    DEVICE_CLASS_POWER,
    DEVICE_CLASS_TEMPERATURE,
    DEVICE_CLASS_VOLTAGE,
    STATE_CLASS_MEASUREMENT,
    STATE_CLASS_TOTAL_INCREASING,
    UNIT_AMPERE,
    UNIT_CELSIUS,
    UNIT_HERTZ,
    UNIT_HOUR,
    UNIT_KILOWATT_HOURS,
    UNIT_MILLIAMP,
    UNIT_VOLT,
    UNIT_WATT,
)

from .. import CONF_GOODWE_AA55_ID, HUB_CHILD_SCHEMA, goodwe_aa55_ns

DEPENDENCIES = ["goodwe_aa55"]

CONF_VPV1 = "vpv1"
CONF_VPV2 = "vpv2"
CONF_IPV1 = "ipv1"
CONF_IPV2 = "ipv2"
CONF_VAC1 = "vac1"
CONF_IAC1 = "iac1"
CONF_FAC1 = "fac1"
CONF_PAC = "pac"
CONF_TEMPERATURE = "temperature"
CONF_E_TOTAL = "e_total"
CONF_H_TOTAL = "h_total"
CONF_TEMPERATURE_FAULT_VALUE = "temperature_fault_value"
CONF_VPV1_FAULT_VALUE = "vpv1_fault_value"
CONF_VPV2_FAULT_VALUE = "vpv2_fault_value"
CONF_VAC1_FAULT_VALUE = "vac1_fault_value"
CONF_FAC1_FAULT_VALUE = "fac1_fault_value"
CONF_GFCI_FAULT_VALUE = "gfci_fault_value"
CONF_E_TODAY = "e_today"
CONF_SKIP_UPDATES = "skip_updates"

GoodweAA55Sensor = goodwe_aa55_ns.class_(
    "GoodweAA55Sensor", sensor.Sensor, cg.Component
)
SensorType = goodwe_aa55_ns.enum("SENSOR_TYPE", is_class=True)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.Optional(
                CONF_VPV1,
                default={CONF_ID: "vpv1", CONF_NAME: "PV1 Voltage (Vpv1)"},
            ): sensor.sensor_schema(
                class_=GoodweAA55Sensor,
                unit_of_measurement=UNIT_VOLT,
                accuracy_decimals=1,
                state_class=STATE_CLASS_MEASUREMENT,
                device_class=DEVICE_CLASS_VOLTAGE,
            ).extend({cv.Optional(CONF_SKIP_UPDATES, default=0): cv.positive_int}),
            cv.Optional(
                CONF_VPV2,
                default={CONF_ID: "vpv2", CONF_NAME: "PV2 Voltage (Vpv2)"},
            ): sensor.sensor_schema(
                class_=GoodweAA55Sensor,
                unit_of_measurement=UNIT_VOLT,
                accuracy_decimals=1,
                state_class=STATE_CLASS_MEASUREMENT,
                device_class=DEVICE_CLASS_VOLTAGE,
            ).extend({cv.Optional(CONF_SKIP_UPDATES, default=0): cv.positive_int}),
            cv.Optional(
                CONF_IPV1,
                default={CONF_ID: "ipv1", CONF_NAME: "PV1 Current (Ipv1)"},
            ): sensor.sensor_schema(
                class_=GoodweAA55Sensor,
                unit_of_measurement=UNIT_AMPERE,
                accuracy_decimals=1,
                state_class=STATE_CLASS_MEASUREMENT,
                device_class=DEVICE_CLASS_CURRENT,
            ).extend({cv.Optional(CONF_SKIP_UPDATES, default=0): cv.positive_int}),
            cv.Optional(
                CONF_IPV2,
                default={CONF_ID: "ipv2", CONF_NAME: "PV2 Current (Ipv2)"},
            ): sensor.sensor_schema(
                class_=GoodweAA55Sensor,
                unit_of_measurement=UNIT_AMPERE,
                accuracy_decimals=1,
                state_class=STATE_CLASS_MEASUREMENT,
                device_class=DEVICE_CLASS_CURRENT,
            ).extend({cv.Optional(CONF_SKIP_UPDATES, default=0): cv.positive_int}),
            cv.Optional(
                CONF_VAC1,
                default={CONF_ID: "vac1", CONF_NAME: "Phase 1 Voltage (Vac1)"},
            ): sensor.sensor_schema(
                class_=GoodweAA55Sensor,
                unit_of_measurement=UNIT_VOLT,
                accuracy_decimals=1,
                state_class=STATE_CLASS_MEASUREMENT,
                device_class=DEVICE_CLASS_VOLTAGE,
            ).extend({cv.Optional(CONF_SKIP_UPDATES, default=0): cv.positive_int}),
            cv.Optional(
                CONF_IAC1,
                default={CONF_ID: "iac1", CONF_NAME: "Phase 1 Current (Iac1)"},
            ): sensor.sensor_schema(
                class_=GoodweAA55Sensor,
                unit_of_measurement=UNIT_AMPERE,
                accuracy_decimals=1,
                state_class=STATE_CLASS_MEASUREMENT,
                device_class=DEVICE_CLASS_CURRENT,
            ).extend({cv.Optional(CONF_SKIP_UPDATES, default=0): cv.positive_int}),
            cv.Optional(
                CONF_FAC1,
                default={CONF_ID: "fac1", CONF_NAME: "Phase 1 Frequency (Fac1)"},
            ): sensor.sensor_schema(
                class_=GoodweAA55Sensor,
                unit_of_measurement=UNIT_HERTZ,
                accuracy_decimals=2,
                state_class=STATE_CLASS_MEASUREMENT,
                device_class=DEVICE_CLASS_FREQUENCY,
            ).extend({cv.Optional(CONF_SKIP_UPDATES, default=0): cv.positive_int}),
            cv.Optional(
                CONF_PAC, default={CONF_ID: "pac", CONF_NAME: "AC Power (Pac)"}
            ): sensor.sensor_schema(
                class_=GoodweAA55Sensor,
                unit_of_measurement=UNIT_WATT,
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
                device_class=DEVICE_CLASS_POWER,
            ).extend({cv.Optional(CONF_SKIP_UPDATES, default=0): cv.positive_int}),
            cv.Optional(
                CONF_TEMPERATURE,
                default={CONF_ID: "temperature", CONF_NAME: "Temperature"},
            ): sensor.sensor_schema(
                class_=GoodweAA55Sensor,
                unit_of_measurement=UNIT_CELSIUS,
                accuracy_decimals=1,
                state_class=STATE_CLASS_MEASUREMENT,
                device_class=DEVICE_CLASS_TEMPERATURE,
            ).extend({cv.Optional(CONF_SKIP_UPDATES, default=0): cv.positive_int}),
            cv.Optional(
                CONF_E_TOTAL,
                default={
                    CONF_ID: "e_total",
                    CONF_NAME: "Total energy generated (E-total)",
                },
            ): sensor.sensor_schema(
                class_=GoodweAA55Sensor,
                unit_of_measurement=UNIT_KILOWATT_HOURS,
                accuracy_decimals=1,
                state_class=STATE_CLASS_TOTAL_INCREASING,
                device_class=DEVICE_CLASS_ENERGY,
            ).extend({cv.Optional(CONF_SKIP_UPDATES, default=0): cv.positive_int}),
            cv.Optional(
                CONF_H_TOTAL,
                default={CONF_ID: "h_total", CONF_NAME: "Total runtime (H-total)"},
            ): sensor.sensor_schema(
                class_=GoodweAA55Sensor,
                unit_of_measurement=UNIT_HOUR,
                accuracy_decimals=0,
                state_class=STATE_CLASS_TOTAL_INCREASING,
                device_class=DEVICE_CLASS_DURATION,
            ).extend({cv.Optional(CONF_SKIP_UPDATES, default=0): cv.positive_int}),
            cv.Optional(
                CONF_GFCI_FAULT_VALUE,
                default={CONF_ID: "gfci_fault_value", CONF_NAME: "GFCI fault value"},
            ): sensor.sensor_schema(
                class_=GoodweAA55Sensor,
                unit_of_measurement=UNIT_MILLIAMP,
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
                device_class=DEVICE_CLASS_CURRENT,
            ).extend({cv.Optional(CONF_SKIP_UPDATES, default=0): cv.positive_int}),
            cv.Optional(
                CONF_E_TODAY,
                default={
                    CONF_ID: "e_today",
                    CONF_NAME: "Energy generated today (E-today)",
                },
            ): sensor.sensor_schema(
                class_=GoodweAA55Sensor,
                unit_of_measurement=UNIT_KILOWATT_HOURS,
                accuracy_decimals=1,
                state_class=STATE_CLASS_MEASUREMENT,
                device_class=DEVICE_CLASS_ENERGY,
            ).extend({cv.Optional(CONF_SKIP_UPDATES, default=0): cv.positive_int}),
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
            # await cg.register_component(sens, conf)
            cg.add(sens.set_skip_updates(conf[CONF_SKIP_UPDATES]))
            cg.add(sens.set_id(key))
            cg.add(sens.set_type(getattr(SensorType, key.upper())))
            cg.add(hub.add_sensor(sens))
