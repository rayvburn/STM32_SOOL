/*
 * PinSwitch.h
 *
 *  Created on: 25.04.2019
 *      Author: user
 */

#ifndef EFFECTORS_PINSWITCH_PINSWITCH_H_
#define EFFECTORS_PINSWITCH_PINSWITCH_H_

// - - - - - - - - - - -

#include <sool/Peripherals/GPIO/PinConfig.h>

#include "stm32f10x.h"

// - - - - - - - - - - -

// forward declare a structure and typedef it to further make use of it inside a structure
struct _SOOL_PinSwitchStruct;
typedef struct _SOOL_PinSwitchStruct SOOL_PinSwitch;

// define a structure
struct _SOOL_PinSwitchStruct {
	SOOL_PinConfigNoInt 	_setup;				// pin switcher is not interrupt-driven
	void (*SetHigh)(const SOOL_PinSwitch*);
	void (*SetLow)(const SOOL_PinSwitch*);
};

// - - - - - - - - - - -

SOOL_PinSwitch SOOL_Effectors_PinSwitch_Init(SOOL_PinConfigNoInt setup);

// - - - - - - - - - - -

#endif /* EFFECTORS_PINSWITCH_PINSWITCH_H_ */
