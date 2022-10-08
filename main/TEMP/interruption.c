#include "interruption.h"


#include "esp_timer.h"
#include "esp_intr_alloc.h"
#include "soc/soc.h"
#include "driver/gpio.h"



extern int64_t IRAM_ATTR timeRequest;

#define GPIO_RX2 GPIO_NUM_16 // Interruption pin

static void IRAM_ATTR interruption_handler(void* arg) {
    timeRequest = esp_timer_get_time();
}

void setup_int_ext(){
	gpio_config_t gpioConfig;
	gpioConfig.pin_bit_mask = GPIO_SEL_16; // RX2
	gpioConfig.mode         = GPIO_MODE_INPUT;
	gpioConfig.pull_up_en   = GPIO_PULLUP_DISABLE;
	gpioConfig.pull_down_en = GPIO_PULLDOWN_ENABLE;
	gpioConfig.intr_type    = GPIO_INTR_POSEDGE;
	gpio_config(&gpioConfig);

	gpio_install_isr_service(ESP_INTR_FLAG_IRAM);
	gpio_isr_handler_add(GPIO_RX2, interruption_handler, NULL);
}
