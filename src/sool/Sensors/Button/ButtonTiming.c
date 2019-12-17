/*
 * ButtonTiming.c
 *
 *  Created on: 08.07.2019
 *      Author: user
 */

#include "sool/Sensors/Button/ButtonTiming.h"
#include "sool/Peripherals/GPIO/GPIO_common.h"

static uint8_t SOOL_ButtonTiming_GetCurrentState(volatile SOOL_ButtonTiming *button_tim_ptr);
static uint8_t SOOL_ButtonTiming_GetShortPressFlag(volatile SOOL_ButtonTiming *button_tim_ptr);
static uint8_t SOOL_ButtonTiming_GetLongPressFlag(volatile SOOL_ButtonTiming *button_tim_ptr);
static uint8_t SOOL_ButtonTiming_ExtiInterruptHandler(volatile SOOL_ButtonTiming *button_tim_ptr);
static uint8_t SOOL_ButtonTiming_TimerInterruptHandler(volatile SOOL_ButtonTiming *button_tim_ptr);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

volatile SOOL_ButtonTiming SOOL_Sensor_ButtonTiming_Init(SOOL_PinConfig_Int setup,
									  uint8_t active_state, SOOL_TimerBasic *timer_ptr) {

	/* Create a new instance */
	volatile SOOL_ButtonTiming button_tim;

	/* Only Rising_Falling trigger is allowed, otherwise timer cannot be used */
	if ( setup._exti.setup.EXTI_Trigger != EXTI_Trigger_Rising_Falling ) {
		return (button_tim);
	}

	/* Save base `class` */
	button_tim.base_pin = setup;

	/* Save timer instance */
	button_tim.base_timer_ptr = timer_ptr;

	/* Set internal state */
	button_tim._state.long_push_flag = 0;
	button_tim._state.short_push_flag = 0;
	button_tim._state.timer_started = 0;
	button_tim._state.active_state = active_state;

	/* Update function pointers */
	button_tim.GetCurrentState = SOOL_ButtonTiming_GetCurrentState;
	button_tim.GetShortPressFlag = SOOL_ButtonTiming_GetShortPressFlag;
	button_tim.GetLongPressFlag = SOOL_ButtonTiming_GetLongPressFlag;
	button_tim._ExtiInterruptHandler = SOOL_ButtonTiming_ExtiInterruptHandler;
	button_tim._TimerInterruptHandler = SOOL_ButtonTiming_TimerInterruptHandler;

	return (button_tim);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void SOOL_Sensor_ButtonTiming_Startup(SOOL_ButtonTiming* button_ptr) {
	button_ptr->base_pin.EnableEXTI(button_ptr->base_pin);
	button_ptr->base_timer_ptr->EnableNVIC(button_ptr->base_timer_ptr);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t SOOL_ButtonTiming_GetCurrentState(volatile SOOL_ButtonTiming *button_tim_ptr) {
	return (SOOL_Periph_GPIO_ReadInputDataBit(button_tim_ptr->base_pin._gpio.port, button_tim_ptr->base_pin._gpio.pin));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t SOOL_ButtonTiming_GetShortPressFlag(volatile SOOL_ButtonTiming *button_tim_ptr) {
	uint8_t temp = button_tim_ptr->_state.short_push_flag;
	button_tim_ptr->_state.short_push_flag = 0;
	return (temp);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t SOOL_ButtonTiming_GetLongPressFlag(volatile SOOL_ButtonTiming *button_tim_ptr) {
	uint8_t temp = button_tim_ptr->_state.long_push_flag;
	button_tim_ptr->_state.long_push_flag = 0;
	return (temp);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t SOOL_ButtonTiming_ExtiInterruptHandler(volatile SOOL_ButtonTiming *button_tim_ptr) {

	/* Check the current state of the button */
	if ( SOOL_Periph_GPIO_ReadInputDataBit(button_tim_ptr->base_pin._gpio.port, button_tim_ptr->base_pin._gpio.pin) == button_tim_ptr->_state.active_state ) {

		// button is in active state (just got pressed)

		/* Start counting time */
		button_tim_ptr->base_timer_ptr->Stop(button_tim_ptr->base_timer_ptr);
		button_tim_ptr->base_timer_ptr->SetCounter(button_tim_ptr->base_timer_ptr, 0);
		button_tim_ptr->base_timer_ptr->Start(button_tim_ptr->base_timer_ptr);

		/* Update internal state */
		button_tim_ptr->_state.timer_started = 1;
		button_tim_ptr->_state.long_push_flag = 0;
		button_tim_ptr->_state.short_push_flag = 0;

		return (1);

	} else {

		// button is in idle state (just got released)

		/* Check whether counting currently - if yes then button has been released before
		 * `timeout` elapsed */
		if ( button_tim_ptr->_state.timer_started ) {

			// stop the timer
			button_tim_ptr->base_timer_ptr->Stop(button_tim_ptr->base_timer_ptr);
			button_tim_ptr->_state.timer_started = 0;

			// set short press flag
			button_tim_ptr->_state.short_push_flag = 1;

			return (1);
		}

	}

	return (0);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t SOOL_ButtonTiming_TimerInterruptHandler(volatile SOOL_ButtonTiming *button_tim_ptr) {

	/* This check prevents calling ISR just after NVIC initialization */
	if ( button_tim_ptr->_state.timer_started ) {

		/* This will only be executed when button has not been released before `long-press-time` elapsed */
		// update internal state
		button_tim_ptr->_state.long_push_flag = 1;
		button_tim_ptr->_state.short_push_flag = 0; // just to be sure

		// turn the timer off
		button_tim_ptr->base_timer_ptr->Stop(button_tim_ptr->base_timer_ptr);
		button_tim_ptr->_state.timer_started = 0;

		return (1);
	}

	return (0);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
