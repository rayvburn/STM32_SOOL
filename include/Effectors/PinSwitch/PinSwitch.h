/*
 * PinSwitch.h
 *
 *  Created on: 25.04.2019
 *      Author: user
 */

#ifndef EFFECTORS_PINSWITCH_PINSWITCH_H_
#define EFFECTORS_PINSWITCH_PINSWITCH_H_

// - - - - - - - - - - -

#include "stm32f10x.h"
#include "include/Common/PinConfig.h"

// - - - - - - - - - - -

// forward declare a structure and typedef it to further make use of it inside a structure
struct PinSwitchStruct;
typedef struct PinSwitchStruct PinSwitch;

// define a structure
struct PinSwitchStruct {
	SOOL_PinConfigNoInt setup;				// pin switcher is not interrupt-driven
	void (*SetHigh)(const PinSwitch*);
	void (*SetLow)(const PinSwitch*);
};

// - - - - - - - - - - -

PinSwitch SOOL_PinSwitch_Init(SOOL_PinConfigNoInt setup);

// - - - - - - - - - - -

#endif /* EFFECTORS_PINSWITCH_PINSWITCH_H_ */
