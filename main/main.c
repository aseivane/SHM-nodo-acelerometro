/*
 * main.c
 *
 *  Created on: May 24, 2020
 *      Author: jaatadia@gmail.com
 */
#include <daemon.h>
#include <stdio.h>
#include <stdint.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_timer.h"

#include "main.h"

#include "wifi.h"
#include "daemon.h"
#include "microtime.h"

#include "esp_timer.h"
#include "esp_intr_alloc.h"
#include "soc/soc.h"
#include "driver/gpio.h"

//#define GPIO_RX2 GPIO_NUM_16 // Interruption pin

// // static int64_t IRAM_ATTR timeRequest = -1;
// // static void IRAM_ATTR interruption_handler(void* arg) {
// //     *((int64_t *)arg) = esp_timer_get_time();
// // }
//
//
//
// void setup_int_ext(int64_t * timeRequest){
//  gpio_config_t gpioConfig;
//  gpioConfig.pin_bit_mask = GPIO_SEL_16; // RX2
//  gpioConfig.mode         = GPIO_MODE_INPUT;
//  gpioConfig.pull_up_en   = GPIO_PULLUP_DISABLE;
//  gpioConfig.pull_down_en = GPIO_PULLDOWN_ENABLE;
//  gpioConfig.intr_type    = GPIO_INTR_POSEDGE;
//  gpio_config(&gpioConfig);
//
//  gpio_install_isr_service(ESP_INTR_FLAG_IRAM);
//  gpio_isr_handler_add(GPIO_RX2, interruption_handler, (void*) timeRequest);
// }



/************************************************************************
* Variables Globales
************************************************************************/
FILE *f_samples = NULL;

SemaphoreHandle_t xSemaphore_tomamuestra = NULL;
SemaphoreHandle_t xSemaphore_guardatabla = NULL;
SemaphoreHandle_t xSemaphore_mutex_archivo = NULL;

uint8_t LED;

mensaje_t mensaje_consola;
muestreo_t Datos_muestreo;

TicTocData * ticTocData;

static const char *TAG = "MAIN "; // Para los mensajes del micro

char id_nodo[20];
volatile char dir_ip[20];


//////////////////////////////////////////////////////////////////
/**
 * @brief Función main
 */
void app_main(void)
{

  //    ESP_ERROR_CHECK(esp_event_loop_create_default()); // Pare el http server

// Configuracion de los mensajes de log por el puerto serie
        esp_log_level_set("MAIN ", ESP_LOG_INFO );
        esp_log_level_set("MPU6050 ", ESP_LOG_ERROR );
        esp_log_level_set("TAREAS ", ESP_LOG_INFO );
        esp_log_level_set("SD_CARD ", ESP_LOG_INFO );
        esp_log_level_set("WIFI ", ESP_LOG_ERROR );
        esp_log_level_set("MQTT ", ESP_LOG_INFO );
        esp_log_level_set("MENSAJES_MQTT ", ESP_LOG_INFO );
        esp_log_level_set("TICTOC ", ESP_LOG_ERROR );
        esp_log_level_set("HTTP_FILE_SERVER ", ESP_LOG_ERROR );
        esp_log_level_set("VARIAS ", ESP_LOG_ERROR );




        /* Valores posibles
           ESP_LOG_NONE → No log output
           ESP_LOG_ERROR → Critical errors, software module can not recover on its own
           ESP_LOG_WARN → Error conditions from which recovery measures have been taken
           ESP_LOG_INFO → Information messages which describe normal flow of events
           ESP_LOG_DEBUG → Extra information which is not necessary for normal use (values, pointers, sizes, etc).
           ESP_LOG_VERBOSE → Bigger chunks of debugging information, or frequent messages which can potentially flood the output.
         */



// Obtencion de la identificación del nodo (string del macadress de la interfaz Wifi)
        uint8_t derived_mac_addr[6] = {0};
        ESP_ERROR_CHECK(esp_read_mac(derived_mac_addr, ESP_MAC_WIFI_STA));
        sprintf(id_nodo, "%x%x%x%x%x%x",
                 derived_mac_addr[0], derived_mac_addr[1], derived_mac_addr[2],
                 derived_mac_addr[3], derived_mac_addr[4], derived_mac_addr[5]);
        ESP_LOGI(TAG, "Identificación: %s", id_nodo);


        // Primero que nada me conecto a la red
        connectToWiFi();

// Inicialización de Variables /////////
        Datos_muestreo.selec_tabla_escritura = 0;
        Datos_muestreo.selec_tabla_lectura = 0;
        Datos_muestreo.nro_muestra_en_seg = 0;
        Datos_muestreo.flag_tabla_llena = false;
        Datos_muestreo.flag_tomar_muestra = false;
        Datos_muestreo.flag_muestra_perdida = false;
        Datos_muestreo.nro_archivo=0;
        Datos_muestreo.nro_tabla=0;
        Datos_muestreo.contador_segundos=0;
        Datos_muestreo.epoch_inicio=0;
        Datos_muestreo.estado_muestreo=ESTADO_ESPERANDO_MENSAJE_DE_INICIO;
//        Datos_muestreo.estado_muestreo=ESTADO_MUESTREANDO;

// Creo los semaforos que voy a usar////////////////////////////////////////////
        xSemaphore_tomamuestra = xSemaphoreCreateBinary();
        xSemaphore_guardatabla = xSemaphoreCreateBinary();
        xSemaphore_mutex_archivo = xSemaphoreCreateMutex();



        if( xSemaphore_tomamuestra != NULL &&  xSemaphore_guardatabla != NULL &&  xSemaphore_mutex_archivo != NULL)
        {
                ESP_LOGI(TAG, "SEMAFOROS CREADOS CORECTAMENTE");
        }
////////////////////////////////////////////////////////////////////////////////

        inicializacion_gpios();
        ESP_ERROR_CHECK(inicializacion_i2c());
        ESP_LOGI(TAG, "I2C Inicializado correctamente");
        inicializacion_tarjeta_SD();
        inicio_mqtt();

        /* Start the file server */
        ESP_ERROR_CHECK(start_file_server("/sdcard"));

// ////// Inicialización del algoritmo de Javier  /////////////////////////
//         TicTocData* ticTocData = malloc(sizeof(TicTocData));
//         setupTicToc(ticTocData, TICTOC_SERVER, TICTOC_PORT);
// ////////////////////////////////////////////////////////////////////////


/* ALGORITMO DE SINCRONISMO*/
TicTocData * ticTocData1 = malloc(sizeof(TicTocData)); /* ALGORITMO DE SINCRONISMO*/
ticTocData = ticTocData1;  /* ALGORITMO DE SINCRONISMO*/
setupTicToc(ticTocData, TICTOC_SERVER, TICTOC_PORT);  /* ALGORITMO DE SINCRONISMO*/

/* ------------------------------------------
   Una pausa al inicio
   --------------------------------------------- */
        // printf("ESPERANDO SEÑAL DE INICIO \n");
        // while (gpio_get_level(GPIO_INPUT_IO_0)) { // Freno todo hasta apretar un boton
        //         esp_task_wdt_reset(); // Esto no detiene el WDT para el idle task que no llega a correr.
        // }
/* ------------------------------------------ */

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// CREO LAS TAREAS DEFINIENDO EN QUE NÚCLEO SE EJECUTAN
// IMPORTANTE: El nombre de la tarea tiene que tener menos de 16 caracteres

        ESP_LOGI(TAG, "INICIANDO TAREAS");

        TaskHandle_t Handle_tarea_i2c = NULL;
        xTaskCreatePinnedToCore(leo_muestras, "leo_muestras", 1024 * 16, (void *)0, 10, &Handle_tarea_i2c,1);
        TaskHandle_t Handle_guarda_datos = NULL;
        xTaskCreatePinnedToCore(guarda_datos, "guarda_datos", 1024 * 16, (void *)0, 9, &Handle_guarda_datos,0);

        // SOLO PARA DEBUGGING
        TaskHandle_t Handle_muestra_info = NULL;
        xTaskCreatePinnedToCore(muestra_info, "muestra_info", 1024 * 2, (void *)0, 1, &Handle_muestra_info,0);

// La interrupcion la inicializo al final
        int timer_muestreo_idx = 0;
        ESP_LOGI(TAG, "INICIANDO TIMER");
        inicializacion_timer_muestreo(timer_muestreo_idx, 1,(40000000/MUESTRAS_POR_SEGUNDO));


// Aparentemente esto no es necesario
        // for(;;) {
        //         vTaskDelay(1000 / portTICK_PERIOD_MS);
        // }
}








// printf("Inicio fwrite(): %u ", ciclos_inic = xthal_get_ccount());
// ciclos_inic = xthal_get_ccount();
// printf("FIN fwrite(): %u ", ciclos_fin = xthal_get_ccount());
// printf("Duracion fwrite(): %u\n", (uint32_t)(ciclos_fin-ciclos_inic) );



// ///////////////////////////////////////////////////////////////////////////////////////////////////////////
// // SIN DEFINIR EL NÚCLEO ...
//
//         ESP_LOGI(TAG, "INICIANDO TAREAS");
//         TaskHandle_t Handle_tarea_i2c = NULL;
//
//         xTaskCreate(leo_muestras, "leo_muestras", 1024 * 2, (void *)0, 10, &Handle_tarea_i2c);
//         TaskHandle_t Handle_muestra_info = NULL;
//         xTaskCreate(muestra_info, "muestra_info", 1024 * 2, (void *)0, 4, &Handle_muestra_info);
//
// #ifndef DESACTIVAR_SD
//         TaskHandle_t Handle_guarda_datos = NULL;
//         xTaskCreate(guarda_datos, "guarda_datos", 1024 * 8, (void *)0, 9, &Handle_guarda_datos);
// #endif
