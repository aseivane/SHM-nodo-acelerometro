/** \file	mqtt.c
 *  \brief	Contiene las funciones de inicializacion y manejo del protocolo mqtt
 *  Autor: Ramiro Alonso
 *  Versión: 1
 *	Contiene las funciones de manejo e inicializacion de los GPIOs
 */

#include "mqtt.h"
#include "main.h"


const char* mqtt_server = IP_BROKER_MQTT;
const int mqttPort = PUERTO_MQTT;
const char* mqttUser = USUARIO_MQTT;
const char* mqttPassword = PASSWD_MQTT;


static const char *TAG = "MQTT ";


#define MAX_TOPIC_LENGTH  100
#define MAX_MSG_LENGTH  100
#define MAX_ARGS_LENGTH  20
#define MAX_CANT_ARGS  5


const char Topic_Base[] = "puente_x";
const char Topic_InicioMuestreo[] = "/control/inicio_muestreo";

extern int64_t tiempo_inicio;  // Epoch (UTC) resolucion en segundos
extern bool esperando_inicio;
extern muestreo_t Datos_muestreo;


void analizar_mensaje_mqtt(char * topic, int topic_size, char * mensaje, int mensaje_size){

// Le saco la base al topic recibido. Porque eso cambia entre instalaciones.
        char topic_rcv[MAX_TOPIC_LENGTH] = "";
        memcpy(topic_rcv, topic+strlen(Topic_Base), topic_size - strlen(Topic_Base));

        /*SEPARO EN ARGUMENTOS*/
        char args_rcv[MAX_CANT_ARGS][MAX_ARGS_LENGTH];
        char msg_rcv[MAX_MSG_LENGTH] = "";
        strncpy(msg_rcv, mensaje, mensaje_size);
        //https://www.tutorialspoint.com/c_standard_library/c_function_strtok.htm
        char *token;
        const char s[2] = " "; // separador
        token = strtok(msg_rcv, s);
        int nro_arg = 0;
        while( token != NULL ) {
                printf( " %s\n", token );
                strcpy(args_rcv[nro_arg], token);
                token = strtok(NULL, s);
                nro_arg++;
        }

        printf("TOPIC=%.*s\r\n", topic_size,  topic);
        printf("TOPIC sin base= %s\r\n", topic_rcv);
        printf("DATA=%.*s\r\n", mensaje_size, mensaje);


        if(strcmp(Topic_InicioMuestreo, topic_rcv)==0) {
                Datos_muestreo.epoch_inicio = atoll(args_rcv[0]);
                Datos_muestreo.epoch_inicio = Datos_muestreo.epoch_inicio * 1000000; // Lo paso a microsegundos
                Datos_muestreo.duracion_muestreo = atoi(args_rcv[1]);
                Datos_muestreo.nro_muestreo = atoi(args_rcv[2]);
                Datos_muestreo.estado_muestreo = ESTADO_CONFIGURAR_ALARMA_INICIO;
                Datos_muestreo.contador_segundos = 0; // Reinicio el contador de segundos
                printf("Tiempo de inicio: %llu \n",Datos_muestreo.epoch_inicio);
                printf("Duracion del muestreo: %d \n",Datos_muestreo.duracion_muestreo);
                printf("Numero de muestreo : %d \n",Datos_muestreo.nro_muestreo);


        }

}

static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{
        esp_mqtt_client_handle_t client = event->client;
        int msg_id;
        // your_context_t *context = event->context;
        switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
                ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
                subscripciones(client); // Despues de conectarme me subscribo a los topics


                msg_id = esp_mqtt_client_publish(client, "/topic/qos1", "data_3", 0, 1, 0);
                ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);

                break;
        case MQTT_EVENT_DISCONNECTED:
                ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
                break;

        case MQTT_EVENT_SUBSCRIBED:
                ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
                break;

        case MQTT_EVENT_UNSUBSCRIBED:
                ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
                break;
        case MQTT_EVENT_PUBLISHED:
                ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
                break;
        case MQTT_EVENT_DATA:   // Cuando recibo un mensaje a un topic que estoy subcripto.
                ESP_LOGI(TAG, "MQTT_EVENT_DATA");
                // printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
                // printf("DATA=%.*s\r\n", event->data_len, event->data);
                analizar_mensaje_mqtt(event->topic,event->topic_len, event->data, event->data_len );
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

        esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg); //   Creates mqtt client handle based on the configuration.
        esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client); // Registro el evento?
        esp_mqtt_client_start(client); // Starts mqtt client with already created client handle.

}

void subscripciones(esp_mqtt_client_handle_t client){

        char topic_subscribe[MAX_TOPIC_LENGTH] = "";

        strcat(topic_subscribe, Topic_Base);
        strcat(topic_subscribe, Topic_InicioMuestreo);
        esp_mqtt_client_subscribe(client, topic_subscribe, 0);
        strcpy(topic_subscribe, ""); // Limpio el string

}



void inicio_mqtt(void){
        mqtt_app_start();
}




/*

   // Para publicar
   msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
   ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);


   msg_id = esp_mqtt_client_subscribe(client, "/topic/qos0", 0);
   ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
 */
