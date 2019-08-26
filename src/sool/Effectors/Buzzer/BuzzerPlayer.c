/*
 * BuzzerPlayer.c
 *
 *  Created on: 22.08.2019
 *      Author: user
 */

#include "sool/Effectors/Buzzer/BuzzerPlayer.h"

static uint8_t SOOL_Buzzer_SetMode(SOOL_Buzzer *buzz_ptr, SOOL_Buzzer_Mode mode, uint32_t millis);
static uint8_t SOOL_Buzzer_Play(SOOL_Buzzer *buzz_ptr, uint32_t millis);

// private
static uint8_t SOOL_Buzzer_Single(SOOL_Buzzer *buzz_ptr, uint32_t millis);
static uint8_t SOOL_Buzzer_Double(SOOL_Buzzer *buzz_ptr, uint32_t millis);
static uint8_t SOOL_Buzzer_Warning(SOOL_Buzzer *buzz_ptr, uint32_t millis);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

SOOL_Buzzer SOOL_Effector_Buzzer_Init(SOOL_PinConfig_NoInt setup) {

	// create new buzzer instance
	SOOL_Buzzer buzzer;

	// initialize PinSwitch instance
	buzzer.base = SOOL_Effector_PinSwitch_Init(setup);

	// save initial setup
	buzzer._setup.start_time = 0;
	buzzer._setup.status = 0x00;

	// save method pointers
	buzzer.SetMode = SOOL_Buzzer_SetMode;
	buzzer.Play = SOOL_Buzzer_Play;

	return (buzzer);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t SOOL_Buzzer_SetMode(SOOL_Buzzer *buzz_ptr, SOOL_Buzzer_Mode mode, uint32_t millis) {

	buzz_ptr->_setup.mode = (uint8_t)mode;

	// TODO: millis + X can overflow, implement some kind of immunity

	switch (mode) {

	case(SOOL_BUZZER_MODE_SINGLE):
			buzz_ptr->_setup.start_time = millis;
			break;

	case(SOOL_BUZZER_MODE_DOUBLE):
			buzz_ptr->_setup.start_time = millis;
			break;

	case(SOOL_BUZZER_MODE_WARNING):
			buzz_ptr->_setup.start_time = millis;
			break;

	default:
			return (0);
			break;

	}

	buzz_ptr->_setup.status = 1; // prepared
	return (1);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t SOOL_Buzzer_Play(SOOL_Buzzer *buzz_ptr, uint32_t millis) {

	// play only when status is 1
	if ( buzz_ptr->_setup.status == 1 ) {

		// execute correct function related to the current mode
		switch (buzz_ptr->_setup.mode) {

		case(SOOL_BUZZER_MODE_SINGLE):
				return (SOOL_Buzzer_Single(buzz_ptr, millis));
				break;

		case(SOOL_BUZZER_MODE_DOUBLE):
				return (SOOL_Buzzer_Double(buzz_ptr, millis));
				break;

		case(SOOL_BUZZER_MODE_WARNING):
				return (SOOL_Buzzer_Warning(buzz_ptr, millis));
				break;

		default:
				return (0);

		}

	}
	return (0);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t SOOL_Buzzer_Single(SOOL_Buzzer *buzz_ptr, uint32_t millis) {

	if ( (millis - buzz_ptr->_setup.start_time) <= 1000 ) {
		buzz_ptr->base.SetHigh(&buzz_ptr->base);
	} else {
		buzz_ptr->base.SetLow(&buzz_ptr->base);
		buzz_ptr->_setup.status = 0;
		return (0);
	}
	return (1);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t SOOL_Buzzer_Double(SOOL_Buzzer *buzz_ptr, uint32_t millis) {

	if ( (millis - buzz_ptr->_setup.start_time) <= 750 ) {
		buzz_ptr->base.SetHigh(&buzz_ptr->base);
	} else if ( (millis - buzz_ptr->_setup.start_time) <= 1500 ) {
		buzz_ptr->base.SetLow(&buzz_ptr->base);
	} else if ( (millis - buzz_ptr->_setup.start_time) <= 2250 ) {
		buzz_ptr->base.SetHigh(&buzz_ptr->base);
	} else { // if ( (millis - buzz_ptr->_setup.start_time) >= 3000 ) {
		buzz_ptr->base.SetLow(&buzz_ptr->base);
		buzz_ptr->_setup.status = 0;
		return (0);
	}
	return (1);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t SOOL_Buzzer_Warning(SOOL_Buzzer *buzz_ptr, uint32_t millis) {

	// first ticking section
	if ( (millis - buzz_ptr->_setup.start_time) <= 100 ) {
		buzz_ptr->base.SetHigh(&buzz_ptr->base);
	} else if ( (millis - buzz_ptr->_setup.start_time) <= 200 ) {
		buzz_ptr->base.SetLow(&buzz_ptr->base);
	} else if ( (millis - buzz_ptr->_setup.start_time) <= 300 ) {
		buzz_ptr->base.SetHigh(&buzz_ptr->base);
	} else if ( (millis - buzz_ptr->_setup.start_time) <= 400 ) {
		buzz_ptr->base.SetLow(&buzz_ptr->base);
	} else if ( (millis - buzz_ptr->_setup.start_time) <= 500 ) {
		buzz_ptr->base.SetHigh(&buzz_ptr->base);
	} else if ( (millis - buzz_ptr->_setup.start_time) <= 1500 ) {
		buzz_ptr->base.SetLow(&buzz_ptr->base);

	// second ticking section
	} else if ( (millis - buzz_ptr->_setup.start_time) <= 1600 ) {
		buzz_ptr->base.SetHigh(&buzz_ptr->base);
	} else if ( (millis - buzz_ptr->_setup.start_time) <= 1700 ) {
		buzz_ptr->base.SetLow(&buzz_ptr->base);
	} else if ( (millis - buzz_ptr->_setup.start_time) <= 1800 ) {
		buzz_ptr->base.SetHigh(&buzz_ptr->base);
	} else if ( (millis - buzz_ptr->_setup.start_time) <= 1900 ) {
		buzz_ptr->base.SetLow(&buzz_ptr->base);
	} else if ( (millis - buzz_ptr->_setup.start_time) <= 2000 ) {
		buzz_ptr->base.SetHigh(&buzz_ptr->base);

	// finish
	} else { // if ( (millis - buzz_ptr->_setup.start_time) >= 2100 ) {
		buzz_ptr->base.SetLow(&buzz_ptr->base);
		buzz_ptr->_setup.status = 0;
		return (0);
	}

	return (1);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
