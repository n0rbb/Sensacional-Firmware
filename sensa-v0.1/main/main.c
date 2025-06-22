#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"


#include "esp32s2_sensacional.h"
#include "mqtt.h"


static const char *TAG = "ESP32";
extern uint8_t MQTT_CONNECTED;


extern uint8_t LED_FLAG, TEM_FLAG, PRE_FLAG, HUM_FLAG, COV_FLAG;

void app_main(void)
{   
    double temperature, pressure, humidity;
    //char[sizeof(double)] str_temperature, str_pressure, str_humidity;
    uint16_t tvoc;
    //char[sizeof(uint16_t)] str_tvoc;
    printf("BSP Version %d\n", BSP_GetVersion());

    I2C_init();
    ESP_LOGI(TAG, "IIC initialised successfully");
    I2C_scan();
    
    BSP_LED_Init(LED_PIN);
    BSP_BME280_Init();
    BSP_SGP40_Init();
    
    //mqtt_app_start();
    
    for(;;){
        BSP_LED_Write(LED_PIN, LED_FLAG);
        if (1) BSP_BME280_Get_Temperature(&temperature);
        if (1) BSP_BME280_Get_Pressure(&pressure);
        if (1) BSP_BME280_Get_Humidity(&humidity);
        if (1) BSP_SGP40_Get_Raw((uint16_t) 27, (uint16_t) 50, &tvoc);
        printf("Temperatura: %.2f\n", temperature);
        printf("Presión: %.2f\n", pressure);
        printf("Humedad: %.2f\n", humidity);
        printf("COVs: %d\n", tvoc);
        
        
        if (MQTT_CONNECTED){
            if (TEM_FLAG) mqtt_publish("Temperatura", &temperature, TYPE_DOUBLE);
            if (PRE_FLAG) mqtt_publish("Presion", &pressure, TYPE_DOUBLE);
            if (HUM_FLAG) mqtt_publish("Humedad", &humidity, TYPE_DOUBLE);
            if (COV_FLAG) mqtt_publish("COVs", &tvoc, TYPE_UINT16); 
        }
        
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }

    BSP_SGP40_Deinit();
    BSP_BME280_Deinit();
    I2C_deinit();
    ESP_LOGI(TAG, "IIC de-initialised successfully");
}
