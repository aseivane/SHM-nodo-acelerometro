/*
 * main.c
 *
 *  Created on: May 24, 2020
 *      Author: jaatadia@gmail.com
 */
#include <stdio.h>
#include <stdint.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"

#include "wifi.h"
#include "tic-toc.h"
#include "microtime.h"

#define TICTOC_SERVER "192.168.0.100"
#define TICTOC_PORT 8080


void app_main(void)
{
	connectToWiFi();

	TicTocData * ticTocData = malloc(sizeof(TicTocData));
	setupTicToc(ticTocData, TICTOC_SERVER, TICTOC_PORT);

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
