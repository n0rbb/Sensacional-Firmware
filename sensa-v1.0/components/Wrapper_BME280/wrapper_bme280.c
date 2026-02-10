
#include "wrapper_bme280.h"

i2c_master_dev_handle_t bme280_handle;
//static uint8_t BME_dev_addr; 

void bme280_delay_us(uint32_t period, void *intf_ptr)
{
    ets_delay_us(period);
}

BME280_INTF_RET_TYPE bme280_i2c_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
    int8_t rslt;
    //BME_dev_addr= *(uint8_t*)intf_ptr;
    rslt = (int8_t) i2c_master_transmit_receive(bme280_handle, &reg_addr, 1, reg_data, (size_t) len, 1000 / portTICK_PERIOD_MS); 
    return rslt;
}

BME280_INTF_RET_TYPE bme280_i2c_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
    int8_t rslt;
    uint8_t write_buf[len + 1];
    //BME_dev_addr = *(uint8_t*)intf_ptr;
    write_buf[0] = reg_addr;
    memcpy(&write_buf[1], reg_data, len);
    rslt = i2c_master_transmit(bme280_handle, write_buf, sizeof(write_buf), 1000 / portTICK_PERIOD_MS);
    return rslt;
}
