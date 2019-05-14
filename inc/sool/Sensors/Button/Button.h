/*
 * Button.h
 *
 *  Created on: 12.05.2019
 *      Author: user
 */

#ifndef INC_SOOL_SENSORS_BUTTON_BUTTON_H_
#define INC_SOOL_SENSORS_BUTTON_BUTTON_H_

#include "sool/Common/PinConfig.h"
#include <stdint.h>

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

struct _SOOL_ButtonStateStruct {
	uint8_t 			pushed_flag;
	uint8_t				active_state; // whether a signal goes low or high on push
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

typedef struct _SOOL_ButtonStruct SOOL_Button;

struct _SOOL_ButtonStruct {

	SOOL_PinConfigInt 				_setup;
	struct _SOOL_ButtonStateStruct 	_state;

	uint8_t 	(*GetPushedFlag)(volatile SOOL_Button*); 							// interrupt-driven, clears the flag
	uint8_t 	(*GetCurrentState)(const volatile SOOL_Button*);
	void 		(*SetNvicState)(SOOL_PinConfigInt*, const FunctionalState);
	void 		(*SetExtiState)(SOOL_PinConfigInt*, const FunctionalState);
	uint8_t 	(*_InterruptHandler)(volatile SOOL_Button*); 						// routine fired in a proper ISR (firstly it must check if interrupt has been triggered on sensor's EXTI line)

};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

volatile SOOL_Button SOOL_Sensors_Button_Init(SOOL_PinConfigInt setup, const uint8_t active_state);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#endif /* INC_SOOL_SENSORS_BUTTON_BUTTON_H_ */
