/*
 * SoftStarterCustom.c
 *
 *  Created on: 06.02.2020
 *      Author: user
 */

#include <sool/Effectors/SoftStarter/SoftStarterCustom.h>
static uint8_t SoftStarterCustom_Reconfigure(SOOL_SoftStarterCustom* ss_ptr, uint16_t pulse_start, uint16_t pulse_end, uint32_t duration);
static void SoftStarterCustom_Start(SOOL_SoftStarterCustom* ss_ptr, uint32_t stamp);
static uint8_t SoftStarterCustom_Process(SOOL_SoftStarterCustom* ss_ptr, uint32_t stamp);
static uint8_t SoftStarterCustom_IsFinished(const SOOL_SoftStarterCustom* ss_ptr);
static uint16_t SoftStarterCustom_Get(const SOOL_SoftStarterCustom* ss_ptr);

// --------------------------------------------------------------

SOOL_SoftStarterCustom SOOL_Effector_SoftStarterCustom_Initialize(uint16_t pulse_start, uint16_t pulse_end, uint32_t duration) {

	/* Newly created instance */
	SOOL_SoftStarterCustom ss;

	if ( !SoftStarterCustom_Reconfigure(&ss, pulse_start, pulse_end, duration) ) {
		return (ss);
	}

	/* Method pointers */
	ss.Get = SoftStarterCustom_Get;
	ss.IsFinished = SoftStarterCustom_IsFinished;
	ss.Process = SoftStarterCustom_Process;
	ss.Reconfigure = SoftStarterCustom_Reconfigure;
	ss.Start = SoftStarterCustom_Start;

	return (ss);

}

// --------------------------------------------------------------

static uint8_t SoftStarterCustom_Reconfigure(SOOL_SoftStarterCustom* ss_ptr, uint16_t pulse_start, uint16_t pulse_end, uint32_t duration) {
	return (SOOL_Effector_SoftStarter_Reconfigure(&ss_ptr->_config, &ss_ptr->_setup, &ss_ptr->_state, pulse_start, pulse_end, duration));
}

// --------------------------------------------------------------

static void SoftStarterCustom_Start(SOOL_SoftStarterCustom* ss_ptr, uint32_t stamp) {
	SOOL_Effector_SoftStarter_Start(&ss_ptr->_config, &ss_ptr->_setup, &ss_ptr->_state, stamp);
}

// --------------------------------------------------------------

static uint8_t SoftStarterCustom_Process(SOOL_SoftStarterCustom* ss_ptr, uint32_t stamp) {
	return (SOOL_Effector_SoftStarter_Process(&ss_ptr->_setup, &ss_ptr->_state, stamp));
}

// --------------------------------------------------------------

static uint8_t SoftStarterCustom_IsFinished(const SOOL_SoftStarterCustom* ss_ptr) {
	return (SOOL_Effector_SoftStarter_IsFinished(&ss_ptr->_state));
}

// --------------------------------------------------------------

static uint16_t SoftStarterCustom_Get(const SOOL_SoftStarterCustom* ss_ptr) {
	return (SOOL_Effector_SoftStarter_Get(&ss_ptr->_state));
}
