/*
 * PinConfig.h
 *
 *  Created on: 25.04.2019
 *      Author: user
 */

#ifndef COMMON_PINCONFIG_H_
#define COMMON_PINCONFIG_H_

// - - - - - - - - - - -

#include "stm32f10x.h"
#include "stm32f10x_exti.h"
#include "misc.h" // NVIC

// - - - - - - - - - - -

struct SoolPinConfigNoInt  {
	GPIO_TypeDef*		gpio_port;		// @ref Peripheral_declaration
	uint16_t 			gpio_pin;		// @ref GPIO_pins
};

// - - - - - - - - - - -

struct SoolPinConfigInt {
	GPIO_TypeDef*		gpio_port;		// @ref Peripheral_declaration
	uint16_t 			gpio_pin;		// @ref GPIO_pins
	uint32_t			exti_line;		// @ref EXTI_Lines
	uint8_t				port_source;	// @ref GPIO_Port_Sources
	uint8_t				pin_source;		// @ref GPIO_Pin_sources
	EXTI_InitTypeDef	exti_setup;
	uint8_t				irq_channel;	// @ref IRQn_Type
	NVIC_InitTypeDef	nvic_setup;
};

// - - - - - - - - - - -

/// \brief Initializes non-interrupt pin
/// hard-coded 50 MHz speed
extern struct SoolPinConfigNoInt SOOL_PinConfig_Initialize_NoInt(GPIO_TypeDef* gpio_port, const uint16_t gpio_pin, const GPIOMode_TypeDef gpio_mode);

extern struct SoolPinConfigInt		SOOL_PinConfig_Initialize_Int(GPIO_TypeDef* gpio_port, const uint16_t gpio_pin, const uint32_t exti_line, const uint8_t port_source, const uint8_t pin_source, const EXTITrigger_TypeDef exti_trigger, const uint8_t irq_channel);

// - - - - - - - - - - -

#endif /* COMMON_PINCONFIG_H_ */
