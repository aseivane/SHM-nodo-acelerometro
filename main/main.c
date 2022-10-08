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

#include "wifi.h"
#include "daemon.h"
#include "microtime.h"

//#define TICTOC_SERVER "192.168.0.146"
#define TICTOC_SERVER "192.168.0.120"
#define TICTOC_PORT 8080

#include "esp_timer.h"
#include "esp_intr_alloc.h"
#include "soc/soc.h"
#include "driver/gpio.h"

#define GPIO_RX2 GPIO_NUM_16 // Interruption pin

static int64_t IRAM_ATTR timeRequest = -1;
static void IRAM_ATTR interruption_handler(void* arg) {
    *((int64_t *)arg) = esp_timer_get_time();
}

void setup_int_ext(int64_t * timeRequest){
	gpio_config_t gpioConfig;
	gpioConfig.pin_bit_mask = GPIO_SEL_16; // RX2
	gpioConfig.mode         = GPIO_MODE_INPUT;
	gpioConfig.pull_up_en   = GPIO_PULLUP_DISABLE;
	gpioConfig.pull_down_en = GPIO_PULLDOWN_ENABLE;
	gpioConfig.intr_type    = GPIO_INTR_POSEDGE;
	gpio_config(&gpioConfig);

	gpio_install_isr_service(ESP_INTR_FLAG_IRAM);
	gpio_isr_handler_add(GPIO_RX2, interruption_handler, (void*) timeRequest);
}


void app_main(void)
{
	connectToWiFi();
	int64_t* timeRequest = malloc(sizeof(int64_t));
	*timeRequest = -1;
	TicTocData* ticTocData = malloc(sizeof(TicTocData));
	ticTocData->timeRequest = timeRequest;

    setup_int_ext(timeRequest);
    setupTicToc(ticTocData, TICTOC_SERVER, TICTOC_PORT);

    for(;;){
    	vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
}
