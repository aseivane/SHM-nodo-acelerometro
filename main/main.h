/** \file	sd_card.h
 *  \brief	Contiene las funciones de manejo e inicializacion de la tarjeta SD
 *  Autor: Ramiro Alonso
 *  Versión: 1
 *	Contiene las funciones de manejo e inicializacion de la tarjeta SD
 */


#ifndef MAIN_H_
#define MAIN_H_

#define TICTOC_SERVER "192.168.0.100"   // Algoritmo de sincronismo
#define TICTOC_PORT 8080                // Algoritmo de sincronismo
#define EXAMPLE_ESP_WIFI_SSID "The Dude"
#define EXAMPLE_ESP_WIFI_PASS "zarzaparrilla"


// Configuracion del MQTT
#define Id_NODO "nodo_vialidad"
#define IP_BROKER_MQTT "192.168.0.10"
#define PUERTO_MQTT 1883
#define USUARIO_MQTT "usuario"
#define PASSWD_MQTT  "usuariopassword"


#define MUESTRA_DATOS_SINCRONIZACION
//#define MUESTRA_ESTADISTICAS_CPU
#define ARCHIVOS_CON_ENCABEZADO

//#define SD_40MHZ  // Para mas velocidad en la tarjeta SD (default es 20MHz), a 20MHz es mas estable.


/*
* ------  CONFIGURACIONES DEL MUESTREO  --------
*/
#define CANT_BYTES_LECTURA 14    // Cantidad de bytes leidos del MPU6050 (aceletometro, temperatura, giroscopo)
//#define CANT_BYTES_LECTURA 6    // Cantidad de bytes leidos del MPU6050 (solo acelerometro)
#define MUESTRAS_POR_SEGUNDO 500
#define MUESTRAS_POR_TABLA   500
#define TABLAS_POR_ARCHIVO 60
#define LONG_TABLAS MUESTRAS_POR_TABLA*CANT_BYTES_LECTURA


#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/unistd.h>

#include <stdint.h>             // Algoritmo de sincronismo
#include "freertos/FreeRTOS.h"  // Algoritmo de sincronismo
#include "freertos/task.h"      // Algoritmo de sincronismo
#include "esp_system.h"         // Algoritmo de sincronismo
#include "esp_spi_flash.h"      // Algoritmo de sincronismo
#include "wifi.h"               // Algoritmo de sincronismo


#include "esp_err.h"
#include "esp_log.h"
#include "driver/i2c.h"
#include "driver/timer.h"
#include "sdkconfig.h"
#include "esp_task_wdt.h"

#include "driver/uart.h"


#include "sd_card.h"
#include "acelerometroI2C.h"
#include "GPIO.h"
#include "timer_muestreo.h"
#include "file_server.h"


#include "esp_vfs_fat.h"
#include "driver/sdspi_host.h"
#include "driver/spi_common.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"
#include "mqtt.h"
#include "tareas.h"


// Para la publicacion de mensajes por consola
typedef struct mentaje_t {
        bool mensaje_nuevo;
        char mensaje[100];
} mensaje_t;



// ESTADOS MUESTREO
#define ESTADO_ESPERANDO_MENSAJE_DE_INICIO  0
#define ESTADO_CONFIGURAR_ALARMA_INICIO_A   1
#define ESTADO_CONFIGURAR_ALARMA_INICIO_B   2
#define ESTADO_ESPERANDO_INICIO             3
#define ESTADO_MUESTREANDO                  4
#define ESTADO_FINALIZANDO_MUESTREO         5
#define ESTADO_MUESTREANDO_ASYNC            6


typedef struct muestreo_t {
        uint8_t estado_muestreo;
        int64_t epoch_inicio;  // Epoch (UTC) resolucion en segundos
        uint32_t int_contador_segundos;  // Contador de la duracion del muestreo
        uint32_t nro_muestreo;       // Identificador del muestreo en curso
        uint8_t datos_mpu [CANT_BYTES_LECTURA]; // Lugar donde guardo los datos leidos del mpu
        uint8_t TABLA0[LONG_TABLAS];
        uint8_t TABLA1[LONG_TABLAS];
        uint8_t selec_tabla_escritura;
        uint8_t selec_tabla_lectura;
        uint8_t nro_tabla_guardada;
        uint8_t nro_tabla_enviada;
        uint32_t nro_muestra_en_seg;
        uint32_t nro_muestra_total_muestreo;
        uint32_t nro_archivo;
        uint32_t duracion_muestreo;
        bool flag_tomar_muestra;
        bool flag_muestra_perdida;
        bool flag_tabla_llena;
        bool flag_tabla_perdida;
        uint32_t cant_muestras_perdidas;  // Contador de muestras perdidas en una tabla.
        uint32_t cantidad_de_interrupciones_de_muestreo;
        uint32_t cantidad_de_muestras_leidas;
}muestreo_t;

/*****************************************************************************
* Prototipos
*****************************************************************************/

// void inicializacion_tarjeta_SD(void);
// void extraccion_tarjeta_SD(void);

/*****************************************************************************
* Definiciones
*****************************************************************************/

#define MOUNT_POINT "/sdcard"
// DMA channel to be used by the SPI peripheral
#ifndef SPI_DMA_CHAN
#define SPI_DMA_CHAN    1
#endif //SPI_DMA_CHAN







#endif /* MAIN_H_ */
