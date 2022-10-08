/** \file	main.c
 *  \brief	Programa para noso de vialidad, toma muestras de un acelerómetro/giroscopo MPU6050, los graba en una tarjeta SD y los envía a un nodo central.
 *  Autor: Ramiro Alonso
 *  Versión: 1
    idf.py set-target esp32
    idf.py build
    idf.py -p /dev/ttyUSB0 flash
 *	Programa para probar calibrar un acelerómetro I2C, en principio un MPU6050 \

   Documentacion API:
   https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/index.html

   Documentacion RTOS:
   https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/freertos.html
   https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/freertos-smp.html
   https://www.esp32.com/viewtopic.php?t=1293
   https://esp32.com/viewtopic.php?f=2&t=764
   https://www.freertos.org/RTOS_Task_Notification_As_Binary_Semaphore.html → Notificacion en lugar de semaforos

   Para activar las variables de entorno
   get_idf
 */

/*
   ARCHIVOS DEL ALGORITMO DE SINCRONIZACION:
   circularOrderedArray.c
   circularOrderedArray.h
   CMakeLists.txt
   component.mk
   linearfit.c
   linearfit.h
   main.c
   microtime.c
   microtime.h
   sic.c
   sic.h
   tic-toc.c
   tic-toc.h
   wifi.c
   wifi.h

   FORMA DE USO:
   setupTicToc(TicTocData *, const char * serverIp, int serverPort);  → INICIALIZACIÓN (empieza a correr)
   Cuando sincroniza: ticTocReady(TicTocData *) devuelve true
    Estando sincronizado:
        ticTocTime(TicTocData *) (cuando se te antoje y las veces que se te antoje, no hay restricciones)
        para obtener el timestamp actual (que serían los micro segundos desde `epoch`) en vez de llamar al system time.

        En formato humano - legible podes usar la función void microsToTimestamp(int64_t micros, char* stringBuffer);
 */
#include "main.h"


/************************************************************************
* Variables Globales
************************************************************************/

FILE *f_samples = NULL;

uint8_t datos [CANT_BYTES_LECTURA];  // Lugar donde guardo los datos leidos del mpu

SemaphoreHandle_t xSemaphore_tomamuestra = NULL;
SemaphoreHandle_t xSemaphore_guardatabla = NULL;


uint8_t LED;

uint32_t dir_ticTocData;
mensaje_t mensaje_consola;

int64_t tiempo_inicio;  // Epoch (UTC) resolucion en segundos

muestreo_t Datos_muestreo;

static const char *TAG = "MAIN "; // Para los mensajes del micro


/**
 * @brief Función main
 */
TicTocData * ticTocData;

//////////////////////////////////////////////////////////////////

void app_main(void)
{

// Configuracion de los mensajes de log por el puerto serie
        esp_log_level_set("MAIN ", ESP_LOG_INFO );
        esp_log_level_set("MPU6050 ", ESP_LOG_ERROR );
        esp_log_level_set("TAREAS ", ESP_LOG_ERROR );
        esp_log_level_set("SD_CARD ", ESP_LOG_ERROR );
//        esp_log_level_set("TIMER ", ESP_LOG_ERROR );
        esp_log_level_set("WIFI ", ESP_LOG_ERROR );
        esp_log_level_set("MQTT ", ESP_LOG_INFO );
        esp_log_level_set("MQTT_ANALISIS ", ESP_LOG_INFO );
        /* Valores posibles
           ESP_LOG_NONE → No log output
           ESP_LOG_ERROR → Critical errors, software module can not recover on its own
           ESP_LOG_WARN → Error conditions from which recovery measures have been taken
           ESP_LOG_INFO → Information messages which describe normal flow of events
           ESP_LOG_DEBUG → Extra information which is not necessary for normal use (values, pointers, sizes, etc).
           ESP_LOG_VERBOSE → Bigger chunks of debugging information, or frequent messages which can potentially flood the output.
         */

// Inicialización de Variables /////////
        Datos_muestreo.selec_tabla_escritura = 0;
        Datos_muestreo.selec_tabla_lectura = 0;
        Datos_muestreo.nro_muestra = 0;
        Datos_muestreo.flag_tabla_llena = false;
        Datos_muestreo.flag_tomar_muestra = false;
        Datos_muestreo.flag_muestra_perdida = false;
        Datos_muestreo.nro_archivo=0;
        Datos_muestreo.nro_tabla=0;
        Datos_muestreo.contador_segundos=0;
        Datos_muestreo.epoch_inicio=0;
        Datos_muestreo.estado_muestreo=ESTADO_ESPERANDO_MENSAJE_DE_INICIO;

// Creo los semaforos que voy a usar////////////////////////////////////////////
        xSemaphore_tomamuestra = xSemaphoreCreateBinary();
        xSemaphore_guardatabla = xSemaphoreCreateBinary();

        if( xSemaphore_tomamuestra != NULL &&  xSemaphore_guardatabla != NULL)
        {
                ESP_LOGI(TAG, "SEMAFOROS CREADOS CORECTAMENTE");
        }
////////////////////////////////////////////////////////////////////////////////

        inicializacion_gpios();
        connectToWiFi();
        inicio_mqtt();
        ESP_ERROR_CHECK(inicializacion_i2c());

#ifndef DESACTIVAR_SD
        inicializacion_tarjeta_SD();
#endif

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
        xTaskCreatePinnedToCore(leo_muestras, "leo_muestras", 1024 * 2, (void *)0, 10, &Handle_tarea_i2c,1);
        TaskHandle_t Handle_muestra_info = NULL;
        xTaskCreatePinnedToCore(muestra_info, "muestra_info", 1024 * 2, (void *)0, 4, &Handle_muestra_info,0);

#ifndef DESACTIVAR_SD
        TaskHandle_t Handle_guarda_datos = NULL;
        xTaskCreatePinnedToCore(guarda_datos, "guarda_datos", 1024 * 8, (void *)0, 9, &Handle_guarda_datos,0);
#endif

// La interrupcion la inicializo al final
        int timer_muestreo_idx = 0;
        ESP_LOGI(TAG, "INICIANDO TIMER");
        inicializacion_timer_muestreo(timer_muestreo_idx, 1,(40000000/MUESTRAS_POR_SEGUNDO));

} // Fin de MAIN



// printf("Inicio fwrite(): %u ", ciclos_inic = xthal_get_ccount());
// ciclos_inic = xthal_get_ccount();
// printf("FIN fwrite(): %u ", ciclos_fin = xthal_get_ccount());
// printf("Duracion fwrite(): %u\n", (uint32_t)(ciclos_fin-ciclos_inic) );
