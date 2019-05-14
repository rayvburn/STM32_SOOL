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

//typedef struct {
//	GPIO_TypeDef*		gpio_port;		// @ref Peripheral_declaration
//	uint16_t 			gpio_pin;		// @ref GPIO_pins
//} SOOL_PinConfigNoInt;

// - - - - - - - - - - -

struct PinConfigGPIO {
	GPIO_TypeDef*		port;		// @ref Peripheral_declaration
	uint16_t 			pin;		// @ref GPIO_pins
};

struct PinConfigEXTI {
	uint32_t			line;		// @ref EXTI_Lines
	uint8_t				port_src;	// @ref GPIO_Port_Sources
	uint8_t				pin_src;		// @ref GPIO_Pin_sources
	EXTI_InitTypeDef	setup;
};

struct PinConfigNVIC {
	uint8_t				irqn;	// @ref IRQn_Type
	NVIC_InitTypeDef	setup;
};
//
//typedef struct {
//	GPIO_TypeDef*		gpio_port;		// @ref Peripheral_declaration
//	uint16_t 			gpio_pin;		// @ref GPIO_pins
//	uint32_t			exti_line;		// @ref EXTI_Lines
//	uint8_t				port_source;	// @ref GPIO_Port_Sources
//	uint8_t				pin_source;		// @ref GPIO_Pin_sources
//	EXTI_InitTypeDef	exti_setup;
//	uint8_t				irq_channel;	// @ref IRQn_Type
//	NVIC_InitTypeDef	nvic_setup;
//} SOOL_PinConfigInt;

typedef struct {
	struct PinConfigGPIO gpio;
} SOOL_PinConfigNoInt;

typedef struct {
	struct PinConfigGPIO gpio;
	struct PinConfigEXTI exti;
	struct PinConfigNVIC nvic;
} SOOL_PinConfigInt;

// - - - - - - - - - - -

/// \brief Initializes a non-interrupt pin; hard-coded 50 MHz speed
extern SOOL_PinConfigNoInt	SOOL_Common_PinConfig_Initialize_NoInt(GPIO_TypeDef* gpio_port, const uint16_t gpio_pin, const GPIOMode_TypeDef gpio_mode);

/// \brief Initializes a interrupt pin; hard-coded Internal Pull-Up
extern SOOL_PinConfigInt	SOOL_Common_PinConfig_Initialize_Int(GPIO_TypeDef* gpio_port, const uint16_t gpio_pin, const EXTITrigger_TypeDef exti_trigger);

/// \brief NVIC interrupts switcher ( on (ENABLE) or off (DISABLE) )
extern void	 				SOOL_Common_PinConfig_NvicSwitch(SOOL_PinConfigInt *config, const FunctionalState state);

/// \brief EXTI interrupts switcher ( on (ENABLE) or off (DISABLE) )
extern void	 				SOOL_Common_PinConfig_ExtiSwitch(SOOL_PinConfigInt *config, const FunctionalState state);

// - - - - - - - - - - -

#endif /* COMMON_PINCONFIG_H_ */
