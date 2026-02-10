idf_component_register(SRCS "esp32_sensacional.c"                          
                        INCLUDE_DIRS "include"
                        REQUIRES driver
                                 Wrapper_BME280
                                 Wrapper_SGP41
                        )