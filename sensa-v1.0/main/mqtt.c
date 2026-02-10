#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"

#include "mqtt_client.h"
#include "mqtt.h"
#include "esp_log.h"

static char command_channel[MAX_N_CHARS];
uint8_t MQTT_CONNECTED = 0;
static esp_mqtt_client_handle_t mqtt_client = NULL;
static char *TAG_MQTT = "ESP32-MQTT";

//commands
typedef enum{
    NOP,
    LED,
    //LON,
    //LOFF, 
    TEM,
    PRE,
    HUM,
    COV,
    RES, 
    SOE,
    NO2,
    CO,
    NH3, 
} command_type_t;


typedef struct {
    command_type_t command;
    const char *command_str;
} command_str_t; 

//command flags
uint8_t LED_FLAG, TEM_FLAG, PRE_FLAG, HUM_FLAG, COV_FLAG, RES_FLAG, SOE_FLAG, NO2_FLAG, CO_FLAG, NH3_FLAG; //añadir mics

command_str_t command_parser[] = {
    {NOP, "N"},
    //{LED, "L"},
    //{LON, "LON"},
    //{LOFF, "LOFF"},
    {TEM, "TEM"},
    {PRE, "PRE"},
    {HUM, "HUM"},
    {COV, "COV"},
    {RES, "RES"}, 
    {SOE, "SOE"},
    {NO2, "NO2"}, 
    {CO,  "CO"},
    {NH3, "NH3"},
    
};

static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0)
    {
        ESP_LOGE(TAG_MQTT, "Last error %s: 0x%x", message, error_code);
    }
}

static void mqtt_command_handler(const char* cmd_char){
    command_type_t cmd;
    int i;
    for (i = 0;  i < sizeof (command_parser) / sizeof (command_parser[0]);  i++){
        if (!strcmp (cmd_char, command_parser[i].command_str)){
            cmd = command_parser[i].command;
            break; 
        }
        if ( i >= sizeof (command_parser) / sizeof (command_parser[0]) - 1){
            cmd = NOP;
        }
    }
    switch (cmd){
        case NOP:
            //printf("Hola mundo\r\n");
            ESP_LOGE(TAG_MQTT, "Unknown command");
            mqtt_publish("LOG", "Comando no reconocido", TYPE_LOG);
            break;

        case LED:
            LED_FLAG = ~LED_FLAG;
            if(LED_FLAG){
                mqtt_publish("LOG", "LED encendido", TYPE_LOG);
            }
            else{
                mqtt_publish("LOG", "LED apagado", TYPE_LOG);
            }
            break;
        /*
        case LON:
            LED_FLAG = 1;
            mqtt_publish("LOG", "LED encendido", TYPE_LOG);
            break;
        case LOFF:
            LED_FLAG = 0;
            mqtt_publish("LOG", "LED apagado", TYPE_LOG);
            break;
        */
        case TEM:
            TEM_FLAG = ~TEM_FLAG;
            if (TEM_FLAG){
                mqtt_publish("LOG", "Medida de temperatura iniciada", TYPE_LOG);
            }
            else{
                mqtt_publish("LOG", "Medida de temperatura parada", TYPE_LOG);
            }
            break;
        
        case PRE:
            PRE_FLAG = ~PRE_FLAG;
            if (PRE_FLAG){
                mqtt_publish("LOG", "Medida de presión iniciada", TYPE_LOG);
            }
            else{
                mqtt_publish("LOG", "Medida de presión parada", TYPE_LOG);
            }
            break;

        case HUM:
            HUM_FLAG = ~HUM_FLAG;
            if (HUM_FLAG){
                mqtt_publish("LOG", "Medida de humedad iniciada", TYPE_LOG);
            }
            else{
                mqtt_publish("LOG", "Medida de humedad parada", TYPE_LOG);
            }
            break;
            
        case COV:
            COV_FLAG = ~COV_FLAG;
            if (COV_FLAG){
                mqtt_publish("LOG", "Medida de COV iniciada", TYPE_LOG);
            }
            else{
                mqtt_publish("LOG", "Medida de COV parada", TYPE_LOG);
            }
            break;

        case RES:
            RES_FLAG = ~RES_FLAG;
            if (RES_FLAG){
                mqtt_publish("LOG", "Medida de sensor resistivo iniciada", TYPE_LOG);
            }
            else{
                mqtt_publish("LOG", "Medida de sensor resistivo parada", TYPE_LOG);
            }
            break;
        
        case SOE:
            SOE_FLAG = ~SOE_FLAG;
            if (SOE_FLAG){
                mqtt_publish("LOG", "Medida de sensor de ondas de espín iniciada", TYPE_LOG);
            }
            else{
                mqtt_publish("LOG", "Medida de sensor de ondas de espín parada", TYPE_LOG);
            }
            break;

        case NO2:
            NO2_FLAG = ~NO2_FLAG;
            if (NO2_FLAG){
                mqtt_publish("LOG", "Medida de sensor MICS de NO2 iniciada", TYPE_LOG);
            }
            else{
                mqtt_publish("LOG", "Medida de sensor MICS de NO2 parada", TYPE_LOG);
            }
            break;
        
        case CO:
            CO_FLAG = ~CO_FLAG;
            if (CO_FLAG){
                mqtt_publish("LOG", "Medida de sensor MICS de CO iniciada", TYPE_LOG);
            }
            else{
                mqtt_publish("LOG", "Medida de sensor MICS de CO parada", TYPE_LOG);
            }
            break;

        case NH3:
            NH3_FLAG = ~NH3_FLAG;
            if (NH3_FLAG){
                mqtt_publish("LOG", "Medida de sensor MICS de NH3 iniciada", TYPE_LOG);
            }
            else{
                mqtt_publish("LOG", "Medida de sensor MICS de NH3 espín parada", TYPE_LOG);
            }
            break;

        default:
            return;
    }
}


static void MessageFunction(void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;
    char msg_data[MAX_N_CHARS];
    char msg_topic[MAX_N_CHARS];
    
   
    sprintf(msg_topic, "%.*s", event->topic_len, event->topic);
    sprintf(msg_data, "%.*s", event->data_len, event->data);

    printf("TOPIC=%s\r\n", msg_topic);
    if (!strcmp(msg_topic, command_channel)){
        printf("COMANDO = %s\r\n", msg_data);
        //llamada a función de parsear comando
        mqtt_command_handler(msg_data);
    } 
}




static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG_MQTT, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id = 0;
    switch ((esp_mqtt_event_id_t)event_id)
    {
    case MQTT_EVENT_CONNECTED:
        MQTT_CONNECTED = 1;
        ESP_LOGI(TAG_MQTT, "MQTT_EVENT_CONNECTED");
        snprintf(command_channel, sizeof(command_channel), "%s%s", "IMA/Comandos/", SENSOR_ID);
        msg_id = esp_mqtt_client_subscribe(client, command_channel, 1);
        ESP_LOGI(TAG_MQTT, "sent subscribe successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_DISCONNECTED:
        MQTT_CONNECTED = 0;
        ESP_LOGI(TAG_MQTT, "MQTT_EVENT_DISCONNECTED");
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG_MQTT, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        ESP_LOGI(TAG_MQTT, "sent publish successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG_MQTT, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG_MQTT, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG_MQTT, "MQTT_EVENT_DATA");
        MessageFunction(event_data);
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG_MQTT, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT)
        {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno", event->error_handle->esp_transport_sock_errno);
            ESP_LOGI(TAG_MQTT, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
        }
        break;
    default:
        ESP_LOGI(TAG_MQTT, "Other event id:%d", event->event_id);
        break;
    }
}

static void mqtt_helper_init(void){
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());
}

void mqtt_app_start(void)
{
    mqtt_helper_init();

    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = BROKER_URL,
        .broker.address.port = BROKER_PORT
    };

#if CONFIG_BROKER_URL_FROM_STDIN
    char line[128];

    if (strcmp(mqtt_cfg.broker.address.uri, "FROM_STDIN") == 0)
    {
        int count = 0;
        printf("Please enter url of mqtt broker\n");
        while (count < 128)
        {
            int c = fgetc(stdin);
            if (c == '\n')
            {
                line[count] = '\0';
                break;
            }
            else if (c > 0 && c < 127)
            {
                line[count] = c;
                ++count;
            }
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
        mqtt_cfg.broker.address.uri = line;
        printf("Broker url: %s\n", line);
    }
    else
    {
        ESP_LOGE(TAG_MQTT, "Configuration mismatch: wrong broker url");
        abort();
    }
#endif /* CONFIG_BROKER_URL_FROM_STDIN */

    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
   
    //mqtt_client = client;
    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(mqtt_client);
    
}

void mqtt_publish(char *quantity, void *value_ptr, data_type_t type)
{
    char topic[MAX_N_CHARS];
    char payload[MAX_N_CHARS];
    int msg_id = 0;
    sprintf(topic, "IMA/Sensores/%s/%s", SENSOR_ID, quantity);

    switch (type){
        case TYPE_DOUBLE:
            sprintf(payload, "%.2f", *(double *)value_ptr);
            break;
        case TYPE_UINT16:
            sprintf(payload, "%d", *(uint16_t *)value_ptr);
            break;
        case TYPE_INT:
            sprintf(payload, "%d", *(int *)value_ptr);
            break;
        case TYPE_LOG:
            sprintf(payload, "%s", (char *)value_ptr);
            break;
        default:
            ESP_LOGE(TAG_MQTT, "%x", ESP_ERR_INVALID_ARG);
            return;     
    }
    
    //printf("\r\n[Publish]\r\n[TOPIC][%s]\r\n[PAYLOAD][%s]\r\n", topic, payload);
    msg_id = esp_mqtt_client_publish(mqtt_client, topic, payload, 0, 0, 0);
    ESP_LOGI(TAG_MQTT, "%d", msg_id);

}