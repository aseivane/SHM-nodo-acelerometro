/** \file	GPIOS.h
 *  \brief	Contiene las funciones de manejo e inicializacion de los GPIOs
 *  Autor: Ramiro Alonso
 *  Versión: 1
 *	Contiene las funciones de manejo e inicializacion de los GPIOs
 */

#ifndef GPIO_H_
#define GPIO_H_

/*****************************************************************************
* Prototipos
*****************************************************************************/

 //static void IRAM_ATTR gpio_isr_handler(void* arg);
 int inicializacion_gpios(void);


/*****************************************************************************
* Definiciones
*****************************************************************************/
#define GPIO_INPUT_IO_0     16  // Entrada en GPIO 16  → BOTON


#define GPIO_OUTPUT_IO_0    32 // LED
#define GPIO_OUTPUT_IO_1    33
#define GPIO_OUTPUT_PIN_SEL  ((1ULL<<GPIO_OUTPUT_IO_0) | (1ULL<<GPIO_OUTPUT_IO_1))


//#define GPIO_INPUT_IO_1     5
#define GPIO_INPUT_PIN_SEL  ((1ULL<<GPIO_INPUT_IO_0))
#define ESP_INTR_FLAG_DEFAULT 0

#endif /* GPIO_H_ */
