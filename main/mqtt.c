/** \file	mqtt.c
 *  \brief	Contiene las funciones de inicializacion y manejo del protocolo mqtt
 *  Autor: Ramiro Alonso
 *  Versión: 1
 *	Contiene las funciones de manejo e inicializacion de los GPIOs
 */

#include "mqtt.h"

const char* mqtt_server = IP_BROKER_MQTT;
const int   mqttPort = PUERTO_MQTT;
const char* mqttUser = USUARIO_MQTT;
const char* mqttPassword = PASSWD_MQTT;

static const char *TAG = "MQTT_CLIENT";

static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    // your_context_t *context = event->context;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            msg_id = esp_mqtt_client_publish(client, "/topic/qos1", "data_3", 0, 1, 0);
            ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);

            msg_id = esp_mqtt_client_subscribe(client, "/topic/qos0", 0);
            ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

            msg_id = esp_mqtt_client_subscribe(client, "/topic/qos1", 1);
            ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

            msg_id = esp_mqtt_client_unsubscribe(client, "/topic/qos1");
            ESP_LOGI(TAG, "sent unsubscribe successful, msg_id=%d", msg_id);
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
            ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            printf("DATA=%.*s\r\n", event->data_len, event->data);
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            break;
        default:
            ESP_LOGI(TAG, "Other event id:%d", event->event_id);
            break;
    }
    return ESP_OK;
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    mqtt_event_handler_cb(event_data);
}

static void mqtt_app_start(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        //.uri = CONFIG_BROKER_URL,
        .host= mqtt_server,
        .username = mqttUser,
        .password = mqttPassword,
        .port = mqttPort,
    };

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_mqtt_client_start(client);

}

void inicio_mqtt(void){
     mqtt_app_start();
}

// void app_main(void)
// {
//     // ESP_LOGI(TAG, "[APP] Startup..");
//     // ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
//     // ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());
//     //
//     // esp_log_level_set("*", ESP_LOG_INFO);
//     // esp_log_level_set("MQTT_CLIENT", ESP_LOG_VERBOSE);
//     // esp_log_level_set("MQTT_EXAMPLE", ESP_LOG_VERBOSE);
//     // esp_log_level_set("TRANSPORT_TCP", ESP_LOG_VERBOSE);
//     // esp_log_level_set("TRANSPORT_SSL", ESP_LOG_VERBOSE);
//     // esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
//     // esp_log_level_set("OUTBOX", ESP_LOG_VERBOSE);
//
//     ESP_ERROR_CHECK(nvs_flash_init());
//     ESP_ERROR_CHECK(esp_netif_init());
//     ESP_ERROR_CHECK(esp_event_loop_create_default());
//
//     /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
//      * Read "Establishing Wi-Fi or Ethernet Connection" section in
//      * examples/protocols/README.md for more information about this function.
//      */
//     ESP_ERROR_CHECK(example_connect());
//
//     mqtt_app_start();
//
//     // esp_mqtt_client_handle_t client = event->client;
//     // esp_mqtt_client_publish(client, "/topic/qos1", "hola", 0, 1, 0);
//
//
//   }
