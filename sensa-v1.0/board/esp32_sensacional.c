
//SENSACIONAL 2 BSP
#include "esp32_sensacional.h"

/**
 * @brief BSP version 1.0
 */
#define __ESP32E_SENSACIONAL_BSP_VERSION_MAIN (0x01) 
#define __ESP32E_SENSACIONAL_BSP_VERSION_SUB (0X00)
#define __ESP32E_SENSACIONAL_BSP_VERSION ((__ESP32E_SENSACIONAL_BSP_VERSION_MAIN << 8)|(__ESP32E_SENSACIONAL_BSP_VERSION_SUB))


static i2c_master_bus_handle_t I2C_handle;
static int i2c_status, mics_status, res_status;
extern i2c_master_dev_handle_t bme280_handle;
extern i2c_master_dev_handle_t sgp41_handle; //También se usa directamente en la hal de sensirion.
static const char *TAG_I2C = "ESP32-I2C";

static struct bme280_dev BME280;
static struct bme280_settings BME280_settings;
static uint32_t BME280_period;
static uint8_t BME_dev_addr; 
static const char *TAG_BME = "BME280";

static const char *TAG_SGP = "SGP41";

static const char *TAG_SPI = "MCP3204";
static spi_device_handle_t mcp3204_handle;

uint16_t BSP_GetVersion(void);



/*FUNCTION DECLARATION & DOC*/
/**
 * @brief Initialise GPIO pin from ESP32
 * 
 * @param pin Pin number
 * @param mode Pin direction (output)
 * @return * SENSA_RET_t Error code
 */
SENSA_RET_t BSP_GPIO_Init(uint8_t pin, int mode);

/**
 * @brief Set output of GPIO pin
 * 
 * @param pin Output GPIO pin number
 * @param value Logic level (1, 0)
 * @return SENSA_RET_t Error code
 */
SENSA_RET_t BSP_GPIO_Write(uint8_t pin, uint8_t value);

/*I2C bus functions*/

/**
 * @brief Initialise I2C1 bus 
 * 
 */
static void I2C_init(void);

/**
 * @brief Free I2C1 bus resources
 * 
 */
static void I2C_deinit(void);

/**
 * @brief Scan for addresses in the I2C1 bus
 * 
 * @return int Number of found devices
 */
static int I2C_scan(void);

/**
 * @brief Add 
 * 
 * @param device_handle Pointer to device structure
 * @param device_addr Device I2C address
 */
static void I2C_add_device(i2c_master_dev_handle_t *device_handle, uint8_t device_addr);

/**
 * @brief Add I2C device to the bus
 * 
 * @param device_handle Pointer to I2C device structure
 */
static void I2C_remove_device(i2c_master_dev_handle_t device_handle);

/**
 * @brief Helper function to configure BME280 structure upon initialisation
 * 
 * @param dev Pointer to BME280 device structure
 * @param address BME280 address
 * @return SENSA_RET_t Error code
 */
static SENSA_RET_t bme280_set_i2c(struct bme280_dev *dev, uint8_t address);


/**
 * @brief Initialise BME280 sensor
 * 
 * @return SENSA_RET_t Error code
 */
SENSA_RET_t BSP_BME280_Init(void);

/**
 * @brief Free resources associated to BME280 sensor
 * 
 * @return SENSA_RET_t Error code
 */
SENSA_RET_t BSP_BME280_Deinit(void);

/**
 * @brief Get temperature measurement from BME280
 * 
 * @param temp_ptr Pointer to store temperature value
 * @return SENSA_RET_t Error code
 */
SENSA_RET_t BSP_BME280_Get_Temperature(double *temp_ptr);

/**
 * @brief Get pressure measurement from BME280
 * 
 * @param pres_ptr Pointer to store pressure value
 * @return SENSA_RET_t Error code
 */
SENSA_RET_t BSP_BME280_Get_Pressure(double *pres_ptr);


/**
 * @brief Get relative humidity measurement from BME280
 * 
 * @param hum_ptr Pointer to store relative humidity value
 * @return SENSA_RET_t Error code
 */
SENSA_RET_t BSP_BME280_Get_Humidity(double *hum_ptr);

/**
 * @brief Initialise SGP41 sensor
 * 
 * @return SENSA_RET_t Error code
 */
SENSA_RET_t BSP_SGP41_Init(void);

/**
 * @brief Free resources associated to SGP41 sensor
 * 
 * @return SENSA_RET_t 
 */
SENSA_RET_t BSP_SGP41_Deinit(void);

/**
 * @brief Get VOC measurement from SGP41
 * 
 * @param relative_humidity Relative humidity value (callibration)
 * @param temperature Temperature value (callibration)
 * @param sraw_voc Pointer to store VOC measurement
 * @return SENSA_RET_t 
 */
SENSA_RET_t BSP_SGP41_Get_Raw(uint16_t relative_humidity, uint16_t temperature, uint16_t* sraw_voc);

/**
 * @brief Initialise ADC resources
 * 
 * @return SENSA_RET_t Error code
 */
static SENSA_RET_t BSP_ADC_Init(void);

/**
 * @brief Get ADC measurement
 * 
 * @param channel ADC channel to sample
 * @param meas_ptr Pointer to store ADC result
 * @return SENSA_RET_t Error code
 */
static SENSA_RET_t BSP_ADC_ReadChannel(uint8_t channel, uint16_t *meas_ptr);

/**
 * @brief Free SPI resources associated to ADC
 * 
 * @return SENSA_RET_t Error code
 */
static SENSA_RET_t BSP_ADC_Deinit(void);

/**
 * @brief Initialise MiCS-6814 sensor and turn heaters on
 * 
 * @return SENSA_RET_t Error code
 */
SENSA_RET_t BSP_MICS_Init(void);

/**
 * @brief Get MiCS-6814 NH3 sensor measurement
 * 
 * @param meas_ptr Pointer to store the measurement
 * @return SENSA_RET_t Error code
 */
SENSA_RET_t BSP_MICS_Get_NH3(uint16_t *meas_ptr);

/**
 * @brief Get MiCS-6814 CO sensor measurement
 * 
 * @param meas_ptr Pointer to store the measurement
 * @return SENSA_RET_t Error code
 */
SENSA_RET_t BSP_MICS_Get_CO(uint16_t *meas_ptr);

/**
 * @brief Get MiCS-6814 NO2 sensor measurement
 * 
 * @param meas_ptr Pointer to store the measurement
 * @return SENSA_RET_t Error code
 */
SENSA_RET_t BSP_MICS_Get_NO2(uint16_t *meas_ptr);

/**
 * @brief Free resources associated to MiCS-6814 sensor and turn heaters off
 * 
 * @return SENSA_RET_t Error code
 */
SENSA_RET_t BSP_MICS_Deinit(void);

/**
 * @brief Initialise resources associated to FLMG resistive sensor
 * 
 * @return SENSA_RET_t Error code
 */
SENSA_RET_t BSP_RES_Init(void);

/**
 * @brief Get FLMG Sensor raw measurement
 * 
 * @param meas_ptr Pointer to store the measurement
 * @return SENSA_RET_t Error code
 */
SENSA_RET_t BSP_RES_Get_Raw(uint16_t *meas_ptr);

/**
 * @brief Free resources associated to FLMG resistive sensor
 * 
 * @return SENSA_RET_t Error code
 */
SENSA_RET_t BSP_RES_Deinit(void);

/**
 * @brief Initialise communications with onboard FPGA
 * 
 * @return SENSA_RET_t Error code
 */
SENSA_RET_t BSP_SOE_Init(void);

/**
 * @brief Send command to onboard FPGA
 * 
 * @param command Command to send (3 bytes)
 * @param recv_ptr [Optional] pointer to store FPGA response
 * @return SENSA_RET_t Error code
 */
SENSA_RET_t BSP_SOE_Send_Command(uint8_t *command, uint8_t *recv_ptr);

/**
 * @brief Run and read counter
 * 
 * @param counter Counter number (1, 2)
 * @param count_ptr Pointer to store count result
 * @return SENSA_RET_t Error code
 */
SENSA_RET_t BSP_SOE_Get_Counter(uint8_t counter, uint32_t *count_ptr);

/**
 * @brief Set counter target
 * 
 * @param counter Counter to configure (1, 2)
 * @param target Amount of periods to count 
 * @return SENSA_RET_t Error code
 */
SENSA_RET_t BSP_SOE_Set_Target(uint8_t counter, uint8_t target);

/**
 * @brief Free resources associated to communications with onboard FPGA
 * 
 * @return SENSA_RET_t Error code
 */
SENSA_RET_t BSP_SOE_Deinit(void);


/*-------------------------------------------------------------*/

uint16_t BSP_GetVersion(void)
{
    return __ESP32E_SENSACIONAL_BSP_VERSION;
}

/*I2C GROUP*/ 
static void I2C_add_device(i2c_master_dev_handle_t *device_handle, uint8_t device_addr)
{
    i2c_device_config_t device_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = device_addr,
        .scl_speed_hz = I2C_MASTER_FREQ_HZ,
    };
    //rslt = i2c_master_bus_add_device(*bus_handle, &device_config, device_handle);
    //printf("Add device %X: %d\n", device_addr, rslt);
    ESP_ERROR_CHECK(i2c_master_bus_add_device(I2C_handle, &device_config, device_handle));
}

static void I2C_remove_device(i2c_master_dev_handle_t device_handle)
{
    ESP_ERROR_CHECK(i2c_master_bus_rm_device(device_handle));
}

static void I2C_init(void)
{
    i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_MASTER_NUM,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
         //Lo cambio porque he metido pullups externos ya.
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &I2C_handle));
    i2c_status = 1;
}

static int I2C_scan(void)
{
    int nDevices = 0;
    ESP_LOGI(TAG_I2C, "Run IIC bus dev scan...");

    for (uint16_t scan_addr = 1; scan_addr < 127; scan_addr++) {  
        if (i2c_master_probe(I2C_handle, scan_addr, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS) == ESP_OK){
            ESP_LOGI(TAG_I2C, "Found device at 0x%X\n", scan_addr);
            nDevices++;
        }
    }
    //ESP_LOGI(TAG_I2C, "%d", nDevices);
    return nDevices;
}

static void I2C_deinit(void){
    ESP_ERROR_CHECK(i2c_del_master_bus(I2C_handle));
    i2c_status = 0;
}

/*GPIO functions*/
SENSA_RET_t BSP_GPIO_Init(uint8_t pin, int mode)
{
    int8_t rslt;
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = mode;
    io_conf.pin_bit_mask = (1UL<<pin);
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    rslt = (int8_t) gpio_config(&io_conf);
    if (rslt != 0){
        return SENSA_ERR_OTHER;
    }
    return SENSA_OK;
}

SENSA_RET_t BSP_GPIO_Write(uint8_t pin, uint8_t value)
{
    int8_t rslt;
    rslt = (int8_t) gpio_set_level(pin, value);
    if (rslt != 0) return SENSA_ERR_OTHER;
    return SENSA_OK;
}


/*BME 280 GROUP*/ 
static SENSA_RET_t bme280_set_i2c(struct bme280_dev *dev, uint8_t address)
{
    
    BME_dev_addr = address;
    if (dev != NULL){ //He pasado un argumento
        dev->read = bme280_i2c_read;
        dev->write = bme280_i2c_write;
        dev->intf = BME280_I2C_INTF;
        dev->intf_ptr = &BME_dev_addr;
        dev->delay_us = bme280_delay_us;
        return SENSA_OK;
    } 
    else{
        return SENSA_ERR_NULL_PTR;
    }
    
}


SENSA_RET_t BSP_BME280_Init(void)
{
    int8_t rslt;
    uint8_t whoami;
    if (i2c_status == 0){
        I2C_init();
    }
    I2C_add_device(&bme280_handle, BME280_ADDR);
    rslt = bme280_set_i2c(&BME280, BME280_ADDR);
    if(rslt != BME280_OK){
        return SENSA_ERR_OTHER;
    }
    ESP_LOGI(TAG_BME, "Set i2c config: %d", rslt);

    //Init & check id
    rslt = bme280_init(&BME280);
    if(rslt != BME280_OK){
        return SENSA_ERR_OTHER;
    }
    ESP_LOGI(TAG_BME, "Init at address 0x%X: %d", BME280_ADDR, rslt);
    ESP_ERROR_CHECK(bme280_get_regs(0xD0, &whoami, 1, &BME280));
    ESP_LOGI(TAG_BME, "Who am I -> 0x%X \n", whoami);

    //Set default settings
    rslt = bme280_get_sensor_settings(&BME280_settings, &BME280);
    ESP_LOGI(TAG_BME, "Get last settings: %d", rslt);

    BME280_settings.filter = BME280_FILTER_COEFF_2;
    BME280_settings.osr_h = BME280_OVERSAMPLING_1X;
    BME280_settings.osr_p = BME280_OVERSAMPLING_1X;
    BME280_settings.osr_t = BME280_OVERSAMPLING_1X;
    BME280_settings.standby_time = BME280_STANDBY_TIME_0_5_MS;

    rslt = bme280_set_sensor_settings(BME280_SEL_ALL_SETTINGS, &BME280_settings, &BME280);
    if(rslt != BME280_OK){
        return SENSA_ERR_OTHER;
    }
    ESP_LOGI(TAG_BME, "Write default settings: %d", rslt);

    //Set power mode
    rslt = bme280_set_sensor_mode(BME280_POWERMODE_NORMAL, &BME280);
    if(rslt != BME280_OK){
        return SENSA_ERR_OTHER;
    }
    ESP_LOGI(TAG_BME, "Set normal power mode: %d", rslt);

    rslt = bme280_cal_meas_delay(&BME280_period, &BME280_settings);
    if(rslt != BME280_OK){
        return SENSA_ERR_OTHER;
    }

    return SENSA_OK;
}

SENSA_RET_t BSP_BME280_Deinit(void)
{
    I2C_remove_device(bme280_handle);
    memset(&BME280, 0, sizeof(BME280));
    memset(&BME280_settings, 0, sizeof(BME280_settings));
    BME280_period = 0;
    ESP_LOGI(TAG_BME, "Successful deinit");
    if(I2C_scan() == 0){
        I2C_deinit();
    }
    return SENSA_OK;
}

SENSA_RET_t BSP_BME280_Get_Temperature(double *temp_ptr)
{
    if (temp_ptr == NULL){
        return SENSA_ERR_NULL_PTR;
    }
    int8_t rslt = BME280_E_NULL_PTR;
    int8_t idx = 0;
    uint8_t status_reg;
    struct bme280_data comp_data;

    while (idx < 1)
    {
        rslt = bme280_get_regs(BME280_REG_STATUS, &status_reg, 1, &BME280);
        if(rslt != BME280_OK){
            return SENSA_ERR_OTHER;
        }
        if (status_reg & BME280_STATUS_MEAS_DONE)
        {
            /* Measurement time delay given to read sample */
            BME280.delay_us(BME280_period, BME280.intf_ptr);

            /* Read compensated data */
            rslt = bme280_get_sensor_data(BME280_TEMP, &comp_data, &BME280);
            if(rslt != BME280_OK){
                return SENSA_ERR_OTHER;
            }
           // ESP_LOGI(TAG, "bme280_get_sensor_data %d ", rslt);

#ifndef BME280_DOUBLE_ENABLE
            comp_data.temperature = comp_data.temperature / 100;
#endif
            idx++;
        }
    }
    *temp_ptr = comp_data.temperature;

    return SENSA_OK;
}

SENSA_RET_t BSP_BME280_Get_Pressure(double *pres_ptr)
{
    if (pres_ptr == NULL){
        return SENSA_ERR_NULL_PTR;
    }
    int8_t rslt = BME280_E_NULL_PTR;
    int8_t idx = 0;
    uint8_t status_reg;
    struct bme280_data comp_data;

    while (idx < 1)
    {
        rslt = bme280_get_regs(BME280_REG_STATUS, &status_reg, 1, &BME280);
        if(rslt != BME280_OK){
            return SENSA_ERR_OTHER;
        }

        if (status_reg & BME280_STATUS_MEAS_DONE)
        {
            /* Measurement time delay given to read sample */
            BME280.delay_us(BME280_period, BME280.intf_ptr);

            /* Read compensated data */
            rslt = bme280_get_sensor_data(BME280_PRESS, &comp_data, &BME280);
            if(rslt != BME280_OK){
                return SENSA_ERR_OTHER;
            }

#ifdef BME280_64BIT_ENABLE
            comp_data.pressure = comp_data.pressure / 100;
#endif
            idx++;
        }
    }
    *pres_ptr = comp_data.pressure;
    return SENSA_OK;
}

SENSA_RET_t BSP_BME280_Get_Humidity(double *hum_ptr)
{
    if (hum_ptr == NULL){
        return SENSA_ERR_NULL_PTR;
    }
    int8_t rslt = BME280_E_NULL_PTR;
    int8_t idx = 0;
    uint8_t status_reg;
    struct bme280_data comp_data;

    while (idx < 5)
    {
        rslt = bme280_get_regs(BME280_REG_STATUS, &status_reg, 1, &BME280);
        if(rslt != BME280_OK){
            return SENSA_ERR_OTHER;
        }

        if (status_reg & BME280_STATUS_MEAS_DONE)
        {
            /* Measurement time delay given to read sample */
            BME280.delay_us(BME280_period, BME280.intf_ptr);

            /* Read compensated data */
            rslt = bme280_get_sensor_data(BME280_HUM, &comp_data, &BME280);
            if(rslt != BME280_OK){
                return SENSA_ERR_OTHER;
            }

#ifndef BME280_DOUBLE_ENABLE
            comp_data.humidity = comp_data.humidity / 1000;
#endif
            idx++;
        }
    }
    *hum_ptr = comp_data.humidity;
    return SENSA_OK;
}

/*SGP41 group */
SENSA_RET_t BSP_SGP41_Init(void)
{
    I2C_add_device(&sgp41_handle, SGP41_ADDR);
    ESP_LOGI(TAG_SGP, "Successful init at address 0x%X ", SGP41_ADDR);
    return SENSA_OK;
}

SENSA_RET_t BSP_SGP41_Get_Raw(uint16_t relative_humidity, uint16_t temperature, uint16_t* sraw_voc) 
{
    if (sraw_voc == NULL){
        return SENSA_ERR_NULL_PTR;
    }
    int8_t ret;
    ret = 0;
    uint16_t *nox_buff = malloc(sizeof(uint16_t));
    if (nox_buff != NULL){
        ret = sgp41_measure_raw_signals(relative_humidity, temperature, sraw_voc, nox_buff);
        free(nox_buff);
        if (ret != 0){
            return SENSA_ERR_OTHER;
        }
    }   
    return SENSA_OK;
}

SENSA_RET_t BSP_SGP41_Deinit(void)
{
    I2C_remove_device(sgp41_handle);
    ESP_LOGI(TAG_SGP, "Successful deinit");
    if(I2C_scan() == 0){
        I2C_deinit();
    }
    return SENSA_OK;
}

/*Analog group - SPI MCP3204*/ 
static SENSA_RET_t BSP_ADC_Init(void)
{ //Rename to SPI init? 
    int8_t ret;

    spi_bus_config_t spi3_config = {
        .mosi_io_num = VSPI_MOSI, //change for mosi
        .miso_io_num = VSPI_MISO, 
        .sclk_io_num = VSPI_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
    };

    spi_device_interface_config_t mcp3204_spi_config = {
        .command_bits = 0,
        .address_bits = 0,
        .dummy_bits = 0,
        .clock_speed_hz = 100000,
        .duty_cycle_pos = 128,      //50% duty cycle
        .mode = 0,
        .spics_io_num = MCP_CS, 
        .queue_size = 1
  
    };
    
    
    ret = spi_bus_initialize(SPI3_HOST, &spi3_config, SPI_DMA_CH_AUTO);
    if (ret != 0){
        return SENSA_ERR_OTHER;
    }
    ret = spi_bus_add_device(SPI3_HOST, &mcp3204_spi_config, &mcp3204_handle);
    if (ret != 0){
        return SENSA_ERR_OTHER;
    }
    ESP_LOGI(TAG_SPI, "ADC Init");
    return SENSA_OK;
}

static SENSA_RET_t BSP_ADC_ReadChannel(uint8_t channel, uint16_t *meas_ptr)
{
    if (channel > 3){
        return SENSA_ERR_BAD_ARG;
    } 
    if (meas_ptr == NULL){
        return SENSA_ERR_NULL_PTR;
    }

    uint8_t send_buf[3] = {0x00, 0x00};
    uint8_t recv_buf[3];

    
    send_buf[0] = 0x06; //0000 0110
    send_buf[1] = (channel & 0x03) << 6;
    send_buf[2] = 0x00;

    spi_transaction_t trans = {
        .length = 24,
        .tx_buffer = send_buf,
        //.tx_buffer = NULL,
        .rx_buffer = recv_buf,
    };
    

    if (spi_device_transmit(mcp3204_handle, &trans) != 0) return SENSA_ERR_OTHER;

    
    uint16_t value;
    value = ((recv_buf[1] & 0x0F) << 8) | recv_buf[2];

    *meas_ptr = value;

    return SENSA_OK;
}

static SENSA_RET_t BSP_ADC_Deinit(void)
{
    SENSA_RET_t ret;
    ret = spi_bus_remove_device(mcp3204_handle);
    if (ret != 0){
        return SENSA_ERR_OTHER;
    }
    ret = spi_bus_free(SPI3_HOST);
    if (ret != 0){
        return SENSA_ERR_OTHER;
    }
    return SENSA_OK;
}

SENSA_RET_t BSP_MICS_Init(void){
    SENSA_RET_t ret;
    ret = BSP_ADC_Init();
    ret = BSP_GPIO_Init(MICS_EN_PIN, GPIO_MODE_OUTPUT);
    ret = BSP_GPIO_Write(MICS_EN_PIN, 0);
    mics_status = 1;
    return ret;
}

SENSA_RET_t BSP_MICS_Get_NH3(uint16_t *meas_ptr){
    if (meas_ptr == NULL){
        return SENSA_ERR_NULL_PTR;
    }
    
    SENSA_RET_t ret;
    uint16_t data;
    ret = BSP_ADC_ReadChannel(0, &data);
    *meas_ptr = data;
    return ret;
}

SENSA_RET_t BSP_MICS_Get_CO(uint16_t *meas_ptr){
    if (meas_ptr == NULL){
        return SENSA_ERR_NULL_PTR;
    }

    SENSA_RET_t ret;
    uint16_t data;
    ret = BSP_ADC_ReadChannel(1, &data);
    *meas_ptr = data;
    return ret;
}

SENSA_RET_t BSP_MICS_Get_NO2(uint16_t *meas_ptr){
    if (meas_ptr == NULL){
        return SENSA_ERR_NULL_PTR;
    }

    SENSA_RET_t ret;
    uint16_t data;
    ret = BSP_ADC_ReadChannel(2, &data);
    *meas_ptr = data;
    return ret;
}

SENSA_RET_t BSP_MICS_Deinit(void){
    SENSA_RET_t ret;
    if(!res_status) BSP_ADC_Deinit();
    mics_status = 0;
    ret = BSP_GPIO_Write(MICS_EN_PIN, 1);
    return ret;
}


SENSA_RET_t BSP_RES_Init(void){
    SENSA_RET_t ret;
    ret = BSP_ADC_Init();
    ret = BSP_GPIO_Init(RES_EN_PIN, GPIO_MODE_OUTPUT);
    
    res_status = 1;
    return ret;
}

SENSA_RET_t BSP_RES_Get_Raw(uint16_t *meas_ptr){
    SENSA_RET_t ret;
    uint16_t data;
    ret = BSP_GPIO_Write(RES_EN_PIN, 0);
    ret = BSP_ADC_ReadChannel(3, &data);
    *meas_ptr = data;
    ret = BSP_GPIO_Write(RES_EN_PIN, 1);
    return ret;
}

SENSA_RET_t BSP_RES_Deinit(void){
    SENSA_RET_t ret;
    if(!res_status) BSP_ADC_Deinit();
    res_status = 0;
    ret = BSP_GPIO_Write(RES_EN_PIN, 1);
    return ret;
}


/*UART GROUP*/
SENSA_RET_t BSP_SOE_Init(void)
{
    int8_t ret;
    /* Configure parameters of an UART driver,
     * communication pins and install the driver */
    uart_config_t uart_config = {
        .baud_rate = UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    
    ret = uart_driver_install(UART_NUM, RX_SIZE * 2, 0, 0, NULL, 0); //En teoría con un 4 bytes tengo
    if (ret != 0){
        return SENSA_ERR_OTHER;
    }
    ret = uart_param_config(UART_NUM, &uart_config);
    if (ret != 0){
        return SENSA_ERR_OTHER;
    }
    ret = uart_set_pin(UART_NUM, LATTICE_UART_TXD, LATTICE_UART_RXD, -1, -1);
    if (ret != 0){
        return SENSA_ERR_OTHER;
    }
    ret = BSP_GPIO_Init(SOE_EN_PIN, GPIO_MODE_OUTPUT);
    return ret;
}

SENSA_RET_t BSP_SOE_Send_Command(uint8_t *command, uint8_t *recv_ptr)
{   
    if (command == NULL){
        return SENSA_ERR_NULL_PTR;
    } 
    int len;
    uint8_t *data = (uint8_t *) malloc(RX_SIZE);
    len = uart_write_bytes(UART_NUM, (const char *) command, 3);
    if (len != 3){
        free(data);
        return SENSA_ERR_OTHER;
    }
    len = uart_read_bytes(UART_NUM, data, (RX_SIZE - 1), 20/portTICK_PERIOD_MS);
    if (len < 2){
        free(data);
        return SENSA_ERR_OTHER;
    }    
    //printf("%c%c\n", data[0], data[1]);
    if (data[0] == 0x45){
        free(data);
        return SENSA_ERR_FPGA;
    } 
    
    if(recv_ptr != NULL) memcpy(recv_ptr, data, len);
    free(data);
    return SENSA_OK;
}

SENSA_RET_t BSP_SOE_Get_Counter(uint8_t counter, uint32_t *count_ptr)
{
    SENSA_RET_t ret;
    //int len;
    ret = BSP_GPIO_Write(SOE_EN_PIN, 0); // Start the sensor
    uint32_t count;

    //ARM counter
    if(counter > 2 || counter == 0){
        BSP_GPIO_Write(SOE_EN_PIN, 1);
        return SENSA_ERR_BAD_ARG; //INVALID COUNTER
    }

    uint8_t arm_counters[3] = {0x43, 0x01, counter};
    ret = BSP_SOE_Send_Command(arm_counters, NULL);
    if (ret != SENSA_OK){
        BSP_GPIO_Write(SOE_EN_PIN, 1);
        return ret;
    }
    //START counter
    uint8_t start_counters[3] = {0x53, 0x30, 0x01};
    ret = BSP_SOE_Send_Command(start_counters, NULL);
    if (ret != SENSA_OK){
        BSP_GPIO_Write(SOE_EN_PIN, 1);
        return ret;
    }
    //READ result
    uint8_t *recv_ptr = (uint8_t *) malloc(RX_SIZE);
    uint8_t read_counter[3] = {0x52, 0x30, counter};
    ret = BSP_SOE_Send_Command(read_counter, recv_ptr);
    if (ret != 0){
        free(recv_ptr);
        BSP_GPIO_Write(SOE_EN_PIN, 1);
        return ret;
    }
    
    count = (uint32_t)recv_ptr[0] << 24 | (uint32_t)recv_ptr[1] << 16 | (uint32_t)recv_ptr[2] << 8 | (uint32_t)recv_ptr[3];
    //printf("%ld\n", count);
    *count_ptr = count;//*frq_ptr = 120e8 / count;
    free(recv_ptr);
    BSP_GPIO_Write(SOE_EN_PIN, 1);
    return SENSA_OK;
}

SENSA_RET_t BSP_SOE_Set_Target(uint8_t counter, uint8_t target){
    //SENSA_RET_t ret;
    if(counter > 2 || counter == 0){
        return SENSA_ERR_BAD_ARG; //INVALID COUNTER
    }
    if(target < 1){ //No comparo con 255 porque es uint8_t
        return SENSA_ERR_BAD_ARG;
    }
    uint8_t command[3] = {0x43, counter + 1, target};
    return BSP_SOE_Send_Command(command, NULL);
}

SENSA_RET_t BSP_SOE_Deinit(void){
    if(uart_driver_delete(UART_NUM) != 0){
        return SENSA_ERR_OTHER;
    }
    
    ESP_LOGI(TAG_SGP, "Successful deinit");
    return SENSA_OK;
}


