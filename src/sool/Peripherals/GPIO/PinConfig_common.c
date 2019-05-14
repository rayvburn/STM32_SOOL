/*
 * PinConfig_common.c
 *
 *  Created on: 15.05.2019
 *      Author: user
 */

#include <sool/Peripherals/GPIO/PinConfig_common.h>
#include "stm32f10x_rcc.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void SOOL_GPIO_PinConfig_EnableAPBClock(GPIO_TypeDef* port) {

	if ( port == GPIOA ) {
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	} else if ( port == GPIOB ) {
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	} else if ( port == GPIOC ) {
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	} else if ( port == GPIOD ) {
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
	}

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
