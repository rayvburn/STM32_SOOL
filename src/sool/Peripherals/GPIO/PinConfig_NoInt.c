/*
 * PinConfig_NoInt.c
 *
 *  Created on: 15.05.2019
 *      Author: user
 */

#include <sool/Peripherals/GPIO/PinConfig_NoInt.h>

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

SOOL_PinConfig_NoInt SOOL_Periph_GPIO_PinConfig_Initialize_NoInt(GPIO_TypeDef* gpio_port, const uint16_t gpio_pin, const GPIOMode_TypeDef gpio_mode) {

	// copy values into structure
	SOOL_PinConfig_NoInt config;
	config.gpio.port = gpio_port;
	config.gpio.pin = gpio_pin;

	// start the clock
	SOOL_Periph_GPIO_PinConfig_EnableAPBClock(gpio_port);

	// initialize pin
	GPIO_InitTypeDef gpio;
	GPIO_StructInit(&gpio);
	gpio.GPIO_Pin = gpio_pin;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	gpio.GPIO_Mode = gpio_mode;
	GPIO_Init(gpio_port, &gpio);

	return (config);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
