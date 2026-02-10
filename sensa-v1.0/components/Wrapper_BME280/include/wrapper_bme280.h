#ifndef _WRAPPER_BME280_H
#define _WRAPPER_BME280_H

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>

#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"
#include "rom/ets_sys.h"
#include "esp_err.h"

#include "bme280.h"

//LOW-LEVEL FUNCTIONS FOR BME280
void bme280_delay_us(uint32_t period, void *intf_ptr);
BME280_INTF_RET_TYPE bme280_i2c_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr);
BME280_INTF_RET_TYPE bme280_i2c_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr);


#ifdef __cplusplus
}
#endif

#endif
