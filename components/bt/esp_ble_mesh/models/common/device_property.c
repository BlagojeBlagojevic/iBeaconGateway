/*
 * SPDX-FileCopyrightText: 2017-2021 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mesh/common.h"
#include "mesh/server_common.h"
#include "mesh/device_property.h"

static struct bt_mesh_dev_prop {
    uint16_t prop_id;
    uint8_t  len;
} device_properties[] = {
    { BLE_MESH_INVALID_DEVICE_PROPERTY_ID,                               0xFF }, /* Prohibited */
    { BLE_MESH_AVERAGE_AMBIENT_TEMPERATURE_IN_A_PERIOD_OF_DAY,           0x03 },
    { BLE_MESH_AVERAGE_INPUT_CURRENT,                                    0x03 },
    { BLE_MESH_AVERAGE_INPUT_VOLTAGE,                                    0x03 },
    { BLE_MESH_AVERAGE_OUTPUT_CURRENT,                                   0x03 },
    { BLE_MESH_AVERAGE_OUTPUT_VOLTAGE,                                   0x03 },
    { BLE_MESH_CENTER_BEAM_INTENSITY_AT_FULL_POWER,                      0x02 },
    { BLE_MESH_CHROMATICITY_TOLERANCE,                                   0x01 },
    { BLE_MESH_COLOR_RENDERING_INDEX_R9,                                 0x01 },
    { BLE_MESH_COLOR_RENDERING_INDEX_RA,                                 0x01 },
    { BLE_MESH_DEVICE_APPEARANCE,                                        0x02 },
    { BLE_MESH_DEVICE_COUNTRY_OF_ORIGIN,                                 0x02 },
    { BLE_MESH_DEVICE_DATE_OF_MANUFACTURE,                               0x03 },
    { BLE_MESH_DEVICE_ENERGY_USE_SINCE_TURN_ON,                          0x03 },
    { BLE_MESH_DEVICE_FIRMWARE_REVISION,                                 0x08 },
    { BLE_MESH_DEVICE_GLOBAL_TRADE_ITEM_NUMBER,                          0x06 },
    { BLE_MESH_DEVICE_HARDWARE_REVISION,                                 0x10 },
    { BLE_MESH_DEVICE_MANUFACTURER_NAME,                                 0x24 },
    { BLE_MESH_DEVICE_MODEL_NUMBER,                                      0x18 },
    { BLE_MESH_DEVICE_OPERATING_TEMPERATURE_RANGE_SPECIFICATION,         0x04 },
    { BLE_MESH_DEVICE_OPERATING_TEMPERATURE_STATISTICAL_VALUES,          0x09 },
    { BLE_MESH_DEVICE_OVER_TEMPERATURE_EVENT_STATISTICS,                 0x06 },
    { BLE_MESH_DEVICE_POWER_RANGE_SPECIFICATION,                         0x09 },
    { BLE_MESH_DEVICE_RUNTIME_SINCE_TURN_ON,                             0x03 },
    { BLE_MESH_DEVICE_RUNTIME_WARRANTY,                                  0x03 },
    { BLE_MESH_DEVICE_SERIAL_NUMBER,                                     0x10 },
    { BLE_MESH_DEVICE_SOFTWARE_REVISION,                                 0x08 },
    { BLE_MESH_DEVICE_UNDER_TEMPERATURE_EVENT_STATISTICS,                0x06 },
    { BLE_MESH_INDOOR_AMBIENT_TEMPERATURE_STATISTICAL_VALUES,            0x05 },
    { BLE_MESH_INITIAL_CIE_1931_CHROMATICITY_COORDINATES,                0x04 },
    { BLE_MESH_INITIAL_CORRELATED_COLOR_TEMPERATURE,                     0x02 },
    { BLE_MESH_INITIAL_LUMINOUS_FLUX,                                    0x02 },
    { BLE_MESH_INITIAL_PLANCKIAN_DISTANCE,                               0x02 },
    { BLE_MESH_INPUT_CURRENT_RANGE_SPECIFICATION,                        0x06 },
    { BLE_MESH_INPUT_CURRENT_STATISTICS,                                 0x09 },
    { BLE_MESH_INPUT_OVER_CURRENT_EVENT_STATISTICS,                      0x06 },
    { BLE_MESH_INPUT_OVER_RIPPLE_VOLTAGE_EVENT_STATISTICS,               0x06 },
    { BLE_MESH_INPUT_OVER_VOLTAGE_EVENT_STATISTICS,                      0x06 },
    { BLE_MESH_INPUT_UNDER_CURRENT_EVENT_STATISTICS,                     0x06 },
    { BLE_MESH_INPUT_UNDER_VOLTAGE_EVENT_STATISTICS,                     0x06 },
    { BLE_MESH_INPUT_VOLTAGE_RANGE_SPECIFICATION,                        0x06 },
    { BLE_MESH_INPUT_VOLTAGE_RIPPLE_SPECIFICATION,                       0x01 },
    { BLE_MESH_INPUT_VOLTAGE_STATISTICS,                                 0x09 },
    { BLE_MESH_LIGHT_CONTROL_AMBIENT_LUXLEVEL_ON,                        0x03 },
    { BLE_MESH_LIGHT_CONTROL_AMBIENT_LUXLEVEL_PROLONG,                   0x03 },
    { BLE_MESH_LIGHT_CONTROL_AMBIENT_LUXLEVEL_STANDBY,                   0x03 },
    { BLE_MESH_LIGHT_CONTROL_LIGHTNESS_ON,                               0x02 },
    { BLE_MESH_LIGHT_CONTROL_LIGHTNESS_PROLONG,                          0x02 },
    { BLE_MESH_LIGHT_CONTROL_LIGHTNESS_STANDBY,                          0x02 },
    { BLE_MESH_LIGHT_CONTROL_REGULATOR_ACCURACY,                         0x01 },
    { BLE_MESH_LIGHT_CONTROL_REGULATOR_KID,                              0x04 },
    { BLE_MESH_LIGHT_CONTROL_REGULATOR_KIU,                              0x04 },
    { BLE_MESH_LIGHT_CONTROL_REGULATOR_KPD,                              0x04 },
    { BLE_MESH_LIGHT_CONTROL_REGULATOR_KPU,                              0x04 },
    { BLE_MESH_LIGHT_CONTROL_TIME_FADE,                                  0x03 },
    { BLE_MESH_LIGHT_CONTROL_TIME_FADE_ON,                               0x03 },
    { BLE_MESH_LIGHT_CONTROL_TIME_FADE_STANDBY_AUTO,                     0x03 },
    { BLE_MESH_LIGHT_CONTROL_TIME_FADE_STANDBY_MANUAL,                   0x03 },
    { BLE_MESH_LIGHT_CONTROL_TIME_OCCUPANCY_DELAY,                       0x03 },
    { BLE_MESH_LIGHT_CONTROL_TIME_PROLONG,                               0x03 },
    { BLE_MESH_LIGHT_CONTROL_TIME_RUN_ON,                                0x03 },
    { BLE_MESH_LUMEN_MAINTENANCE_FACTOR,                                 0x01 },
    { BLE_MESH_LUMINOUS_EFFICACY,                                        0x02 },
    { BLE_MESH_LUMINOUS_ENERGY_SINCE_TURN_ON,                            0x03 },
    { BLE_MESH_LUMINOUS_EXPOSURE,                                        0x03 },
    { BLE_MESH_LUMINOUS_FLUX_RANGE,                                      0x04 },
    { BLE_MESH_MOTION_SENSED,                                            0x01 },
    { BLE_MESH_MOTION_THRESHOLD,                                         0x01 },
    { BLE_MESH_OPEN_CIRCUIT_EVENT_STATISTICS,                            0x06 },
    { BLE_MESH_OUTDOOR_STATISTICAL_VALUES,                               0x05 },
    { BLE_MESH_OUTPUT_CURRENT_RANGE,                                     0x04 },
    { BLE_MESH_OUTPUT_CURRENT_STATISTICS,                                0x09 },
    { BLE_MESH_OUTPUT_RIPPLE_VOLTAGE_SPECIFICATION,                      0x01 },
    { BLE_MESH_OUTPUT_VOLTAGE_RANGE,                                     0x06 },
    { BLE_MESH_OUTPUT_VOLTAGE_STATISTICS,                                0x09 },
    { BLE_MESH_OVER_OUTPUT_RIPPLE_VOLTAGE_EVENT_STATISTICS,              0x06 },
    { BLE_MESH_PEOPLE_COUNT,                                             0x02 },
    { BLE_MESH_PRESENCE_DETECTED,                                        0x01 },
    { BLE_MESH_PRESENT_AMBIENT_LIGHT_LEVEL,                              0x03 },
    { BLE_MESH_PRESENT_AMBIENT_TEMPERATURE,                              0x01 },
    { BLE_MESH_PRESENT_CIE_1931_CHROMATICITY,                            0x04 },
    { BLE_MESH_PRESENT_CORRELATED_COLOR_TEMPERATURE,                     0x02 },
    { BLE_MESH_PRESENT_DEVICE_INPUT_POWER,                               0x03 },
    { BLE_MESH_PRESENT_DEVICE_OPERATING_EFFICIENCY,                      0x01 },
    { BLE_MESH_PRESENT_DEVICE_OPERATING_TEMPERATURE,                     0x02 },
    { BLE_MESH_PRESENT_ILLUMINANCE,                                      0x03 },
    { BLE_MESH_PRESENT_INDOOR_AMBIENT_TEMPERATURE,                       0x01 },
    { BLE_MESH_PRESENT_INPUT_CURRENT,                                    0x02 },
    { BLE_MESH_PRESENT_INPUT_RIPPLE_VOLTAGE,                             0x01 },
    { BLE_MESH_PRESENT_INPUT_VOLTAGE,                                    0x02 },
    { BLE_MESH_PRESENT_LUMINOUS_FLUX,                                    0x02 },
    { BLE_MESH_PRESENT_OUTDOOR_AMBIENT_TEMPERATURE,                      0x01 },
    { BLE_MESH_PRESENT_OUTPUT_CURRENT,                                   0x02 },
    { BLE_MESH_PRESENT_OUTPUT_VOLTAGE,                                   0x02 },
    { BLE_MESH_PRESENT_PLANCKIAN_DISTANCE,                               0x02 },
    { BLE_MESH_PRESENT_RELATIVE_OUTPUT_RIPPLE_VOLTAGE,                   0x01 },
    { BLE_MESH_RELATIVE_DEVICE_ENERGY_USE_IN_A_PERIOD_OF_DAY,            0x05 },
    { BLE_MESH_RELATIVE_DEVICE_RUNTIME_IN_A_GENERIC_LEVEL_RANGE,         0x05 },
    { BLE_MESH_RELATIVE_EXPOSURE_TIME_IN_AN_ILLUMINANCE_RANGE,           0x05 },
    { BLE_MESH_RELATIVE_RUNTIME_IN_A_CORRELATED_COLOR_TEMPERATURE_RANGE, 0x03 },
    { BLE_MESH_RELATIVE_RUNTIME_IN_A_DEVICE_OPERATING_TEMPERATURE_RANGE, 0x03 },
    { BLE_MESH_RELATIVE_RUNTIME_IN_AN_INPUT_CURRENT_RANGE,               0x05 },
    { BLE_MESH_RELATIVE_RUNTIME_IN_AN_INPUT_VOLTAGE_RANGE,               0x05 },
    { BLE_MESH_SHORT_CIRCUIT_EVENT_STATISTICS,                           0x06 },
    { BLE_MESH_TIME_SINCE_MOTION_SENSED,                                 0x02 },
    { BLE_MESH_TIME_SINCE_PRESENCE_DETECTED,                             0x02 },
    { BLE_MESH_TOTAL_DEVICE_ENERGY_USE,                                  0x03 },
    { BLE_MESH_TOTAL_DEVICE_OFF_ON_CYCLES,                               0x03 },
    { BLE_MESH_TOTAL_DEVICE_POWER_ON_CYCLES,                             0x03 },
    { BLE_MESH_TOTAL_DEVICE_POWER_ON_TIME,                               0x03 },
    { BLE_MESH_TOTAL_DEVICE_RUNTIME,                                     0x03 },
    { BLE_MESH_TOTAL_LIGHT_EXPOSURE_TIME,                                0x03 },
    { BLE_MESH_TOTAL_LUMINOUS_ENERGY,                                    0x03 },
    { BLE_MESH_DESIRED_AMBIENT_TEMPERATURE,                              0x01 },
    { BLE_MESH_PRECISE_TOTAL_DEVICE_ENERGY_USE,                          0x04 },
    { BLE_MESH_POWER_FACTOR,                                             0x01 },
    { BLE_MESH_SENSOR_GAIN,                                              0x04 },
    { BLE_MESH_PRECISE_PRESENT_AMBIENT_TEMPERATURE,                      0x02 },
    { BLE_MESH_PRESENT_AMBIENT_RELATIVE_HUMIDITY,                        0x02 },
    { BLE_MESH_PRESENT_AMBIENT_CARBON_DIOXIDE_CONCENTRATION,             0x02 },
    { BLE_MESH_PRESENT_AMBIENT_VOLATILE_ORGANIC_COMPOUNDS_CONCENTRATION, 0x02 },
    { BLE_MESH_PRESENT_AMBIENT_NOISE,                                    0x01 },
    { 0x007A,                                                            0xFF }, /* Not defined */
    { 0x007B,                                                            0xFF }, /* Not defined */
    { 0x007C,                                                            0xFF }, /* Not defined */
    { 0x007D,                                                            0xFF }, /* Not defined */
    { 0x007E,                                                            0xFF }, /* Not defined */
    { 0x007F,                                                            0xFF }, /* Not defined */
    { BLE_MESH_ACTIVE_ENERGY_LOADSIDE,                                   0x04 },
    { BLE_MESH_ACTIVE_POWER_LOADSIDE,                                    0x03 },
    { BLE_MESH_AIR_PRESSURE,                                             0x04 },
    { BLE_MESH_APPARENT_ENERGY,                                          0x04 },
    { BLE_MESH_APPARENT_POWER,                                           0x03 },
    { BLE_MESH_APPARENT_WIND_DIRECTION,                                  0x02 },
    { BLE_MESH_APPARENT_WIND_SPEED,                                      0x02 },
    { BLE_MESH_DEW_POINT,                                                0x01 },
    { BLE_MESH_EXTERNAL_SUPPLY_VOLTAGE,                                  0x03 },
    { BLE_MESH_EXTERNAL_SUPPLY_VOLTAGE_FREQUENCY,                        0x02 },
    { BLE_MESH_GUST_FACTOR,                                              0x01 },
    { BLE_MESH_HEAT_INDEX,                                               0x01 },
    { BLE_MESH_LIGHT_DISTRIBUTION,                                       0x01 },
    { BLE_MESH_LIGHT_SOURCE_CURRENT,                                     0x03 },
    { BLE_MESH_LIGHT_SOURCE_ON_TIME_NOT_RESETTABLE,                      0x04 },
    { BLE_MESH_LIGHT_SOURCE_ON_TIME_RESETTABLE,                          0x04 },
    { BLE_MESH_LIGHT_SOURCE_OPEN_CIRCUIT_STATISTICS,                     0x06 },
    { BLE_MESH_LIGHT_SOURCE_OVERALL_FAILURES_STATISTICS,                 0x06 },
    { BLE_MESH_LIGHT_SOURCE_SHORT_CIRCUIT_STATISTICS,                    0x06 },
    { BLE_MESH_LIGHT_SOURCE_START_COUNTER_RESETTABLE,                    0x03 },
    { BLE_MESH_LIGHT_SOURCE_TEMPERATURE,                                 0x02 },
    { BLE_MESH_LIGHT_SOURCE_THERMAL_DERATING_STATISTICS,                 0x06 },
    { BLE_MESH_LIGHT_SOURCE_THERMAL_SHUTDOWN_STATISTICS,                 0x06 },
    { BLE_MESH_LIGHT_SOURCE_TOTAL_POWER_ON_CYCLES,                       0x03 },
    { BLE_MESH_LIGHT_SOURCE_VOLTAGE,                                     0x03 },
    { BLE_MESH_LUMINAIRE_COLOR,                                          0x18 },
    { BLE_MESH_LUMINAIRE_IDENTIFICATION_NUMBER,                          0x18 },
    { BLE_MESH_LUMINAIRE_MANUFACTURER_GTIN,                              0x06 },
    { BLE_MESH_LUMINAIRE_NOMINAL_INPUT_POWER,                            0x03 },
    { BLE_MESH_LUMINAIRE_NOMINAL_MAXIMUM_AC_MAINS_VOLTAGE,               0x02 },
    { BLE_MESH_LUMINAIRE_NOMINAL_MINIMUM_AC_MAINS_VOLTAGE,               0x02 },
    { BLE_MESH_LUMINAIRE_POWER_AT_MINIMUM_DIM_LEVEL,                     0x03 },
    { BLE_MESH_LUMINAIRE_TIME_OF_MANUFACTURE,                            0x03 },
    { BLE_MESH_MAGNETIC_DECLINATION,                                     0x02 },
    { BLE_MESH_MAGNETIC_FLUX_DENSITY_2D,                                 0x04 },
    { BLE_MESH_MAGNETIC_FLUX_DENSITY_3D,                                 0x06 },
    { BLE_MESH_NOMINAL_LIGHT_OUTPUT,                                     0x03 },
    { BLE_MESH_OVERALL_FAILURE_CONDITION,                                0x06 },
    { BLE_MESH_POLLEN_CONCENTRATION,                                     0x03 },
    { BLE_MESH_PRESENT_INDOOR_RELATIVE_HUMIDITY,                         0x02 },
    { BLE_MESH_PRESENT_OUTDOOR_RELATIVE_HUMIDITY,                        0x02 },
    { BLE_MESH_PRESSURE,                                                 0x04 },
    { BLE_MESH_RAINFALL,                                                 0x02 },
    { BLE_MESH_RATED_MEDIAN_USEFUL_LIFE_OF_LUMINAIRE,                    0x03 },
    { BLE_MESH_RATED_MEDIAN_USEFUL_LIGHT_SOURCE_STARTS,                  0x03 },
    { BLE_MESH_REFERENCE_TEMPERATURE,                                    0x02 },
    { BLE_MESH_TOTAL_DEVICE_STARTS,                                      0x03 },
    { BLE_MESH_TRUE_WIND_DIRECTION,                                      0x02 },
    { BLE_MESH_TRUE_WIND_SPEED,                                          0x02 },
    { BLE_MESH_UV_INDEX,                                                 0x01 },
    { BLE_MESH_WIND_CHILL,                                               0x01 },
    { BLE_MESH_LIGHT_SOURCE_TYPE,                                        0x01 },
    { BLE_MESH_LUMINAIRE_IDENTIFICATION_STRING,                          0x40 },
    { BLE_MESH_OUTPUT_POWER_LIMITATION,                                  0x06 },
    { BLE_MESH_THERMAL_DERATING,                                         0x06 },
    { BLE_MESH_OUTPUT_CURRENT_PERCENT,                                   0x01 },
};

uint8_t bt_mesh_get_dev_prop_len(uint16_t prop_id)
{
    if (prop_id > BLE_MESH_OUTPUT_CURRENT_PERCENT) {
        BT_ERR("Unknown Device Property ID 0x%04x", prop_id);
        return UINT8_MAX;
    }

    return device_properties[prop_id].len;
}
