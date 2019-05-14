/*
 * Button.h
 *
 *  Created on: 20.10.2018
 *      Author: user
 */

#ifndef BUTTON_H_
#define BUTTON_H_

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#include "stm32f10x.h"
#include "stm32f10x_exti.h"
#include "misc.h"
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

typedef enum {
	STARTER = 0,
	ARM_LIFTER,
	ARM_LOWERER,
	ARM_LIMIT_SWITCH,
	PAWL_PUSHER,
	PAWL_COLLAPSER,
	PAWL_LIMIT_SWITCH
} ButtonID;

// - - - - - - - - - - - - - - - - -

struct ButtonSetup {
	GPIO_TypeDef*		gpio_port;		// @ref Peripheral_declaration
	uint16_t 			gpio_pin;		// @ref GPIO_pins
	uint32_t			exti_line;		// @ref EXTI_Lines
	uint8_t				port_source;	// @ref GPIO_Port_Sources
	uint8_t				pin_source;		// @ref GPIO_Pin_sources
	EXTI_InitTypeDef	exti_setup;
	uint8_t				irq_channel;	// @ref IRQn_Type
	NVIC_InitTypeDef	nvic_setup;
};

// - - - - - - - - - - - - - - - - -

struct ButtonState {
	uint8_t 			pushed_flag;
};

// - - - - - - - - - - - - - - - - -

typedef struct {
	struct ButtonSetup setup;
	struct ButtonState state;
} ButtonData;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

extern void		Button_InitialConfig();
extern void 	Button_Config(ButtonID id, GPIO_TypeDef* port, uint16_t pin);
extern void 	Button_SwitchInterruptNVIC(ButtonID id, FunctionalState state);
extern void 	Button_SwitchInterruptEXTI(ButtonID id, FunctionalState state);
extern uint8_t 	Button_GetPushedFlag(ButtonID id);
extern uint8_t  Button_GetState(ButtonID id);

extern void 	EXTI4_IRQHandler();
extern void 	EXTI9_5_IRQHandler();
extern void 	EXTI15_10_IRQHandler();

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#endif /* BUTTON_H_ */
