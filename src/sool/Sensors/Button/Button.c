/*
 * Button.c
 *
 *  Created on: 14.05.2019
 *      Author: user
 */

#include "sool/Sensors/Button/Button.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t Button_GetPushedFlag(volatile SOOL_Button *button_ptr);
static uint8_t Button_GetCurrentState(const volatile SOOL_Button *button_ptr);
static uint8_t Button_InterruptHandler(volatile SOOL_Button *button_ptr);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

volatile SOOL_Button SOOL_Sensor_Button_Init(SOOL_PinConfig_Int setup) {

	volatile SOOL_Button button;

	// setup
//	button.base.exti.line = setup.exti.line;
//	button.base.exti.setup = setup.exti.setup;
//	button.base.gpio.pin = setup.gpio.pin;
//	button.base.gpio.port = setup.gpio.port;
//	button.base.nvic.irqn = setup.nvic.irqn;
//	button.base.nvic.setup = setup.nvic.setup;
//	button.base.exti.pin_src = setup.exti.pin_src;
//	button.base.exti.port_src = setup.exti.port_src;
	button.base = setup;

	// state initialization
	button._state.pushed_flag = 0;

	// based on a given trigger, detect the active state (during push)
	if ( setup._exti.setup.EXTI_Trigger == EXTI_Trigger_Rising ) {
		button._state.active_state = 1;
	} else if ( setup._exti.setup.EXTI_Trigger == EXTI_Trigger_Falling ) {
		button._state.active_state = 0;
	} else {
		/* ERROR - no other trigger allowed */
	}

//	// interrupt-switching functions
//	button.SetNvicState = SOOL_Periph_GPIO_PinConfig_NvicSwitch;
//	button.SetExtiState = SOOL_Periph_GPIO_PinConfig_ExtiSwitch;

	// other functions
	button.GetPushedFlag = Button_GetPushedFlag;
	button.GetCurrentState = Button_GetCurrentState;
	button._InterruptHandler = Button_InterruptHandler;

	return (button);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static uint8_t Button_GetPushedFlag(volatile SOOL_Button *button_ptr) {
	uint8_t temp = button_ptr->_state.pushed_flag;
	button_ptr->_state.pushed_flag = 0;
	return (temp);
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static uint8_t Button_GetCurrentState(const volatile SOOL_Button *button_ptr) {
	return (GPIO_ReadInputDataBit(button_ptr->base._gpio.port, button_ptr->base._gpio.pin));
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static uint8_t Button_InterruptHandler(volatile SOOL_Button *button_ptr) {

//	if ( EXTI_GetITStatus(button_ptr->base._exti.setup.EXTI_Line) == RESET ) {
//		// interrupt request on different EXTI Line
//		return (0);
//	}

	/* check current state */
	if ( GPIO_ReadInputDataBit(button_ptr->base._gpio.port, button_ptr->base._gpio.pin) == button_ptr->_state.active_state ) {
		button_ptr->_state.pushed_flag = 1;
	}

//	/* clear interrupt flag - on purpose placed after state check */
//	EXTI_ClearITPendingBit(button_ptr->base.exti.line);

	return (1);

}
