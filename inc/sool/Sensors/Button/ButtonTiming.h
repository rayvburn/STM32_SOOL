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
	SOOL_PinConfig_Int 				base_pin;
	SOOL_TimerBasic 				base_timer;

	// ----------- derived class section
	struct _SOOL_ButtonTimingStateStruct	_state;

	uint8_t		(*GetCurrentState)(volatile SOOL_ButtonTiming*);
	uint8_t		(*GetShortPressFlag)(volatile SOOL_ButtonTiming*);
	uint8_t 	(*GetLongPressFlag)(volatile SOOL_ButtonTiming*);

	uint8_t 	(*_ExtiInterruptHandler)(volatile SOOL_ButtonTiming*);
	uint8_t 	(*_TimerInterruptHandler)(volatile SOOL_ButtonTiming*);

};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// requires that SOOL_PinConfig_Int's trigger is set to Rising_Falling
extern volatile SOOL_ButtonTiming SOOL_Sensor_ButtonTiming_Init(SOOL_PinConfig_Int setup, uint8_t active_state, SOOL_TimerBasic timer);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


#endif /* INCLUDE_SENSORS_BUTTON_BUTTONTIMING_H_ */
