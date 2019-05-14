/*
 * PinConfig.h
 *
 *  Created on: 25.04.2019
 *      Author: user
 */

#ifndef SOOL_PERIPHERALS_GPIO_PINCONFIG_H_
#define SOOL_PERIPHERALS_GPIO_PINCONFIG_H_

// - - - - - - - - - - -

#include "stm32f10x.h"
#include "stm32f10x_exti.h"
#include "misc.h" 			// NVIC

// - - - - - - - - - - -

struct _SOOL_PinConfigGPIO {
	GPIO_TypeDef*		port;		// @ref Peripheral_declaration
	uint16_t 			pin;		// @ref GPIO_pins
};

// - - - - - - - - - - -

struct _SOOL_PinConfigEXTI {
	uint32_t			line;		// @ref EXTI_Lines
	uint8_t				port_src;	// @ref GPIO_Port_Sources
	uint8_t				pin_src;	// @ref GPIO_Pin_sources
	EXTI_InitTypeDef	setup;
};

// - - - - - - - - - - -

struct _SOOL_PinConfigNVIC {
	uint8_t				irqn;		// @ref IRQn_Type
	NVIC_InitTypeDef	setup;
};

// - - - - - - - - - - -

typedef struct {
	struct _SOOL_PinConfigGPIO gpio;
} SOOL_PinConfigNoInt;

// - - - - - - - - - - -

typedef struct {
	struct _SOOL_PinConfigGPIO gpio;
	struct _SOOL_PinConfigEXTI exti;
	struct _SOOL_PinConfigNVIC nvic;
} SOOL_PinConfigInt;

// - - - - - - - - - - -

/// \brief Initializes a non-interrupt pin; hard-coded 50 MHz speed
extern SOOL_PinConfigNoInt	SOOL_GPIO_PinConfig_Initialize_NoInt(GPIO_TypeDef* gpio_port, const uint16_t gpio_pin, const GPIOMode_TypeDef gpio_mode);

/// \brief Initializes an interrupt pin; hard-coded Internal Pull-Up
extern SOOL_PinConfigInt	SOOL_GPIO_PinConfig_Initialize_Int(GPIO_TypeDef* gpio_port, const uint16_t gpio_pin, const EXTITrigger_TypeDef exti_trigger);

/// \brief Initializes an unused pin; internal pull-up prevents pin's state changes caused by noisy environment
extern void					SOOL_GPIO_PinConfig_Initialize_Unused(GPIO_TypeDef* gpio_port, const uint16_t gpio_pin);

/// \brief NVIC interrupts switcher ( on (ENABLE) or off (DISABLE) )
extern void	 				SOOL_GPIO_PinConfig_NvicSwitch(SOOL_PinConfigInt *config, const FunctionalState state);

/// \brief EXTI interrupts switcher ( on (ENABLE) or off (DISABLE) )
extern void	 				SOOL_GPIO_PinConfig_ExtiSwitch(SOOL_PinConfigInt *config, const FunctionalState state);

// - - - - - - - - - - -

#endif /* SOOL_PERIPHERALS_GPIO_PINCONFIG_H_ */
