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
*/
#include "main.h"

/************************************************************************
* Variables Globales
************************************************************************/

uint32_t ciclos_inic;
uint32_t ciclos_fin;

FILE *f_samples = NULL;

uint8_t datos [CANT_BYTES_LECTURA];  // Lugar donde guardo los datos leidos del mpu
int timer_muestreo_idx;

SemaphoreHandle_t xSemaphore_tomamuestra = NULL;
SemaphoreHandle_t xSemaphore_guardatabla = NULL;

bool flag_tomar_muestra;
bool flag_muestra_perdida;
bool flag_tabla_llena;
bool flag_tabla_perdida;
uint8_t LED;
uint8_t TABLA0[LONG_TABLAS];
uint8_t TABLA1[LONG_TABLAS];
uint8_t selec_tabla_escritura;
uint8_t selec_tabla_lectura;
uint8_t nro_tabla;
uint32_t nro_muestra;
uint32_t nro_archivo;

static const char *TAG = "MAIN"; // Para los mensajes del micro


/**
 * @brief Función main
 */

void app_main(void)
{
// Inicialización de Variables /////////
    selec_tabla_escritura = 0;
    selec_tabla_lectura = 0;
    nro_muestra = 0;
    flag_tabla_llena = false;
    flag_tomar_muestra = false;
    flag_muestra_perdida = false;
    nro_archivo=0;
    nro_tabla=0;

// Creo los semaforos que voy a usar////////////////////////////////////////////
    xSemaphore_tomamuestra = xSemaphoreCreateBinary();
    xSemaphore_guardatabla = xSemaphoreCreateBinary();
    if( xSemaphore_tomamuestra != NULL &&  xSemaphore_guardatabla != NULL)
    {
        printf("Semaforos creado correctamente\n");
    }
////////////////////////////////////////////////////////////////////////////////

    inicializacion_gpios();

    connectToWiFi();                                          /* ALGORITMO DE SINCRONISMO*/
    TicTocData * ticTocData = malloc(sizeof(TicTocData));     /* ALGORITMO DE SINCRONISMO*/
    setupTicToc(ticTocData, TICTOC_SERVER, TICTOC_PORT);      /* ALGORITMO DE SINCRONISMO*/

    inicio_mqtt();

/* ------------------------------------------
Una pausa al inicio
--------------------------------------------- */

    printf("ESPERANDO SEÑAL DE INICIO \n");
    while (gpio_get_level(GPIO_INPUT_IO_0)){  // Freno todo hasta apretar un boton
      esp_task_wdt_reset();
    }
/* ------------------------------------------ */

    #ifdef GUARDA_DATOS_SD
        inicializacion_tarjeta_SD();
    #endif


    ESP_ERROR_CHECK(inicializacion_i2c());
    inicializacion_timer_muestreo(timer_muestreo_idx, 1,(40000000/MUESTRAS_POR_SEGUNDO));

// No mejora al definir en que núcleo se ejecutan las tareas ni al configurar el RTOS para que se ejecute sólo en el core 0

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// CREO LAS TAREAS DEFINIENDO EN QUE NÚCLEO SE EJECUTAN
// IMPORTANTE: El nombre de la tarea tiene que tener menos de 16 caracteres
    TaskHandle_t Handle_tarea_i2c = NULL;
    xTaskCreatePinnedToCore(leo_muestras, "leo_muestras", 1024 * 2, (void *)0, 10, &Handle_tarea_i2c,1);
#ifdef GUARDA_DATOS_SD
    TaskHandle_t Handle_guarda_datos = NULL;
    xTaskCreatePinnedToCore(guarda_datos, "guarda_datos", 1024 * 8, (void *)0, 9, &Handle_guarda_datos,1);
#endif
     TaskHandle_t Handle_muestra_info = NULL;
     xTaskCreate(muestra_info, "muestra_info", 1024 * 2, (void *)0, 4, &Handle_muestra_info);

  for(;;){
    vTaskDelay(10000 / portTICK_PERIOD_MS);

    char ticToctimestamp[64];
    char timestamp[64];
  microsToTimestamp(epochInMicros(), timestamp);
    if(!ticTocReady(ticTocData)) {
      printf("Waiting for ticTocTo be ready. (systemTime: %s)\n", timestamp);
    } else {
    int64_t ttTime = ticTocTime(ticTocData);
    microsToTimestamp(ttTime, ticToctimestamp);
    printf("TicTocTime %s (%lld) (systemTime: %s)\n",ticToctimestamp,  ttTime, timestamp);
    }
  }

}



// printf("Inicio fwrite(): %u ", ciclos_inic = xthal_get_ccount());
// ciclos_inic = xthal_get_ccount();
// printf("FIN fwrite(): %u ", ciclos_fin = xthal_get_ccount());
// printf("Duracion fwrite(): %u\n", (uint32_t)(ciclos_fin-ciclos_inic) );
