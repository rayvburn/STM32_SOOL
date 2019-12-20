/*
 * BuzzerPlayer.c
 *
 *  Created on: 22.08.2019
 *      Author: user
 */

#include "sool/Effectors/Buzzer/BuzzerPlayer.h"
#include <sool/Peripherals/TIM/SystickTimer.h>

static uint8_t SOOL_Buzzer_SetMode(SOOL_Buzzer *buzz_ptr, SOOL_Buzzer_Mode mode);
static uint8_t SOOL_Buzzer_Play(SOOL_Buzzer *buzz_ptr);

// private
static uint8_t SOOL_Buzzer_Single(SOOL_Buzzer *buzz_ptr, uint32_t duration);
static uint8_t SOOL_Buzzer_Double(SOOL_Buzzer *buzz_ptr, uint32_t duration);
static uint8_t SOOL_Buzzer_Warning(SOOL_Buzzer *buzz_ptr, uint32_t duration);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

SOOL_Buzzer SOOL_Effector_Buzzer_Init(SOOL_PinConfig_NoInt setup) {

	// create new buzzer instance
	SOOL_Buzzer buzzer;

	// initialize PinSwitch instance
	buzzer.base = SOOL_Effector_PinSwitch_Init(setup);

	// initialize ActionTimer instance
	buzzer.base_tim = SOOL_Workflow_ActionTimer_Init();

	// save initial setup
	buzzer._setup.status = 0;
	buzzer._setup.mode = SOOL_BUZZER_MODE_IDLE;

	// save method pointers
	buzzer.SetMode = SOOL_Buzzer_SetMode;
	buzzer.Play = SOOL_Buzzer_Play;

	return (buzzer);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t SOOL_Buzzer_SetMode(SOOL_Buzzer *buzz_ptr, SOOL_Buzzer_Mode mode) {

	buzz_ptr->_setup.mode = (uint8_t)mode;
	uint32_t millis = SOOL_Periph_TIM_SysTick_GetMillis();

	switch (mode) {

	case(SOOL_BUZZER_MODE_SINGLE):
			buzz_ptr->base_tim.SetStartTime(&buzz_ptr->base_tim, millis);
			break;

	case(SOOL_BUZZER_MODE_DOUBLE):
			buzz_ptr->base_tim.SetStartTime(&buzz_ptr->base_tim, millis);
			break;

	case(SOOL_BUZZER_MODE_WARNING):
			buzz_ptr->base_tim.SetStartTime(&buzz_ptr->base_tim, millis);
			break;

	default:
			return (0);
			break;

	}

	buzz_ptr->_setup.status = 1; // prepared
	return (1);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t SOOL_Buzzer_Play(SOOL_Buzzer *buzz_ptr) {

	uint32_t millis = SOOL_Periph_TIM_SysTick_GetMillis();

	// play only when status is 1
	if ( buzz_ptr->_setup.status == 1 ) {

		buzz_ptr->base_tim.SetEndTime(&buzz_ptr->base_tim, millis);
		uint32_t duration = buzz_ptr->base_tim.GetTimeDiff(&buzz_ptr->base_tim);

		// execute correct function related to the current mode
		switch (buzz_ptr->_setup.mode) {

		case(SOOL_BUZZER_MODE_SINGLE):
				return (SOOL_Buzzer_Single(buzz_ptr, duration));
				break;

		case(SOOL_BUZZER_MODE_DOUBLE):
				return (SOOL_Buzzer_Double(buzz_ptr, duration));
				break;

		case(SOOL_BUZZER_MODE_WARNING):
				return (SOOL_Buzzer_Warning(buzz_ptr, duration));
				break;

		default:
				return (0);

		}

	}
	return (0);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t SOOL_Buzzer_Single(SOOL_Buzzer *buzz_ptr, uint32_t duration) {

	if ( duration <= 1000 ) {
		buzz_ptr->base.SetHigh(&buzz_ptr->base);
	} else if ( duration <= 2000 ) { // delay before possible next repetition
		buzz_ptr->base.SetLow(&buzz_ptr->base);
	} else {
		buzz_ptr->base.SetLow(&buzz_ptr->base); // just in case
		buzz_ptr->_setup.status = 0;
		return (0);
	}
	return (1);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t SOOL_Buzzer_Double(SOOL_Buzzer *buzz_ptr, uint32_t duration) {

	if ( duration <= 750 ) {
		buzz_ptr->base.SetHigh(&buzz_ptr->base);
	} else if ( duration <= 1500 ) {
		buzz_ptr->base.SetLow(&buzz_ptr->base);
	} else if ( duration <= 2250 ) {
		buzz_ptr->base.SetHigh(&buzz_ptr->base);
	} else if ( duration <= 3000 ) { // delay before possible next repetition
		buzz_ptr->base.SetLow(&buzz_ptr->base);
	} else {
		buzz_ptr->base.SetLow(&buzz_ptr->base); // just in case
		buzz_ptr->_setup.status = 0;
		return (0);
	}
	return (1);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t SOOL_Buzzer_Warning(SOOL_Buzzer *buzz_ptr, uint32_t duration) {

	// first ticking section
	if ( duration <= 100 ) {
		buzz_ptr->base.SetHigh(&buzz_ptr->base);
	} else if ( duration <= 200 ) {
		buzz_ptr->base.SetLow(&buzz_ptr->base);
	} else if ( duration <= 300 ) {
		buzz_ptr->base.SetHigh(&buzz_ptr->base);
	} else if ( duration <= 400 ) {
		buzz_ptr->base.SetLow(&buzz_ptr->base);
	} else if ( duration <= 500 ) {
		buzz_ptr->base.SetHigh(&buzz_ptr->base);
	} else if ( duration <= 1500 ) {
		buzz_ptr->base.SetLow(&buzz_ptr->base);

	// second ticking section
	} else if ( duration <= 1600 ) {
		buzz_ptr->base.SetHigh(&buzz_ptr->base);
	} else if ( duration <= 1700 ) {
		buzz_ptr->base.SetLow(&buzz_ptr->base);
	} else if ( duration <= 1800 ) {
		buzz_ptr->base.SetHigh(&buzz_ptr->base);
	} else if ( duration <= 1900 ) {
		buzz_ptr->base.SetLow(&buzz_ptr->base);
	} else if ( duration <= 2000 ) {
		buzz_ptr->base.SetHigh(&buzz_ptr->base);

	// finish
	} else if ( duration <= 2100 ) { // delay before possible next repetition
		buzz_ptr->base.SetLow(&buzz_ptr->base);
	} else {
		buzz_ptr->base.SetLow(&buzz_ptr->base); // just in case
		buzz_ptr->_setup.status = 0;
		return (0);
	}

	return (1);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
