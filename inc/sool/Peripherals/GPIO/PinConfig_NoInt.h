/*
 * PinConfig_NoInt.h
 *
 *  Created on: 15.05.2019
 *      Author: user
 */

#ifndef INC_SOOL_PERIPHERALS_GPIO_PINCONFIG_NOINT_H_
#define INC_SOOL_PERIPHERALS_GPIO_PINCONFIG_NOINT_H_

#include "sool/Peripherals/GPIO/PinConfig_common.h"

// - - - - - - - - - - -

typedef struct {
	struct _SOOL_PinConfigGPIO gpio;
} SOOL_PinConfig_NoInt;

// - - - - - - - - - - -

/// \brief Initializes a non-interrupt pin; hard-coded 50 MHz speed
extern SOOL_PinConfig_NoInt	SOOL_GPIO_PinConfig_Initialize_NoInt(GPIO_TypeDef* gpio_port, const uint16_t gpio_pin, const GPIOMode_TypeDef gpio_mode);

// - - - - - - - - - - -

#endif /* INC_SOOL_PERIPHERALS_GPIO_PINCONFIG_NOINT_H_ */
