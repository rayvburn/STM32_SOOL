/*
 * PinConfig_Int.h
 *
 *  Created on: 15.05.2019
 *      Author: user
 */

#ifndef INC_SOOL_PERIPHERALS_GPIO_PINCONFIG_INT_H_
#define INC_SOOL_PERIPHERALS_GPIO_PINCONFIG_INT_H_

#include "sool/Peripherals/GPIO/PinConfig_common.h"
#include "stm32f10x_exti.h"	// EXTI
#include "misc.h" 			// NVIC

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
	struct _SOOL_PinConfigEXTI exti;
	struct _SOOL_PinConfigNVIC nvic;
} SOOL_PinConfig_Int;

// - - - - - - - - - - -

/// \brief Initializes an interrupt pin; hard-coded Internal Pull-Up
extern SOOL_PinConfig_Int SOOL_Periph_GPIO_PinConfig_Initialize_Int(GPIO_TypeDef* gpio_port, const uint16_t gpio_pin, const EXTITrigger_TypeDef exti_trigger);

/// \brief NVIC interrupts switcher ( on (ENABLE) or off (DISABLE) )
extern void	SOOL_Periph_GPIO_PinConfig_NvicSwitch(SOOL_PinConfig_Int *config, const FunctionalState state);

/// \brief EXTI interrupts switcher ( on (ENABLE) or off (DISABLE) )
extern void	SOOL_Periph_GPIO_PinConfig_ExtiSwitch(SOOL_PinConfig_Int *config, const FunctionalState state);

// - - - - - - - - - - -

#endif /* INC_SOOL_PERIPHERALS_GPIO_PINCONFIG_INT_H_ */
