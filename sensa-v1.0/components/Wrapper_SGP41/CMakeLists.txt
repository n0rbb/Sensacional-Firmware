idf_component_register(SRCS "../Sensor_SGP41/sensirion_i2c.c" "../Sensor_SGP41/sensirion_common.c"
                            "sensirion_i2c_hal_esp32.c" "../Sensor_SGP41/sgp41_i2c.c"
                    INCLUDE_DIRS "../Sensor_SGP41"
                    REQUIRES driver
                             freertos
                             )