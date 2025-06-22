
//SENSATIONAL v0.1 BSP


#include "esp32s2_sensacional.h"

/**
 * @brief BSP version 0.2 
 */
#define __ESP32E_SENSATIONAL_BSP_VERSION_MAIN (0x00) 
#define __ESP32E_SENSATIONAL_BSP_VERSION_SUB (0X02)
#define __ESP32E_SENSATIONAL_BSP_VERSION ((__ESP32E_SENSATIONAL_BSP_VERSION_MAIN << 8)\
                                        |(__ESP32E_SENSATIONAL_BSP_VERSION_SUB))



static i2c_master_bus_handle_t I2C_handle;
static i2c_master_dev_handle_t bme280_handle;
extern i2c_master_dev_handle_t sgp40_handle; //También se usa directamente en la hal de sensirion.
static const char *TAG_I2C = "ESP32-I2C";

static struct bme280_dev BME280;
static struct bme280_settings BME280_settings;
static uint32_t BME280_period;
static uint8_t BME_dev_addr; 
static const char *TAG_BME = "BME280";

static const char *TAG_SGP = "SGP40";

uint16_t BSP_GetVersion(void);


/*I2C bus functions*/
void I2C_init(void);
void I2C_deinit(void);
void I2C_scan(void);
static void I2C_add_device(i2c_master_dev_handle_t *device_handle, uint8_t device_addr);
static void I2C_remove_device(i2c_master_dev_handle_t device_handle);
static int8_t I2C_send(i2c_master_dev_handle_t device_handle, const uint8_t *write_buffer, int xfer_timeout_ms);
//static int8_t I2C_receive();
static int8_t I2C_send_and_receive(i2c_master_dev_handle_t device_handle, uint8_t *write_buffer, size_t write_size, 
                                    uint8_t *read_buffer, size_t read_size, int xfer_timeout_ms);
//static void I2C_error(void);

/*LED functions*/
void BSP_LED_Init(uint8_t pin);
void BSP_LED_Write(uint8_t pin, uint8_t value);


/*BME 280 functions*/
static void bme280_delay_us(uint32_t period, void *intf_ptr);
static BME280_INTF_RET_TYPE bme280_i2c_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr);
static BME280_INTF_RET_TYPE bme280_i2c_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr);
static int8_t bme280_set_i2c(struct bme280_dev *dev, uint8_t address);

void BSP_BME280_Init(void);
void BSP_BME280_Deinit(void);
void BSP_BME280_Get_Temperature(double *temp_ptr);
void BSP_BME280_Get_Pressure(double *pres_ptr);
void BSP_BME280_Get_Humidity(double *hum_ptr);

void BSP_SGP40_Init(void);
void BSP_SGP40_Deinit(void);
void BSP_SGP40_Get_Raw(uint16_t relative_humidity, uint16_t temperature, uint16_t* sraw_voc);


uint16_t BSP_GetVersion(void)
{
    return __ESP32E_SENSATIONAL_BSP_VERSION;
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

static int8_t I2C_send(i2c_master_dev_handle_t device_handle, const uint8_t *write_buffer, int xfer_timeout_ms)
{
    return (int8_t) i2c_master_transmit(device_handle, write_buffer, sizeof(write_buffer), xfer_timeout_ms);
}

static int8_t I2C_send_and_receive(i2c_master_dev_handle_t device_handle, uint8_t *write_buffer, size_t write_size, uint8_t *read_buffer, size_t read_size, int xfer_timeout_ms)
{
    return (int8_t) i2c_master_transmit_receive(device_handle, write_buffer, write_size, read_buffer, read_size, xfer_timeout_ms);
}


void I2C_init(void)
{
    i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_MASTER_NUM,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = false,
         //Lo cambio porque he metido pullups externos ya.
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &I2C_handle));
}

void I2C_deinit(void)
{
    ESP_ERROR_CHECK(i2c_del_master_bus(I2C_handle));
}

void I2C_scan(void)
{
    ESP_LOGI(TAG_I2C, "Run IIC bus dev scan...");
    for (uint16_t scan_addr = 1; scan_addr < 127; scan_addr++) {  
        if (i2c_master_probe(I2C_handle, scan_addr, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS) == ESP_OK){
            ESP_LOGI(TAG_I2C, "Found device at 0x%X\n", scan_addr);
        }
    }
}


/*UART Functions*/

void BSP_UART_Init(){
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 122,
    };
    ESP_ERROR_CHECK(uart_param_config(UART_NUM, &uart_config));
}

void BSP_UART_Send(char* command){
    uart_write_bytes(UART_NUM, (const char*)command, strlen(command));
}

int8_t BSP_UART_Read(uint8_t* buff_data){
    int buff_len;
    int max_len = sizeof(&buff_data);
    uart_get_buffered_data_len(UART_NUM, (size_t*)&buff_len);
    if (buff_len > max_len) buff_len = max_len;
    //uint8_t buff_data[buff_len];
    return uart_read_bytes(UART_NUM, buff_data, buff_len, 100);
}



/*Onboard LED functions*/
void BSP_LED_Init(uint8_t pin){
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1UL<<pin);
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);
}

void BSP_LED_Write(uint8_t pin, uint8_t value){
    gpio_set_level(pin, value);

}
/*BME 280 GROUP*/
static void bme280_delay_us(uint32_t period, void *intf_ptr)
{
    ets_delay_us(period);
}

static BME280_INTF_RET_TYPE bme280_i2c_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
    int8_t rslt;
    //BME_dev_addr= *(uint8_t*)intf_ptr;
    rslt = I2C_send_and_receive(bme280_handle, &reg_addr, 1, reg_data, (size_t)len, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS); 
    return rslt;
}

static BME280_INTF_RET_TYPE bme280_i2c_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
    int8_t rslt;
    uint8_t write_buf[len + 1];
    //BME_dev_addr = *(uint8_t*)intf_ptr;
    write_buf[0] = reg_addr;
    memcpy(&write_buf[1], reg_data, len);
    rslt = I2C_send(bme280_handle, write_buf, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
    return rslt;
}

static int8_t bme280_set_i2c(struct bme280_dev *dev, uint8_t address)
{
    int8_t rslt = BME280_OK;
    BME_dev_addr = address;
    if (dev != NULL){ //He pasado un argumento
        dev->read = bme280_i2c_read;
        dev->write = bme280_i2c_write;
        dev->intf = BME280_I2C_INTF;
        dev->intf_ptr = &BME_dev_addr;
        dev->delay_us = bme280_delay_us;

    } 
    else{
        rslt = BME280_E_NULL_PTR;
    }
    return rslt;
}


void BSP_BME280_Init(void)
{
    int8_t rslt;
    uint8_t whoami;
    I2C_add_device(&bme280_handle, BME280_ADDR);
    rslt = bme280_set_i2c(&BME280, BME280_ADDR);
    if(rslt != BME280_OK){
        return;
    }
    ESP_LOGI(TAG_BME, "Set i2c config: %d", rslt);

    //Init & check id
    rslt = bme280_init(&BME280);
    if(rslt != BME280_OK){
        return;
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
        return;
    }
    ESP_LOGI(TAG_BME, "Write default settings: %d", rslt);

    //Set power mode
    rslt = bme280_set_sensor_mode(BME280_POWERMODE_NORMAL, &BME280);
    if(rslt != BME280_OK){
        return;
    }
    ESP_LOGI(TAG_BME, "Set normal power mode: %d", rslt);

    rslt = bme280_cal_meas_delay(&BME280_period, &BME280_settings);
    if(rslt != BME280_OK){
        return;
    }
}

void BSP_BME280_Deinit(void)
{
    I2C_remove_device(bme280_handle);
    memset(&BME280, 0, sizeof(BME280));
    memset(&BME280_settings, 0, sizeof(BME280_settings));
    BME280_period = 0;
    ESP_LOGI(TAG_BME, "Successful deinit");
}

void BSP_BME280_Get_Temperature(double *temp_ptr)
{
    int8_t rslt = BME280_E_NULL_PTR;
    int8_t idx = 0;
    uint8_t status_reg;
    struct bme280_data comp_data;

    while (idx < 1)
    {
        rslt = bme280_get_regs(BME280_REG_STATUS, &status_reg, 1, &BME280);
        if(rslt != BME280_OK){
            return;
        }
        if (status_reg & BME280_STATUS_MEAS_DONE)
        {
            /* Measurement time delay given to read sample */
            BME280.delay_us(BME280_period, BME280.intf_ptr);

            /* Read compensated data */
            rslt = bme280_get_sensor_data(BME280_TEMP, &comp_data, &BME280);
            if(rslt != BME280_OK){
                return;
            }
           // ESP_LOGI(TAG, "bme280_get_sensor_data %d ", rslt);

#ifndef BME280_DOUBLE_ENABLE
            comp_data.temperature = comp_data.temperature / 100;
#endif
            idx++;
        }
    }
    *temp_ptr = comp_data.temperature;
}

void BSP_BME280_Get_Pressure(double *pres_ptr)
{
    int8_t rslt = BME280_E_NULL_PTR;
    int8_t idx = 0;
    uint8_t status_reg;
    struct bme280_data comp_data;

    while (idx < 1)
    {
        rslt = bme280_get_regs(BME280_REG_STATUS, &status_reg, 1, &BME280);
        if(rslt != BME280_OK){
            return;
        }

        if (status_reg & BME280_STATUS_MEAS_DONE)
        {
            /* Measurement time delay given to read sample */
            BME280.delay_us(BME280_period, BME280.intf_ptr);

            /* Read compensated data */
            rslt = bme280_get_sensor_data(BME280_PRESS, &comp_data, &BME280);
            if(rslt != BME280_OK){
                return;
            }

#ifdef BME280_64BIT_ENABLE
            comp_data.pressure = comp_data.pressure / 100;
#endif
            idx++;
        }
    }
    *pres_ptr = comp_data.pressure;
}

void BSP_BME280_Get_Humidity(double *hum_ptr)
{
    int8_t rslt = BME280_E_NULL_PTR;
    int8_t idx = 0;
    uint8_t status_reg;
    struct bme280_data comp_data;

    while (idx < 5)
    {
        rslt = bme280_get_regs(BME280_REG_STATUS, &status_reg, 1, &BME280);
        if(rslt != BME280_OK){
            return;
        }

        if (status_reg & BME280_STATUS_MEAS_DONE)
        {
            /* Measurement time delay given to read sample */
            BME280.delay_us(BME280_period, BME280.intf_ptr);

            /* Read compensated data */
            rslt = bme280_get_sensor_data(BME280_HUM, &comp_data, &BME280);
            if(rslt != BME280_OK){
                return;
            }

#ifndef BME280_DOUBLE_ENABLE
            comp_data.humidity = comp_data.humidity / 1000;
#endif
            idx++;
        }
    }
    *hum_ptr = comp_data.humidity;
}


/*SGP40 group */
void BSP_SGP40_Init(void)
{
    I2C_add_device(&sgp40_handle, SGP40_ADDR);
    ESP_LOGI(TAG_SGP, "Successful init at address 0x%X ", SGP40_ADDR);
}

void BSP_SGP40_Deinit(void)
{
    I2C_remove_device(sgp40_handle);
    ESP_LOGI(TAG_SGP, "Successful deinit");
}

void BSP_SGP40_Get_Raw(uint16_t relative_humidity, uint16_t temperature, uint16_t* sraw_voc) 
{
    return (void) sgp40_measure_raw_signal(relative_humidity, temperature, sraw_voc);
}
