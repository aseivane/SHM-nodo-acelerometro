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

#include <stdio.h>
//#include <sys/unistd.h>
#include <string.h>
#include <sys/stat.h>


#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>


#include <stdint.h>             // Algoritmo de sincronismo
#include "freertos/FreeRTOS.h"  // Algoritmo de sincronismo
#include "freertos/task.h"      // Algoritmo de sincronismo
#include "esp_system.h"         // Algoritmo de sincronismo
#include "esp_spi_flash.h"      // Algoritmo de sincronismo
#include "wifi.h"               // Algoritmo de sincronismo
#include "tic-toc.h"            // Algoritmo de sincronismo
#include "microtime.h"          // Algoritmo de sincronismo


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

#include "esp_vfs_fat.h"
#include "driver/sdspi_host.h"
#include "driver/spi_common.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"
#include "mqtt.h"
#include "tareas.h"

/*****************************************************************************
* Prototipos
*****************************************************************************/

void inicializacion_tarjeta_SD(void);
void extraccion_tarjeta_SD(void);


/*****************************************************************************
* Definiciones
*****************************************************************************/
#define MOUNT_POINT "/sdcard"

#define CANT_BYTES_LECTURA 14    // Cantidad de bytes leidos del MPU6050 (aceletometro, temperatura, giroscopo)
//#define CANT_BYTES_LECTURA 6    // Cantidad de bytes leidos del MPU6050 (solo acelerometro)

#define MUESTRAS_POR_SEGUNDO 500
#define MUESTRAS_POR_TABLA   500
#define TABLAS_POR_ARCHIVO 60
#define LONG_TABLAS MUESTRAS_POR_TABLA*CANT_BYTES_LECTURA
#define GUARDA_DATOS_SD

// DMA channel to be used by the SPI peripheral
#ifndef SPI_DMA_CHAN
#define SPI_DMA_CHAN    1
#endif //SPI_DMA_CHAN




#endif /* MAIN_H_ */
