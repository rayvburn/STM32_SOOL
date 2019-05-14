/*
 * PinConfig_Unused.h
 *
 *  Created on: 15.05.2019
 *      Author: user
 */

#ifndef INC_SOOL_PERIPHERALS_GPIO_PINCONFIG_UNUSED_H_
#define INC_SOOL_PERIPHERALS_GPIO_PINCONFIG_UNUSED_H_

#include "stm32f10x_gpio.h"

/// \brief Initializes an unused pin; internal pull-up prevents pin's state
/// changes caused by noisy environment
extern void SOOL_GPIO_PinConfig_Initialize_Unused(GPIO_TypeDef* gpio_port, const uint16_t gpio_pin);


#endif /* INC_SOOL_PERIPHERALS_GPIO_PINCONFIG_UNUSED_H_ */
