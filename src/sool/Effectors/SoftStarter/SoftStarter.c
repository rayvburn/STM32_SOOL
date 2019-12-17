/*
 * SoftStarter.c
 *
 *  Created on: 17.12.2019
 *      Author: user
 */

#include <sool/Effectors/SoftStarter/SoftStarter.h>
#include <sool/Peripherals/TIM/SystickTimer.h>

static void SoftStarter_Start(SOOL_SoftStarter* ss_ptr);
static uint8_t SoftStarter_Process(SOOL_SoftStarter* ss_ptr);
static uint8_t SoftStarter_IsFinished(const SOOL_SoftStarter* ss_ptr);
static uint16_t SoftStarter_Get(SOOL_SoftStarter* ss_ptr);

// --------------------------------------------------------------

SOOL_SoftStarter SOOL_Effector_SoftStarter_Initialize(uint16_t pulse_start, uint16_t pulse_end, uint32_t duration) {

	/* Newly created instance */
	SOOL_SoftStarter ss;

	/* Evaluate correctness */
	if ( pulse_end < 0 || pulse_start < 0 ) {
		/* ERROR */
		return (ss);
	} else if ( pulse_end == pulse_start ) {
		/* Does not make sense */
		return (ss);
	}

	/* Fill the state structure */
	ss._state.pulse_last = pulse_start;
	ss._state.changes_left = 0; // temporary
	ss._state.time_last_pulse_change = 0; // does not matter at that moment

	/* Fill the setup structure */
	ss._setup.pulse_change = +1;

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
		ss._setup.time_change_gap = 1;

		/* Calculate pulse change */
		ss._setup.pulse_change = (pulse_bigger - pulse_smaller) / duration; // must be > 1

		/* Set the number of increments/decrements */
		ss._state.changes_left = duration;

	} else {

		/* (pulse_bigger - pulse_smaller) <= duration */

		/* Normal operation - single increment of the `pulse` in each event. */
		ss._setup.time_change_gap = duration / (pulse_bigger - pulse_smaller);

		/* Pulse change -> +1 or -1 */

		/* Set the number of increments/decrements */
		ss._state.changes_left = pulse_bigger - pulse_smaller;

	}

	/* Adjust the sign of the `pulse change` - if needed */
	if ( pulse_end < pulse_start ) {
		ss._setup.pulse_change = -ss._setup.pulse_change;
	}

	/* Fill the config structure */
	ss._config.increments = ss._state.changes_left;
	ss._config.pulse_start = pulse_start;

	/* Method pointers */
	ss.Get = SoftStarter_Get;
	ss.IsFinished = SoftStarter_IsFinished;
	ss.Process = SoftStarter_Process;
	ss.Start = SoftStarter_Start;

	return (ss);

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
