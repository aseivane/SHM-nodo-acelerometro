/** \file	mqtt_analisis.c
 *  \brief	Contiene las funciones para analizar los mensajes entrantes por MQTT
 *  Autor: Ramiro Alonso
 *  Versión: 1
 */


#define MAX_TOPIC_LENGTH  100
const char Topic_Base[] = "puente_x";
const char Topic_InicioMuestreo[] = "/control/inicio_muestreo";



#include "mqtt_analisis.h"
#include <stdio.h>
#include <string.h>
#include "mqtt.h"


static const char *TAG = "MQTT_ANALISIS ";


void subscripciones(esp_mqtt_client_handle_t client){

  char topic_subscribe[MAX_TOPIC_LENGTH] = "";

  strcat(topic_subscribe, Topic_Base);
  strcat(topic_subscribe, Topic_InicioMuestreo);
  esp_mqtt_client_subscribe(client, topic_subscribe, 0);
  strcpy(topic_subscribe, "");


}


void analizar_mensaje_mqtt(char * topic, int topic_size, char * mensaje, int mensaje_size){

// Le saco la base al topic recibido. Porque eso cambia entre instalaciones.
char topic_rcv[MAX_TOPIC_LENGTH] = "";
char topic_comp[MAX_TOPIC_LENGTH] = "";

memcpy(topic_rcv, topic+strlen(Topic_Base) , topic_size - strlen(Topic_Base));

strcat(topic_comp, Topic_Base);
strcat(topic_comp, Topic_InicioMuestreo);

  printf("TOPIC=%.*s\r\n", topic_size,  topic);
  printf("TOPIC sin base= %s\r\n", topic_rcv);
  printf("DATA=%.*s\r\n", mensaje_size, mensaje);


printf(topic_comp);

 if(strncmp(topic_comp, mensaje, mensaje_size)==0){

   printf("IGUAL \n");
 }
 else{
   printf("DISTINTO \n");

 }

}
