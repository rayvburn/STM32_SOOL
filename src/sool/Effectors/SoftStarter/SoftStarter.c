/*
 * SoftStarter.c
 *
 *  Created on: 17.12.2019
 *      Author: user
 */

#include <sool/Effectors/SoftStarter/SoftStarter.h>
#include <sool/Peripherals/TIM/SystickTimer.h>

static uint8_t SoftStarter_Reconfigure(SOOL_SoftStarter* ss_ptr, uint16_t pulse_start, uint16_t pulse_end, uint32_t duration);
static void SoftStarter_Start(SOOL_SoftStarter* ss_ptr);
static uint8_t SoftStarter_Process(SOOL_SoftStarter* ss_ptr);
static uint8_t SoftStarter_IsFinished(const SOOL_SoftStarter* ss_ptr);
static uint16_t SoftStarter_Get(const SOOL_SoftStarter* ss_ptr);

// --------------------------------------------------------------

SOOL_SoftStarter SOOL_Effector_SoftStarter_Initialize(uint16_t pulse_start, uint16_t pulse_end, uint32_t duration) {

	/* Newly created instance */
	SOOL_SoftStarter ss;

	if ( !SoftStarter_Reconfigure(&ss, pulse_start, pulse_end, duration) ) {
		return (ss);
	}

	/* Method pointers */
	ss.Get = SoftStarter_Get;
	ss.IsFinished = SoftStarter_IsFinished;
	ss.Process = SoftStarter_Process;
	ss.Reconfigure = SoftStarter_Reconfigure;
	ss.Start = SoftStarter_Start;

	return (ss);

}

// --------------------------------------------------------------

static uint8_t SoftStarter_Reconfigure(SOOL_SoftStarter* ss_ptr, uint16_t pulse_start, uint16_t pulse_end, uint32_t duration) {
	return (SOOL_Effector_SoftStarter_Reconfigure(&ss_ptr->_config, &ss_ptr->_setup, &ss_ptr->_state, pulse_start, pulse_end, duration));
}

// --------------------------------------------------------------

static void SoftStarter_Start(SOOL_SoftStarter* ss_ptr) {
	SOOL_Effector_SoftStarter_Start(&ss_ptr->_config, &ss_ptr->_setup, &ss_ptr->_state, SOOL_Periph_TIM_SysTick_GetMillis());
}

// --------------------------------------------------------------

static uint8_t SoftStarter_Process(SOOL_SoftStarter* ss_ptr) {
	return (SOOL_Effector_SoftStarter_Process(&ss_ptr->_setup, &ss_ptr->_state, SOOL_Periph_TIM_SysTick_GetMillis()));
}

// --------------------------------------------------------------

static uint8_t SoftStarter_IsFinished(const SOOL_SoftStarter* ss_ptr) {
	return (SOOL_Effector_SoftStarter_IsFinished(&ss_ptr->_state));
}

// --------------------------------------------------------------

static uint16_t SoftStarter_Get(const SOOL_SoftStarter* ss_ptr) {
	return (SOOL_Effector_SoftStarter_Get(&ss_ptr->_state));
}
