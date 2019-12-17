/*
 * ButtonTiming.h
 *
 *  Created on: 12.05.2019
 *      Author: user
 */

#ifndef INCLUDE_SENSORS_BUTTON_BUTTONTIMING_H_
#define INCLUDE_SENSORS_BUTTON_BUTTONTIMING_H_

#include "sool/Peripherals/GPIO/PinConfig_Int.h"
#include "sool/Peripherals/TIM/TimerBasic.h"

typedef struct _SOOL_ButtonTimingStruct SOOL_ButtonTiming;

struct _SOOL_ButtonTimingStateStruct {
	uint8_t 			timer_started;
	uint8_t 			short_push_flag;
	uint8_t 			long_push_flag;
	uint8_t				active_state; // whether a signal goes low or high on push
};

// no need to derive from SOOL_Button because it's interrupt handler must be modified anyway
struct _SOOL_ButtonTimingStruct {

	// ----------- base (internal) classes section
	SOOL_PinConfig_Int 					base_pin;
	SOOL_TimerBasic*					base_timer_ptr;

	// ----------- derived class section
	struct _SOOL_ButtonTimingStateStruct	_state;

	uint8_t		(*GetCurrentState)(volatile SOOL_ButtonTiming*);
	uint8_t		(*GetShortPressFlag)(volatile SOOL_ButtonTiming*);
	uint8_t 	(*GetLongPressFlag)(volatile SOOL_ButtonTiming*);

	uint8_t 	(*_ExtiInterruptHandler)(volatile SOOL_ButtonTiming*);
	uint8_t 	(*_TimerInterruptHandler)(volatile SOOL_ButtonTiming*);

};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/**
 * @brief Provides a short- and long-press detection for a button. Long press detection is determined
 * by a TimerBasic configuration (it's Update event generation frequency)
 * @note Requires SOOL_PinConfig_Int's trigger to be set to Rising_Falling
 * @note To operate it requires calling base_pin's EnableEXTI and base_timer's EnableNVIC
 * after initialization and passing objects to interrupt handlers
 * @note Example of use @ https://gitlab.com/frb-pow/002tubewaterflowmcu/blob/63200cd02eac11177d323c57a406d01d8ad62d96/src/main.c#L76
 * @param setup
 * @param active_state - state which button sets on its output when pressed, 1 if signal is high, 0 if low
 * @param timer_ptr - pointer to an existing instance of TimerBasic
 * @return
 */
extern volatile SOOL_ButtonTiming SOOL_Sensor_ButtonTiming_Init(SOOL_PinConfig_Int setup, uint8_t active_state, SOOL_TimerBasic *timer_ptr);

/**
 * @brief ButtonTiming instance startup routine
 * @param button_ptr
 */
extern void SOOL_Sensor_ButtonTiming_Startup(SOOL_ButtonTiming* button_ptr);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


#endif /* INCLUDE_SENSORS_BUTTON_BUTTONTIMING_H_ */
