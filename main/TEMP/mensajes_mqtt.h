/** \file	mqtt.h
 *  \brief	Contiene las funciones de inicializacion y manejo del protocolo mqtt
 *  Autor: Ramiro Alonso
 *  Versión: 1
 *	Contiene las funciones de manejo e inicializacion de los GPIOs
 */


#ifndef MENSAJES_MQTT_H
#define MENSAJES_MQTT_H

// // Configuracion del MQTT
// #define Id_NODO "nodo_vialidad"
// #define IP_BROKER_MQTT "192.168.0.10"
// #define PUERTO_MQTT 1883
// #define USUARIO_MQTT "usuario"
// #define PASSWD_MQTT  "usuariopassword"

#include "../main.h"

void analizar_mensaje_mqtt(char * topic, int topic_size, char * mensaje, int mensaje_size);
void subscripciones_mqtt(esp_mqtt_client_handle_t client);


#endif //MENSAJES_MQTT_H
