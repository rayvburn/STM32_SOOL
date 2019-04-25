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

// pin switcher is not interrupt-driven
typedef struct SoolPinConfigNoInt PinSwitchSetup;

// declare a structure and typedef it
struct PinSwitchStruct;
typedef struct PinSwitchStruct PinSwitch;

// define a structure
struct PinSwitchStruct {
	PinSwitchSetup	setup;
	void (*SetHigh)(const PinSwitch*);
	void (*SetLow)(const PinSwitch*);
};

// - - - - - - - - - - -

PinSwitch SOOL_PinSwitch_Initialize(PinSwitchSetup *setup_ptr);

// - - - - - - - - - - -

#endif /* EFFECTORS_PINSWITCH_PINSWITCH_H_ */
