/*
 * PinConfig.c
 *
 *  Created on: 25.04.2019
 *      Author: user
 */

#include "PinConfig.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_exti.h"
#include "stm32f10x_rcc.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// private functions declarations
static void SOOL_PinConfig_EnableAPBClock(GPIO_TypeDef* port);
static void SOOL_PinConfig_SetEXTILineEXTIPinSourceIRQn(uint16_t pin, uint32_t *exti_ln, uint8_t *pin_src, uint8_t *irqn);
static void SOOL_PinConfig_SetEXTIPortSource(const GPIO_TypeDef* port, uint8_t *port_src);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

struct SoolPinConfigNoInt PinConfig_Initialize_NoInt(GPIO_TypeDef* gpio_port, const uint16_t gpio_pin) {

	struct SoolPinConfigNoInt config;
	config.gpio_port = gpio_port;
	config.gpio_pin = gpio_pin;
	return (config);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

struct SoolPinConfigInt PinConfig_Initialize_Int(GPIO_TypeDef* gpio_port, const uint16_t gpio_pin,
					const uint32_t exti_line, const uint8_t port_source, const uint8_t pin_source,
					const EXTITrigger_TypeDef exti_trigger, const uint8_t irq_channel) {

	/* object to be filled with given values
	 * and peripherals on which it depends
	 * will be started */
	struct SoolPinConfigInt config;

	uint32_t exti_ln;
	uint8_t pin_src, port_src, irqn;

	SOOL_PinConfig_SetEXTIPortSource(gpio_port, &port_src);
	SOOL_PinConfig_SetEXTILineEXTIPinSourceIRQn(gpio_pin, &exti_ln, &pin_src, &irqn);

	SOOL_PinConfig_EnableAPBClock(gpio_port);

	// alternative function clock
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

	GPIO_InitTypeDef gpio;
	GPIO_StructInit(&gpio);
	gpio.GPIO_Pin = gpio_pin;
	gpio.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(gpio_port, &gpio);

	EXTI_InitTypeDef exti;
	EXTI_StructInit(&exti);
	exti.EXTI_Line = exti_ln;
	exti.EXTI_Mode = EXTI_Mode_Interrupt;
	exti.EXTI_Trigger = exti_trigger;
	exti.EXTI_LineCmd = ENABLE;
	// exti init at the end of the function
	// exti line config at the end of the function

	NVIC_InitTypeDef nvic;
	nvic.NVIC_IRQChannel = irqn;
	nvic.NVIC_IRQChannelPreemptionPriority = 0x00;
	nvic.NVIC_IRQChannelSubPriority = 0x00;
	nvic.NVIC_IRQChannelCmd = ENABLE;
	// nvic init at the end of the function

	// gpio-specific struct fields

	config.gpio_port = gpio_port;
	config.gpio_pin = gpio_pin;
	config.exti_line = exti_ln;
	config.port_source = port_src;
	config.pin_source = pin_src;
	config.irq_channel = irqn;

	// inittypedef struct fields
	config.exti_setup = exti;
	config.nvic_setup = nvic;

	EXTI_Init(&exti);
	GPIO_EXTILineConfig(port_src, pin_src);
	NVIC_Init(&nvic);

	return (config);

}

// =============================================================================================

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// private function
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static void SOOL_PinConfig_EnableAPBClock(GPIO_TypeDef* port) {

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

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// private function
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static void SOOL_PinConfig_SetEXTILineEXTIPinSourceIRQn(uint16_t pin, uint32_t *exti_ln,
										 	  	uint8_t *pin_src, uint8_t *irqn) {

	switch(pin) {

	case(GPIO_Pin_0):
		*exti_ln = EXTI_Line0;
		*pin_src = GPIO_PinSource0;
		*irqn = EXTI0_IRQn;
		break;
	case(GPIO_Pin_1):
		*exti_ln = EXTI_Line1;
		*pin_src = GPIO_PinSource1;
		*irqn = EXTI1_IRQn;
		break;
	case(GPIO_Pin_2):
		*exti_ln = EXTI_Line2;
		*pin_src = GPIO_PinSource2;
		*irqn = EXTI2_IRQn;
		break;
	case(GPIO_Pin_3):
		*exti_ln = EXTI_Line3;
		*pin_src = GPIO_PinSource3;
		*irqn = EXTI3_IRQn;
		break;
	case(GPIO_Pin_4):
		*exti_ln = EXTI_Line4;
		*pin_src = GPIO_PinSource4;
		*irqn = EXTI4_IRQn;
		break;
	case(GPIO_Pin_5):
		*exti_ln = EXTI_Line5;
		*pin_src = GPIO_PinSource5;
		*irqn = EXTI9_5_IRQn;
		break;
	case(GPIO_Pin_6):
		*exti_ln = EXTI_Line6;
		*pin_src = GPIO_PinSource6;
		*irqn = EXTI9_5_IRQn;
		break;
	case(GPIO_Pin_7):
		*exti_ln = EXTI_Line7;
		*pin_src = GPIO_PinSource7;
		*irqn = EXTI9_5_IRQn;
		break;
	case(GPIO_Pin_8):
		*exti_ln = EXTI_Line8;
		*pin_src = GPIO_PinSource8;
		*irqn = EXTI9_5_IRQn;
		break;
	case(GPIO_Pin_9):
		*exti_ln = EXTI_Line9;
		*pin_src = GPIO_PinSource9;
		*irqn = EXTI9_5_IRQn;
		break;
	case(GPIO_Pin_10):
		*exti_ln = EXTI_Line10;
		*pin_src = GPIO_PinSource10;
		*irqn = EXTI15_10_IRQn;
		break;
	case(GPIO_Pin_11):
		*exti_ln = EXTI_Line11;
		*pin_src = GPIO_PinSource11;
		*irqn = EXTI15_10_IRQn;
		break;
	case(GPIO_Pin_12):
		*exti_ln = EXTI_Line12;
		*pin_src = GPIO_PinSource12;
		*irqn = EXTI15_10_IRQn;
		break;
	case(GPIO_Pin_13):
		*exti_ln = EXTI_Line13;
		*pin_src = GPIO_PinSource13;
		*irqn = EXTI15_10_IRQn;
		break;
	case(GPIO_Pin_14):
		*exti_ln = EXTI_Line14;
		*pin_src = GPIO_PinSource14;
		*irqn = EXTI15_10_IRQn;
		break;
	case(GPIO_Pin_15):
		*exti_ln = EXTI_Line15;
		*pin_src = GPIO_PinSource15;
		*irqn = EXTI15_10_IRQn;
		break;

	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// private function
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static void SOOL_PinConfig_SetEXTIPortSource(const GPIO_TypeDef* port, uint8_t *port_src) {

	if ( port == GPIOA ) {
		*port_src = GPIO_PortSourceGPIOA;
	} else if ( port == GPIOB ) {
		*port_src = GPIO_PortSourceGPIOB;
	} else if ( port == GPIOC ) {
		*port_src = GPIO_PortSourceGPIOC;
	} else if ( port == GPIOD ) {
		*port_src = GPIO_PortSourceGPIOD;
	}

}
