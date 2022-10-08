/** \file	mqtt_analisis.h
 *  \brief	Contiene las funciones para analizar los mensajes entrantes por MQTT
 *  Autor: Ramiro Alonso
 *  Versión: 1
 */

#ifndef MQTT_ANALISIS_H_
#define MQTT_ANALISIS_H_

void analizar_mensaje_mqtt(char * topic, int topic_size, char * mensaje, int mensaje_size);
void subscripciones(esp_mqtt_client_handle_t client);

#endif // MQTT_ANALISIS_H_
