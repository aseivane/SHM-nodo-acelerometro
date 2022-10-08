/** \file	timer_muestreo.c
 *  \brief	Contiene las funciones de inicializacion y el handler del timer
 *  Autor: Ramiro Alonso
 *  Versión: 1
 *	Contiene las funciones de manejo e inicializacion de los GPIOs
 */

#include <stdio.h>
#include "main.h"
#include "esp_log.h"
#include "driver/i2c.h"
#include "sdkconfig.h"
#include "driver/timer.h"
#include "timer_muestreo.h"
#include "GPIO.h"


/************************************************************************
* Variables externas
************************************************************************/

extern SemaphoreHandle_t xSemaphore_tomamuestra;
extern SemaphoreHandle_t xSemaphore_guardatabla;
extern bool flag_tomar_muestra;
extern bool flag_muestra_perdida;
extern uint8_t LED;

/************************************************************************
* Variables
************************************************************************/
//extern volatile xQueueHandle gpio_evt_queue = NULL;
volatile uint32_t ciclos_seg;
volatile uint32_t ciclos_int;
volatile uint32_t ciclos_ant;

volatile uint64_t valor_interrupcion_timer;


static const char *TAG = "TIMER ";

/**
 * @brief Handler de la interrupcion del Timer
 * El timer cuenta a 40MHz
 */

bool muestreando;
bool esperando_inicio;


extern int64_t tiempo_inicio;  // Epoch (UTC) resolucion en segundos
extern uint32_t nro_muestra;

int64_t ttTime1;
int32_t cont_segs_muestreo;

extern TicTocData * ticTocData;

uint64_t aux_t;

void IRAM_ATTR ISR_Handler_timer_muestreo(void *ptr)
{
        //static BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        if (LED == 0) {
                gpio_set_level(GPIO_OUTPUT_IO_0, 1);
                LED=1;
        }
        else {
                LED=0;
                gpio_set_level(GPIO_OUTPUT_IO_0, 0);
        }



        ttTime1 = ticTocTime(ticTocData);

        muestreando = true;


        /* Unblock the task by releasing the semaphore. */

        xSemaphoreGiveFromISR( xSemaphore_tomamuestra, NULL );
//        xSemaphoreGiveFromISR( xSemaphore_tomamuestra, &xHigherPriorityTaskWoken );
        /* Esto se usa para comprobar si se perdieron muestras */
        if (flag_tomar_muestra == true){ // Si es true es porque no se leyó la muestra anterior.
          flag_muestra_perdida = true;
        }
        flag_tomar_muestra = true;

        //   valor_interrupcion_timer = valor_interrupcion_timer + 40000; // Modifico el valor de cuenta de la alarma
        timer_set_alarm_value(TIMER_GROUP_0, 0, valor_interrupcion_timer);


        timer_group_clr_intr_status_in_isr(TIMER_GROUP_0, TIMER_0);
        timer_group_enable_alarm_in_isr(TIMER_GROUP_0, TIMER_0);

        portYIELD_FROM_ISR (); // Solicito cambio de contexto
}

/*
 * Inicializacion del timer 0
 *
 * timer_idx - the timer number to initialize
 * auto_reload - should the timer auto reload on alarm?
 * timer_interval_sec - the interval of alarm to set
 */
void inicializacion_timer_muestreo(int timer_idx, bool auto_reload, uint64_t valor_max_conteo)
{
        aux_t = 1000;
        /* Select and initialize basic parameters of the timer */
        timer_config_t config;
        config.divider = TIMER_DIVIDER;
        config.counter_dir = TIMER_COUNT_UP;
        config.counter_en = TIMER_PAUSE;
        config.alarm_en = TIMER_ALARM_EN;
        config.intr_type = TIMER_INTR_LEVEL;
        config.auto_reload = auto_reload;
 #ifdef TIMER_GROUP_SUPPORTS_XTAL_CLOCK
        config.clk_src = TIMER_SRC_CLK_APB;
 #endif
        timer_init(TIMER_GROUP_0, timer_idx, &config);

        /* El timer inicia el conteo en el siguiente valor.
           Si se configura en "auto reload", se reinicia en el siguiente valor.*/
        timer_set_counter_value(TIMER_GROUP_0, timer_idx, 0);

        /* Configure the alarm value and the interrupt on alarm. */
        timer_set_alarm_value(TIMER_GROUP_0, timer_idx, valor_max_conteo);

        valor_interrupcion_timer = valor_max_conteo;
        timer_enable_intr(TIMER_GROUP_0, timer_idx);
        timer_isr_register(TIMER_GROUP_0, timer_idx, ISR_Handler_timer_muestreo,
                           (void *) timer_idx, ESP_INTR_FLAG_IRAM, NULL);

        timer_start(TIMER_GROUP_0, timer_idx);
}

// Funciones para iniciar y frenar el timer
/*
   timer_pause(TIMER_GROUP_0, TIMER_0);

   timer_start(TIMER_GROUP_0, TIMER_0);
 */
