/*
 * FiniteStateMachine.c
 *
 *  Created on: 21.12.2018
 *      Author: user
 */

#include <sool/Workflow/FiniteStateMachine.h>

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t FSM_SwitchToState(SOOL_FSM *fsm, uint8_t state_id);
static void FSM_SetMinStateDuration(SOOL_FSM *fsm, uint32_t ms);		/* useful when states are switched via push-buttons */
static uint8_t FSM_GetCurrentState(SOOL_FSM *fsm);						/* to be used in main's while(1) to choose proper handler */
static uint8_t FSM_GetStateTransitionFlag(SOOL_FSM *fsm); 				/* useful for handling events after state switch */
static uint8_t FSM_IsTimingTerminalConditionFulfilled(SOOL_FSM *fsm);
static uint8_t FSM_IsTerminalConditionFulfilled(SOOL_FSM *fsm, uint8_t predicate);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

SOOL_FSM SOOL_Workflow_FSM_Init(uint8_t init_state_id, uint8_t execute_on_entry, uint32_t min_duration_ms) {

	/* Object to be returned */
	SOOL_FSM fsm;

	/* Set state structure fields */
	fsm._state.start_time = 0;
	fsm._state.MIN_DURATION = min_duration_ms;
	fsm._state.current = init_state_id;
	fsm._state.last = init_state_id;
	(execute_on_entry > 0) ? (fsm._state.transition_flag = 1) : (fsm._state.transition_flag = 0);

	/* Set methods pointers */
	fsm.GetCurrentState = FSM_GetCurrentState;
	fsm.GetStateTransitionFlag = FSM_GetStateTransitionFlag;
	fsm.IsTerminalConditionFulfilled = FSM_IsTerminalConditionFulfilled;
	fsm.SetMinStateDuration = FSM_SetMinStateDuration;
	fsm.SwitchToState = FSM_SwitchToState;

	return (fsm);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t FSM_SwitchToState(SOOL_FSM *fsm, uint8_t state_id) {

	/* Process only if state changed */
	if ( fsm->_state.current != state_id ) {

		/* Check if minimum duration condition is fulfilled */
		if ( FSM_IsTimingTerminalConditionFulfilled(fsm) ) {

			fsm->_state.start_time = SOOL_Periph_TIM_SysTick_GetMillis();
			fsm->_state.last = fsm->_state.current;
			fsm->_state.current = state_id;
			fsm->_state.transition_flag = 1;

			// indicate success
			return (1);

		}

	}

	// indicate fail
	return (0);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void FSM_SetMinStateDuration(SOOL_FSM *fsm, uint32_t ms) {
	fsm->_state.MIN_DURATION = ms;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t FSM_GetCurrentState(SOOL_FSM *fsm) {
	return (fsm->_state.current);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/**
 * @param fsm
 * @return If transition flag is SET then it will be available only once, after each check it is cleared
 */
static uint8_t FSM_GetStateTransitionFlag(SOOL_FSM *fsm) {
	uint8_t temp = fsm->_state.transition_flag;
	fsm->_state.transition_flag = 0;
	return (temp);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t FSM_IsTimingTerminalConditionFulfilled(SOOL_FSM *fsm) {

	/* If MIN_DURATION is 0 - return 1 immediately */
	if ( fsm->_state.MIN_DURATION == 0 ) {
		return (1);
	}

	/* If enough amount of time has passed - return 1 */
	if ( (SOOL_Periph_TIM_SysTick_GetMillis() - fsm->_state.start_time) > fsm->_state.MIN_DURATION ) {
		return (1);
	}

	return (0);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/**
 * Checks whether state's terminal condition is fulfilled
 * @param fsm
 * @param predicate - logically-and`ed set of flags (like: cond1 & cond2 & cond3)
 * @return 1 when terminal predicate is fulfilled
 */
static uint8_t FSM_IsTerminalConditionFulfilled(SOOL_FSM *fsm, uint8_t predicate) {
	if ( predicate ) { return (1); } else { return (0); }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
