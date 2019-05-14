/*
 * PinConfig_Unused.c
 *
 *  Created on: 15.05.2019
 *      Author: user
 */

#include <sool/Peripherals/GPIO/PinConfig_Unused.h>
#include <sool/Peripherals/GPIO/PinConfig_common.h>

void SOOL_GPIO_PinConfig_Initialize_Unused(GPIO_TypeDef* gpio_port, const uint16_t gpio_pin) {

	SOOL_GPIO_PinConfig_EnableAPBClock(gpio_port);
	GPIO_InitTypeDef gpio;
	GPIO_StructInit(&gpio);
	gpio.GPIO_Pin = gpio_pin;
	gpio.GPIO_Speed = GPIO_Speed_2MHz;
	gpio.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(gpio_port, &gpio);

}
