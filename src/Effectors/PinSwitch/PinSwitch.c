/*
 * PinSwitch.c
 *
 *  Created on: 25.04.2019
 *      Author: user
 */

#include "PinSwitch.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static void SOOL_PinSwitch_SetHigh(const PinSwitchSetup *setup);
static void SOOL_PinSwitch_SetLow (const PinSwitchSetup *setup);
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

PinSwitch SOOL_PinSwitch_Initialize(PinSwitchSetup *setup_ptr) {

	PinSwitch obj;
	obj.setup = *setup_ptr;
	obj.SetHigh = SOOL_PinSwitch_SetHigh;
	obj.SetLow = SOOL_PinSwitch_SetLow;
	return (obj);

}

// =============================================================================================

static void SOOL_PinSwitch_SetHigh(const PinSwitchSetup *setup) {
	GPIO_SetBits(setup->gpio_port, setup->gpio_pin);
}

static void SOOL_PinSwitch_SetLow (const PinSwitchSetup *setup) {
	GPIO_ResetBits(setup->gpio_port, setup->gpio_pin);
}

