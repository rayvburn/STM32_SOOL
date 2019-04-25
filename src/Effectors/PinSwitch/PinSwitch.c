/*
 * PinSwitch.c
 *
 *  Created on: 25.04.2019
 *      Author: user
 */

#include "include/Effectors/PinSwitch/PinSwitch.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static void SOOL_PinSwitch_SetHigh(const PinSwitch *obj_ptr);
static void SOOL_PinSwitch_SetLow (const PinSwitch *obj_ptr);
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

PinSwitch SOOL_PinSwitch_Init(SOOL_PinConfigNoInt setup) {

	PinSwitch obj;
	obj.setup = setup;
	obj.SetHigh = SOOL_PinSwitch_SetHigh;
	obj.SetLow = SOOL_PinSwitch_SetLow;
	return (obj);

}

// =============================================================================================

static void SOOL_PinSwitch_SetHigh(const PinSwitch *obj_ptr) {
	GPIO_SetBits(obj_ptr->setup.gpio_port, obj_ptr->setup.gpio_pin);
}

static void SOOL_PinSwitch_SetLow (const PinSwitch *obj_ptr) {
	GPIO_ResetBits(obj_ptr->setup.gpio_port, obj_ptr->setup.gpio_pin);
}

