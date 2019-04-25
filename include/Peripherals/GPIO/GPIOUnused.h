/*
 * GPIOUnused.h
 *
 *  Created on: 22.10.2018
 *      Author: user
 */

#ifndef GPIOUNUSED_H_
#define GPIOUNUSED_H_

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#include "stm32f10x.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

extern void GPIOUnused_Config(GPIO_TypeDef* port, uint16_t pin);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#endif /* GPIOUNUSED_H_ */
