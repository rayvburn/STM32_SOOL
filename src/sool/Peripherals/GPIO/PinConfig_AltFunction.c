/*
 * PinConfig_AltFunction.c
 *
 *  Created on: 29.06.2019
 *      Author: user
 */

#include "sool/Peripherals/GPIO/PinConfig_AltFunction.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

SOOL_PinConfig_AltFunction SOOL_Periph_GPIO_PinConfig_Initialize_AltFunction(GPIO_TypeDef* gpio_port, uint16_t gpio_pin, GPIOMode_TypeDef gpio_mode) {

	/* `Class` instance */
	SOOL_PinConfig_AltFunction af_config;

	/* Check parameter correctness */
	if ( gpio_mode != GPIO_Mode_AF_OD && gpio_mode != GPIO_Mode_AF_PP ) {
		// wrong gpio_mode given
		return (af_config);
	}

	/* Enable AF clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

	/* Call base `class` `constructor` */
	SOOL_PinConfig_NoInt config = SOOL_Periph_GPIO_PinConfig_Initialize_NoInt(gpio_port, gpio_pin, gpio_mode);

	/* Save `base` configuration */
	af_config.base = config;

	return (af_config);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
