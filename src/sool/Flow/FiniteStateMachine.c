/*
 * FiniteStateMachine.c
 *
 *  Created on: 21.12.2018
 *      Author: user
 */

#include "sool/Flow/FiniteStateMachine.h"

// initial conditions
static uint8_t state_changed = 1;
static FSMStates last_state = BULLET_STOPPED;
static FSMStates current_state = IDLE_BLINKING;
static uint8_t min_duration = 0;
static uint32_t state_start_time = 0;
//

void SFM_SwitchToState(FSMStates state_to_get_into) {
	last_state = current_state;
	current_state = state_to_get_into;
	state_changed = 1;
	if ( min_duration != 0 ) {	// if duration is set to 0 then limit is not valid
		state_start_time = SysTick_GetTenthsOfSec();
	}
}

void SFM_SetMinStateDuration(uint32_t tithings_of_sec) {
	// useful for button-driven states switching
	min_duration = tithings_of_sec;
}

uint8_t SFM_GetStateTransitionFlag()  {
	uint8_t temp = state_changed;
	state_changed = 0;
	return temp;
}

FSMStates SFM_GetLastState() {
	return last_state;
}

FSMStates SFM_GetCurrentState() {
	return current_state;
}

uint8_t SFM_IsTimingTerminalConditionFulfilled() {

	if ( min_duration == 0 ) {
		return 1;
	}

	if ( (SysTick_GetTenthsOfSec() - state_start_time) >= min_duration ) {
		return 1;
	}

	return 0;

}
