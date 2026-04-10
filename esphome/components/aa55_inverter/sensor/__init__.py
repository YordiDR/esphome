import esphome.codegen as cg
from esphome.components import sensor
import esphome.config_validation as cv
from esphome.const import (
    CONF_ID,
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

from .. import (
    CONF_INVERTER_ID,
    CONF_OFFLINE_HOLD,
    CONF_OFFLINE_VALUE,
    INVERTER_CHILD_SCHEMA,
    aa55_const_ns,
    aa55_inverter_ns,
)

DEPENDENCIES = ["aa55_inverter"]

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
CONF_NOM_VPV = "nom_vpv"
CONF_COUNTRY_CODE = "country_code"
CONF_SKIP_UPDATES = "skip_updates"

AA55InverterSensor = aa55_inverter_ns.class_(
    "AA55InverterSensor", sensor.Sensor, cg.Component
)
SensorType = aa55_const_ns.enum("SENSOR_TYPE", is_class=True)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.Optional(
                CONF_VPV1,
            ): sensor.sensor_schema(
                class_=AA55InverterSensor,
                unit_of_measurement=UNIT_VOLT,
                accuracy_decimals=1,
                state_class=STATE_CLASS_MEASUREMENT,
                device_class=DEVICE_CLASS_VOLTAGE,
            ).extend(
                {
                    cv.Optional(CONF_SKIP_UPDATES, default=0): cv.positive_int,
                    cv.Optional(CONF_OFFLINE_HOLD, default=False): cv.boolean,
                    cv.Optional(CONF_OFFLINE_VALUE, default=float("nan")): cv.float_,
                }
            ),
            cv.Optional(
                CONF_VPV2,
            ): sensor.sensor_schema(
                class_=AA55InverterSensor,
                unit_of_measurement=UNIT_VOLT,
                accuracy_decimals=1,
                state_class=STATE_CLASS_MEASUREMENT,
                device_class=DEVICE_CLASS_VOLTAGE,
            ).extend(
                {
                    cv.Optional(CONF_SKIP_UPDATES, default=0): cv.positive_int,
                    cv.Optional(CONF_OFFLINE_HOLD, default=False): cv.boolean,
                    cv.Optional(CONF_OFFLINE_VALUE, default=float("nan")): cv.float_,
                }
            ),
            cv.Optional(
                CONF_IPV1,
            ): sensor.sensor_schema(
                class_=AA55InverterSensor,
                unit_of_measurement=UNIT_AMPERE,
                accuracy_decimals=1,
                state_class=STATE_CLASS_MEASUREMENT,
                device_class=DEVICE_CLASS_CURRENT,
            ).extend(
                {
                    cv.Optional(CONF_SKIP_UPDATES, default=0): cv.positive_int,
                    cv.Optional(CONF_OFFLINE_HOLD, default=False): cv.boolean,
                    cv.Optional(CONF_OFFLINE_VALUE, default=float("nan")): cv.float_,
                }
            ),
            cv.Optional(
                CONF_IPV2,
            ): sensor.sensor_schema(
                class_=AA55InverterSensor,
                unit_of_measurement=UNIT_AMPERE,
                accuracy_decimals=1,
                state_class=STATE_CLASS_MEASUREMENT,
                device_class=DEVICE_CLASS_CURRENT,
            ).extend(
                {
                    cv.Optional(CONF_SKIP_UPDATES, default=0): cv.positive_int,
                    cv.Optional(CONF_OFFLINE_HOLD, default=False): cv.boolean,
                    cv.Optional(CONF_OFFLINE_VALUE, default=float("nan")): cv.float_,
                }
            ),
            cv.Optional(
                CONF_VAC1,
            ): sensor.sensor_schema(
                class_=AA55InverterSensor,
                unit_of_measurement=UNIT_VOLT,
                accuracy_decimals=1,
                state_class=STATE_CLASS_MEASUREMENT,
                device_class=DEVICE_CLASS_VOLTAGE,
            ).extend(
                {
                    cv.Optional(CONF_SKIP_UPDATES, default=0): cv.positive_int,
                    cv.Optional(CONF_OFFLINE_HOLD, default=False): cv.boolean,
                    cv.Optional(CONF_OFFLINE_VALUE, default=float("nan")): cv.float_,
                }
            ),
            cv.Optional(
                CONF_IAC1,
            ): sensor.sensor_schema(
                class_=AA55InverterSensor,
                unit_of_measurement=UNIT_AMPERE,
                accuracy_decimals=1,
                state_class=STATE_CLASS_MEASUREMENT,
                device_class=DEVICE_CLASS_CURRENT,
            ).extend(
                {
                    cv.Optional(CONF_SKIP_UPDATES, default=0): cv.positive_int,
                    cv.Optional(CONF_OFFLINE_HOLD, default=False): cv.boolean,
                    cv.Optional(CONF_OFFLINE_VALUE, default=float("nan")): cv.float_,
                }
            ),
            cv.Optional(
                CONF_FAC1,
            ): sensor.sensor_schema(
                class_=AA55InverterSensor,
                unit_of_measurement=UNIT_HERTZ,
                accuracy_decimals=2,
                state_class=STATE_CLASS_MEASUREMENT,
                device_class=DEVICE_CLASS_FREQUENCY,
            ).extend(
                {
                    cv.Optional(CONF_SKIP_UPDATES, default=0): cv.positive_int,
                    cv.Optional(CONF_OFFLINE_HOLD, default=False): cv.boolean,
                    cv.Optional(CONF_OFFLINE_VALUE, default=float("nan")): cv.float_,
                }
            ),
            cv.Optional(
                CONF_PAC,
            ): sensor.sensor_schema(
                class_=AA55InverterSensor,
                unit_of_measurement=UNIT_WATT,
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
                device_class=DEVICE_CLASS_POWER,
            ).extend(
                {
                    cv.Optional(CONF_SKIP_UPDATES, default=0): cv.positive_int,
                    cv.Optional(CONF_OFFLINE_HOLD, default=False): cv.boolean,
                    cv.Optional(CONF_OFFLINE_VALUE, default=0): cv.float_,
                }
            ),
            cv.Optional(
                CONF_TEMPERATURE,
            ): sensor.sensor_schema(
                class_=AA55InverterSensor,
                unit_of_measurement=UNIT_CELSIUS,
                accuracy_decimals=1,
                state_class=STATE_CLASS_MEASUREMENT,
                device_class=DEVICE_CLASS_TEMPERATURE,
            ).extend(
                {
                    cv.Optional(CONF_SKIP_UPDATES, default=0): cv.positive_int,
                    cv.Optional(CONF_OFFLINE_HOLD, default=False): cv.boolean,
                    cv.Optional(CONF_OFFLINE_VALUE, default=float("nan")): cv.float_,
                }
            ),
            cv.Optional(
                CONF_E_TOTAL,
            ): sensor.sensor_schema(
                class_=AA55InverterSensor,
                unit_of_measurement=UNIT_KILOWATT_HOURS,
                accuracy_decimals=1,
                state_class=STATE_CLASS_TOTAL_INCREASING,
                device_class=DEVICE_CLASS_ENERGY,
            ).extend(
                {
                    cv.Optional(CONF_SKIP_UPDATES, default=0): cv.positive_int,
                    cv.Optional(CONF_OFFLINE_HOLD, default=True): cv.boolean,
                    cv.Optional(CONF_OFFLINE_VALUE, default=float("nan")): cv.float_,
                }
            ),
            cv.Optional(
                CONF_H_TOTAL,
            ): sensor.sensor_schema(
                class_=AA55InverterSensor,
                unit_of_measurement=UNIT_HOUR,
                accuracy_decimals=0,
                state_class=STATE_CLASS_TOTAL_INCREASING,
                device_class=DEVICE_CLASS_DURATION,
            ).extend(
                {
                    cv.Optional(CONF_SKIP_UPDATES, default=0): cv.positive_int,
                    cv.Optional(CONF_OFFLINE_HOLD, default=True): cv.boolean,
                    cv.Optional(CONF_OFFLINE_VALUE, default=float("nan")): cv.float_,
                }
            ),
            cv.Optional(
                CONF_GFCI_FAULT_VALUE,
            ): sensor.sensor_schema(
                class_=AA55InverterSensor,
                unit_of_measurement=UNIT_MILLIAMP,
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
                device_class=DEVICE_CLASS_CURRENT,
            ).extend(
                {
                    cv.Optional(CONF_SKIP_UPDATES, default=0): cv.positive_int,
                    cv.Optional(CONF_OFFLINE_HOLD, default=True): cv.boolean,
                    cv.Optional(CONF_OFFLINE_VALUE, default=float("nan")): cv.float_,
                }
            ),
            cv.Optional(
                CONF_E_TODAY,
            ): sensor.sensor_schema(
                class_=AA55InverterSensor,
                unit_of_measurement=UNIT_KILOWATT_HOURS,
                accuracy_decimals=1,
                state_class=STATE_CLASS_MEASUREMENT,
                device_class=DEVICE_CLASS_ENERGY,
            ).extend(
                {
                    cv.Optional(CONF_SKIP_UPDATES, default=0): cv.positive_int,
                    cv.Optional(CONF_OFFLINE_HOLD, default=False): cv.boolean,
                    cv.Optional(CONF_OFFLINE_VALUE, default=float("nan")): cv.float_,
                }
            ),
            cv.Optional(
                CONF_COUNTRY_CODE,
            ): sensor.sensor_schema(
                class_=AA55InverterSensor,
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ).extend(
                {
                    cv.Optional(CONF_SKIP_UPDATES, default=0): cv.positive_int,
                    cv.Optional(CONF_OFFLINE_HOLD, default=False): cv.boolean,
                    cv.Optional(CONF_OFFLINE_VALUE, default=float("nan")): cv.float_,
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
        if id and id.type == sensor.Sensor:
            skip_updates = 0
            if CONF_SKIP_UPDATES in conf:
                skip_updates = conf[CONF_SKIP_UPDATES]

            var = cg.new_Pvariable(
                id,
                key,
                getattr(SensorType, key.upper()),
                skip_updates,
                conf.get(CONF_OFFLINE_HOLD, False),
                conf.get(CONF_OFFLINE_VALUE, float("nan")),
            )
            await cg.register_component(var, conf)
            await sensor.register_sensor(var, conf)
            cg.add(inverter.add_sensor(var))
