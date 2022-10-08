/** \file	mqtt.c
 *  \brief	Contiene las funciones de inicializacion y manejo del protocolo mqtt
 *  Autor: Ramiro Alonso
 *  Versión: 1
 *	Contiene las funciones de manejo e inicializacion de los GPIOs
 */

#include "mensajes_mqtt.h"

static const char *TAG = "MENSAJES_MQTT ";


#define MAX_TOPIC_LENGTH  100
#define MAX_MSG_LENGTH  100
#define MAX_ARGS_LENGTH  20
#define MAX_CANT_ARGS  5


char Topic_Base[] = "puente_x";
char Topic_InicioMuestreo[] = "/control/inicio_muestreo";

extern int64_t tiempo_inicio;  // Epoch (UTC) resolucion en segundos
extern bool esperando_inicio;
extern muestreo_t Datos_muestreo;


void analizar_mensaje_mqtt(char * topic, int topic_size, char * mensaje, int mensaje_size){

esp_mqtt_client_handle_t client = event->client;
int msg_id;
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

        msg_id = esp_mqtt_client_publish(client, "/topic/qos1", "data_3", 0, 1, 0);


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


void subscripciones_mqtt(esp_mqtt_client_handle_t client){

        char topic_subscribe[MAX_TOPIC_LENGTH] = "";
        strcat(topic_subscribe, Topic_Base);
        strcat(topic_subscribe, Topic_InicioMuestreo);
        esp_mqtt_client_subscribe(client, topic_subscribe, 0);
        strcpy(topic_subscribe, ""); // Limpio el string
}
