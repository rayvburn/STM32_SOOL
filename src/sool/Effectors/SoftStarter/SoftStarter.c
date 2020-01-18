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
static uint16_t SoftStarter_Get(SOOL_SoftStarter* ss_ptr);

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

	/* Evaluate correctness */
	if ( pulse_end < 0 || pulse_start < 0 ) {
		/* ERROR */
		return (0);
	} else if ( pulse_end == pulse_start ) {
		/* Does not make sense - but sometimes Reconfigure is called when the motor is stopped */
		// set `_config` to prevent any change (as required based on the `pulse_start` and `pulse_end`
		ss_ptr->_config.increments = 0;
		ss_ptr->_config.pulse_start = pulse_start;
		// set `_state`
		ss_ptr->_state.changes_left = 0;
		ss_ptr->_state.pulse_last = pulse_end;
		return (0);
	}

	/* Fill the state structure */
	ss_ptr->_state.pulse_last = pulse_start;
	ss_ptr->_state.changes_left = 0; // temporary
	ss_ptr->_state.time_last_pulse_change = 0; // does not matter at that moment

	/* Fill the setup structure */
	ss_ptr->_setup.pulse_change = +1;

	/* Check which pulse value is bigger to properly calculate the change of the pulse */
	uint16_t pulse_bigger, pulse_smaller = 0;
	if ( pulse_end > pulse_start ) {
		pulse_bigger = pulse_end;
		pulse_smaller = pulse_start;
	} else {
		pulse_bigger = pulse_start;
		pulse_smaller = pulse_end;
	}

	/* Check feasibility of the soft-starting procedure (with the hard-coded time resolution) */
	if ( (pulse_bigger - pulse_smaller) > duration ) {

		/* Cannot make changes as fast as required, but can still process the soft-start
		 * as fast as possible. The `pulse_change` will be adjusted properly */
		ss_ptr->_setup.time_change_gap = 1;

		/* Calculate pulse change */
		ss_ptr->_setup.pulse_change = (pulse_bigger - pulse_smaller) / duration; // must be > 1

		/* Set the number of increments/decrements */
		ss_ptr->_state.changes_left = duration;

	} else {

		/* (pulse_bigger - pulse_smaller) <= duration */

		/* Normal operation - single increment of the `pulse` in each event. */
		ss_ptr->_setup.time_change_gap = duration / (pulse_bigger - pulse_smaller);

		/* Pulse change -> +1 or -1 */

		/* Set the number of increments/decrements */
		ss_ptr->_state.changes_left = pulse_bigger - pulse_smaller;

	}

	/* Adjust the sign of the `pulse change` - if needed */
	if ( pulse_end < pulse_start ) {
		ss_ptr->_setup.pulse_change = -ss_ptr->_setup.pulse_change;
	}

	/* Fill the config structure */
	ss_ptr->_config.increments = ss_ptr->_state.changes_left;
	ss_ptr->_config.pulse_start = pulse_start;

	return (1);

}

// --------------------------------------------------------------

static void SoftStarter_Start(SOOL_SoftStarter* ss_ptr) {
	ss_ptr->_state.time_last_pulse_change = SOOL_Periph_TIM_SysTick_GetMillis();
	ss_ptr->_state.changes_left = ss_ptr->_config.increments;
	ss_ptr->_state.pulse_last = ss_ptr->_config.pulse_start;
}

// --------------------------------------------------------------

static uint8_t SoftStarter_Process(SOOL_SoftStarter* ss_ptr) {

	if ( (SOOL_Periph_TIM_SysTick_GetMillis() - ss_ptr->_state.time_last_pulse_change) >= ss_ptr->_setup.time_change_gap ) {

		/* Check if finished */
		if ( ss_ptr->_state.changes_left == 0 ) {
			return (0);
		}

		/* Update timestamp */
		ss_ptr->_state.time_last_pulse_change = SOOL_Periph_TIM_SysTick_GetMillis();
		ss_ptr->_state.changes_left--;
		ss_ptr->_state.pulse_last += ss_ptr->_setup.pulse_change;
		return (1);

	}
	return (0);

}

// --------------------------------------------------------------

static uint8_t SoftStarter_IsFinished(const SOOL_SoftStarter* ss_ptr) {

	if ( ss_ptr->_state.changes_left == 0 ) {
		return (1);
	}
	return (0);

}

// --------------------------------------------------------------

static uint16_t SoftStarter_Get(SOOL_SoftStarter* ss_ptr) {
	return (ss_ptr->_state.pulse_last);
}

// --------------------------------------------------------------
