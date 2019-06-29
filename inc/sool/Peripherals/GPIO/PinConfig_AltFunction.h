/*
 * PinConfig_AltFunction.h
 *
 *  Created on: 29.06.2019
 *      Author: user
 */

#ifndef INC_SOOL_PERIPHERALS_GPIO_PINCONFIG_ALTFUNCTION_H_
#define INC_SOOL_PERIPHERALS_GPIO_PINCONFIG_ALTFUNCTION_H_

#include "sool/Peripherals/GPIO/PinConfig_NoInt.h"

// - - - - - - - - - - -

typedef struct {
	SOOL_PinConfig_NoInt base;
} SOOL_PinConfig_AltFunction;

// - - - - - - - - - - -

/// \brief Initializes a non-interrupt pin with alternative function mode; hard-coded 50 MHz speed
extern SOOL_PinConfig_AltFunction SOOL_Periph_GPIO_PinConfig_Initialize_AltFunction(GPIO_TypeDef* gpio_port, uint16_t gpio_pin, GPIOMode_TypeDef gpio_mode);

// - - - - - - - - - - -

#endif /* INC_SOOL_PERIPHERALS_GPIO_PINCONFIG_ALTFUNCTION_H_ */
