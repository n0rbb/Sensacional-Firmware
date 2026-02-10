#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"


#include "esp32_sensacional.h"
#include "mqtt.h"


//static const char *TAG = "ESP32";
extern uint8_t MQTT_CONNECTED;


extern uint8_t TEM_FLAG, PRE_FLAG, HUM_FLAG, COV_FLAG, RES_FLAG, SOE_FLAG, NO2_FLAG, CO_FLAG, NH3_FLAG;

void app_main(void)
{   
    double temperature, pressure, humidity;
    uint16_t tvoc;
    uint16_t analog_vals[4];
    uint32_t count;
    double frequency = 0;
 
    
    BSP_BME280_Init();
    BSP_SGP41_Init();

    BSP_MICS_Init();
    BSP_RES_Init();
    BSP_SOE_Init();
    mqtt_app_start();
    
    for(;;){

        if (TEM_FLAG) BSP_BME280_Get_Temperature(&temperature);
        if (PRE_FLAG) BSP_BME280_Get_Pressure(&pressure);
        if (HUM_FLAG) BSP_BME280_Get_Humidity(&humidity);
        if (COV_FLAG) BSP_SGP41_Get_Raw((uint16_t) 27, (uint16_t) 50, &tvoc);
        if (NH3_FLAG) BSP_MICS_Get_NH3 (&analog_vals[0]);
        if (CO_FLAG) BSP_MICS_Get_CO(&analog_vals[1]);
        if (NO2_FLAG) BSP_MICS_Get_NO2(&analog_vals[2]);
        if (RES_FLAG) BSP_RES_Get_Raw(&analog_vals[3]);

        if (SOE_FLAG) BSP_SOE_Get_Counter(1, &count);
        frequency = 120e8/count; //Valid for 100 periods

        printf("Temperatura: %.2f\n", temperature);
        printf("Presión: %.2f\n", pressure);
        printf("Humedad: %.2f\n", humidity);
        printf("SGP40: %d\n", tvoc);
        printf("NH3: %d\n", analog_vals[0]);
        printf("CO: %d\n", analog_vals[1]);
        printf("NO2: %d\n", analog_vals[2]);
        printf("RES: %d\n", analog_vals[3]);
        printf("SOE: %.2f\n", frequency);
        
        
        if (MQTT_CONNECTED){
            if (TEM_FLAG) mqtt_publish("Temperatura", &temperature, TYPE_DOUBLE);
            if (PRE_FLAG) mqtt_publish("Presion", &pressure, TYPE_DOUBLE);
            if (HUM_FLAG) mqtt_publish("Humedad", &humidity, TYPE_DOUBLE);
            if (COV_FLAG) mqtt_publish("SGP40", &tvoc, TYPE_UINT16); 
            if (NH3_FLAG) mqtt_publish("NH3", &analog_vals[0], TYPE_UINT16);
            if (CO_FLAG) mqtt_publish("CO", &analog_vals[1], TYPE_UINT16);
            if (NO2_FLAG) mqtt_publish("NO2", &analog_vals[2], TYPE_UINT16);
            if (RES_FLAG) mqtt_publish("RES", &analog_vals[3], TYPE_UINT16);
            if (SOE_FLAG) mqtt_publish("SOE", &frequency, TYPE_DOUBLE);
        }
     
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }
    BSP_BME280_Deinit();
    BSP_SGP41_Deinit();
    BSP_MICS_Deinit();
    BSP_RES_Deinit();
    BSP_SOE_Deinit();
}
