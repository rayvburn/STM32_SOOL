/*
 * SoftStarter_common.c
 *
 *  Created on: 06.02.2020
 *      Author: user
 */

#include <sool/Effectors/SoftStarter/SoftStarter_common.h>

// --------------------------------------------------------------

uint8_t SOOL_Effector_SoftStarter_Reconfigure(struct _SOOL_SoftStarterConfigStruct* config_ptr,
		struct _SOOL_SoftStarterSetupStruct* setup_ptr, struct _SOOL_SoftStarterStateStruct* state_ptr,
		uint16_t pulse_start, uint16_t pulse_end, uint32_t duration) {

	/* Evaluate correctness */
	if ( pulse_end < 0 || pulse_start < 0 ) {
		/* ERROR */
		return (0);
	} else if ( pulse_end == pulse_start || duration == 0 ) {
		/* Does not make sense - but sometimes Reconfigure is called when the motor is stopped */
		// set `_config` to prevent any change (as required based on the `pulse_start` and `pulse_end`
		config_ptr->increments = 0;
		config_ptr->pulse_start = pulse_start;
		// set `_state`
		state_ptr->changes_left = 0;
		state_ptr->pulse_last = pulse_end;
		return (0);
	}

	/* Fill the state structure */
	state_ptr->pulse_last = pulse_start;
	state_ptr->changes_left = 0; // temporary
	state_ptr->time_last_pulse_change = 0; // does not matter at that moment

	/* Fill the setup structure */
	setup_ptr->pulse_change = +1;

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
		setup_ptr->time_change_gap = 1;

		/* Calculate pulse change */
		setup_ptr->pulse_change = (pulse_bigger - pulse_smaller) / duration; // must be > 1

		/* Set the number of increments/decrements */
		state_ptr->changes_left = duration;

	} else {

		/* (pulse_bigger - pulse_smaller) <= duration */

		/* Normal operation - single increment of the `pulse` in each event. */
		setup_ptr->time_change_gap = duration / (pulse_bigger - pulse_smaller);

		/* Pulse change -> +1 or -1 */

		/* Set the number of increments/decrements */
		state_ptr->changes_left = pulse_bigger - pulse_smaller;

	}

	/* Adjust the sign of the `pulse change` - if needed */
	if ( pulse_end < pulse_start ) {
		setup_ptr->pulse_change = -setup_ptr->pulse_change;
	}

	/* Fill the config structure */
	config_ptr->increments = state_ptr->changes_left;
	config_ptr->pulse_start = pulse_start;

	return (1);

}

// --------------------------------------------------------------

void SOOL_Effector_SoftStarter_Start(struct _SOOL_SoftStarterConfigStruct* config_ptr,
				struct _SOOL_SoftStarterSetupStruct* setup_ptr, struct _SOOL_SoftStarterStateStruct* state_ptr,
				uint32_t stamp) {

	state_ptr->time_last_pulse_change = stamp;
	state_ptr->changes_left = config_ptr->increments;
	state_ptr->pulse_last = config_ptr->pulse_start;

}

// --------------------------------------------------------------

uint8_t SOOL_Effector_SoftStarter_Process(struct _SOOL_SoftStarterSetupStruct* setup_ptr,
				struct _SOOL_SoftStarterStateStruct* state_ptr, uint32_t stamp) {

//	if ( (stamp - state_ptr->time_last_pulse_change) >= setup_ptr->time_change_gap ) {
	// compared to above, allow `down-counting` AND consider long-term operation (uin32_t flip)
	uint32_t time_diff = abs(SOOL_Workflow_Common_ComputeTimeDifference(state_ptr->time_last_pulse_change, stamp));
	if (time_diff >= setup_ptr->time_change_gap) {


		/* Check if finished */
		if ( state_ptr->changes_left == 0 ) {
			return (0);
		}

		/* Update timestamp */
		state_ptr->time_last_pulse_change = stamp;
		state_ptr->changes_left--;
		state_ptr->pulse_last += setup_ptr->pulse_change;
		return (1);

	}
	return (0);

}

// --------------------------------------------------------------

uint8_t SOOL_Effector_SoftStarter_IsFinished(const struct _SOOL_SoftStarterStateStruct* state_ptr) {

	if ( state_ptr->changes_left == 0 ) {
		return (1);
	}
	return (0);

}

// --------------------------------------------------------------

uint16_t SOOL_Effector_SoftStarter_Get(const struct _SOOL_SoftStarterStateStruct* state_ptr) {
	return (state_ptr->pulse_last);
}

// --------------------------------------------------------------
