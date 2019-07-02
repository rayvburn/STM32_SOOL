/*
 * PinSwitch.h
 *
 *  Created on: 25.04.2019
 *      Author: user
 */

#ifndef EFFECTORS_PINSWITCH_PINSWITCH_H_
#define EFFECTORS_PINSWITCH_PINSWITCH_H_

// - - - - - - - - - - -

#include <sool/Peripherals/GPIO/PinConfig_NoInt.h>
#include <sool/Peripherals/GPIO/PinConfig_AltFunction.h>

#include "stm32f10x.h"

// - - - - - - - - - - -

// forward declare a structure and typedef it to further make use of it inside a structure
struct _SOOL_PinSwitchStruct;
typedef struct _SOOL_PinSwitchStruct SOOL_PinSwitch;

// define a structure
struct _SOOL_PinSwitchStruct {
	SOOL_PinConfig_NoInt 	base;				// pin switcher is not interrupt-driven
	void (*SetHigh)(const SOOL_PinSwitch*);
	void (*SetLow)(const SOOL_PinSwitch*);
};

// - - - - - - - - - - -

extern SOOL_PinSwitch SOOL_Effector_PinSwitch_Init(SOOL_PinConfig_NoInt setup);
extern SOOL_PinSwitch SOOL_Effector_PinSwitch_InitAlt(SOOL_PinConfig_AltFunction setup);

// - - - - - - - - - - -

#endif /* EFFECTORS_PINSWITCH_PINSWITCH_H_ */
