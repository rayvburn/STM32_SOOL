/*
 * PinSwitch.c
 *
 *  Created on: 25.04.2019
 *      Author: user
 */

#include "sool/Effectors/PinSwitch/PinSwitch.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static void SOOL_PinSwitch_SetHigh(const SOOL_PinSwitch *obj_ptr);
static void SOOL_PinSwitch_SetLow (const SOOL_PinSwitch *obj_ptr);
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

SOOL_PinSwitch SOOL_Effector_PinSwitch_Init(SOOL_PinConfig_NoInt setup) {

	SOOL_PinSwitch obj;
	obj._setup = setup;
	obj.SetHigh = SOOL_PinSwitch_SetHigh;
	obj.SetLow = SOOL_PinSwitch_SetLow;
	return (obj);

}

// =============================================================================================

static void SOOL_PinSwitch_SetHigh(const SOOL_PinSwitch *obj_ptr) {
	GPIO_SetBits(obj_ptr->_setup.gpio.port, obj_ptr->_setup.gpio.pin);
}

static void SOOL_PinSwitch_SetLow (const SOOL_PinSwitch *obj_ptr) {
	GPIO_ResetBits(obj_ptr->_setup.gpio.port, obj_ptr->_setup.gpio.pin);
}

