#ifndef _ESP32_SENSACIONAL_H
#define _ESP32_SENSACIONAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <string.h>

#include "soc/soc_caps.h"

#include "driver/i2c_master.h"
#include "driver/gpio.h"
#include "driver/uart.h"

//#include "esp_adc/adc_oneshot.h"
//#include "esp_adc/adc_cali.h"
//#include "esp_adc/adc_cali_scheme.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "rom/ets_sys.h"
#include "bme280.h"
#include "wrapper_bme280.h"
//#include "sensirion_config.h"
#include "SGP41_i2c.h"

#include "driver/spi_common.h"
#include "driver/spi_master.h"


#define I2C_MASTER_SCL_IO           22 //CONFIG_I2C_MASTER_SCL  
#define I2C_MASTER_SDA_IO           21 //CONFIG_I2C_MASTER_SDA  
    
#define I2C_MASTER_NUM              I2C_NUM_0                   /*!< I2C port number for master dev */
#define I2C_MASTER_FREQ_HZ          CONFIG_I2C_MASTER_FREQUENCY /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE   0                           /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE   0                           /*!< I2C master doesn't need buffer */
#define I2C_MASTER_TIMEOUT_MS       1000

#define BME280_ADDR                 BME280_I2C_ADDR_PRIM //Change back to PRIM for real board
#define SGP41_ADDR                  0x59

#define MICS_EN_PIN                 25
#define SOE_EN_PIN                  26
#define RES_EN_PIN                  27

#define VSPI_MOSI                   23
#define VSPI_MISO                   19
#define VSPI_SCLK                   18
#define MCP_CS                      5 //back to 5 on real sensa

#define UART_NUM                    UART_NUM_2
#define UART_BAUD_RATE              115200
#define LATTICE_UART_TXD            16
#define LATTICE_UART_RXD            17
#define RX_SIZE                     1024

//#define SENSA_RET_t                 int8_t

typedef enum{
    SENSA_OK = 0,
    SENSA_ERR_BAD_ARG = -1,
    SENSA_ERR_NULL_PTR = -2,
    SENSA_ERR_FPGA = -3,
    SENSA_ERR_OTHER = -4,
} SENSA_RET_t;


uint16_t BSP_GetVersion(void);

SENSA_RET_t BSP_GPIO_Init(uint8_t pin, int mode);
SENSA_RET_t BSP_GPIO_Write(uint8_t pin, uint8_t value);

SENSA_RET_t BSP_BME280_Init(void);
SENSA_RET_t BSP_BME280_Deinit(void);
SENSA_RET_t BSP_BME280_Get_Temperature(double *temp_ptr);
SENSA_RET_t BSP_BME280_Get_Pressure(double *pres_ptr);
SENSA_RET_t BSP_BME280_Get_Humidity(double *hum_ptr);

SENSA_RET_t BSP_SGP41_Init(void);
SENSA_RET_t BSP_SGP41_Deinit(void);
SENSA_RET_t BSP_SGP41_Get_Raw(uint16_t relative_humidity,
    uint16_t temperature, uint16_t* sraw_voc);

SENSA_RET_t BSP_MICS_Init(void);
SENSA_RET_t BSP_MICS_Deinit(void);
SENSA_RET_t BSP_MICS_Get_NH3(uint16_t *meas_ptr);
SENSA_RET_t BSP_MICS_Get_CO(uint16_t *meas_ptr);
SENSA_RET_t BSP_MICS_Get_NO2(uint16_t *meas_ptr);


SENSA_RET_t BSP_RES_Init(void);
SENSA_RET_t BSP_RES_Deinit(void);
SENSA_RET_t BSP_RES_Get_Raw(uint16_t *meas_ptr);


SENSA_RET_t BSP_SOE_Init(void);
SENSA_RET_t BSP_SOE_Deinit();
SENSA_RET_t BSP_SOE_Send_Command(uint8_t *command, uint8_t *recv_ptr);
SENSA_RET_t BSP_SOE_Get_Counter(uint8_t counter, uint32_t *count_ptr);
SENSA_RET_t BSP_SOE_Set_Target(uint8_t counter, uint8_t target);




#ifdef __cplusplus
}
#endif

#endif