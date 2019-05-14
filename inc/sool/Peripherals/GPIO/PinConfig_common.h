/*
 * PinConfig_common.h
 *
 *  Created on: 15.05.2019
 *      Author: user
 */

#ifndef INC_SOOL_PERIPHERALS_GPIO_PINCONFIG_COMMON_H_
#define INC_SOOL_PERIPHERALS_GPIO_PINCONFIG_COMMON_H_

#include "stm32f10x_gpio.h" // includes stdint.h

// - - - - - - - - - - -

struct _SOOL_PinConfigGPIO {
	GPIO_TypeDef*		port;		// @ref Peripheral_declaration
	uint16_t 			pin;		// @ref GPIO_pins
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

extern void SOOL_GPIO_PinConfig_EnableAPBClock(GPIO_TypeDef* port);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#endif /* INC_SOOL_PERIPHERALS_GPIO_PINCONFIG_COMMON_H_ */
