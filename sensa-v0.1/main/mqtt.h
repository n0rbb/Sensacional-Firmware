#ifndef __MQTT_H
#define __MQTT_H

#ifdef __cplusplus
extern "C" {
#endif

#define BROKER_URL "mqtt://192.168.0.103" //IMA goes 192.168.226.102
#define BROKER_PORT 1885
#define MAX_N_CHARS 100
#define SENSOR_ID "sen2"

typedef enum {
    TYPE_DOUBLE,
    TYPE_UINT16,
    TYPE_LOG
} data_type_t;



void mqtt_app_start(void);
void mqtt_publish(char *quantity, void *value_ptr, data_type_t value_type);

#ifdef __cplusplus
}
#endif

#endif