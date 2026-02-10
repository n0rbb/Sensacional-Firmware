idf_component_register(SRCS "wrapper_bme280.c" "../Sensor_BME280/bme280.c"
                    INCLUDE_DIRS "include" "../Sensor_BME280" 
                    REQUIRES driver
                             freertos
                    )