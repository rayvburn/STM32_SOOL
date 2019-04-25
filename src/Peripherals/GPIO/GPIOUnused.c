/*
 * GPIOUnused.c
 *
 *  Created on: 22.10.2018
 *      Author: user
 */

#include "GPIOUnused.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void GPIOUnused_EnableAPBClock(GPIO_TypeDef* port);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void GPIOUnused_Config(GPIO_TypeDef* port, uint16_t pin) {

	GPIOUnused_EnableAPBClock(port);
	GPIO_InitTypeDef gpio;
	GPIO_StructInit(&gpio);
	gpio.GPIO_Pin = pin;
	gpio.GPIO_Speed = GPIO_Speed_2MHz;
	gpio.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(port, &gpio);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// private function
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static void GPIOUnused_EnableAPBClock(GPIO_TypeDef* port) {

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
