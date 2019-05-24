/*
 * FiniteStateMachine.h
 *
 *  Created on: 21.12.2018
 *      Author: user
 */

#ifndef FINITESTATEMACHINE_H_
#define FINITESTATEMACHINE_H_

#include <sool/Peripherals/TIM/SystickTimer.h>
#include <stdint.h>

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

struct _SOOL_FSM_StateStruct {
	uint8_t  current;
	uint8_t  last;
	uint32_t min_duration;
	uint8_t  transition_flag;
	uint32_t start_time;
};

typedef struct _SOOL_FSMStruct SOOL_FSM;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/*
 * Below meant to be some kind of a vector (or C++'s map) storing state id along with state's transition function

//
//struct _SOOL_FSM_StateInfoStruct {
//	uint8_t	state_id;
//	uint8_t (*TransitionFunction)(SOOL_FSM *fsm); // (NOTE: before there were another arguments: `uint8_t arg_num, ...` - deprecated as they ..)
//};
//void (*AddState)(SOOL_FSM *fsm, const uint8_t state_id, uint8_t (*TransitionFunction)(SOOL_FSM *fsm, uint8_t arg_num, ...));
//uint8_t (*ExecuteTransitionFunction)(SOOL_FSM *fsm);
 */

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/**
 * Finite State Machine `alike` class which manages states switching in an application;
 * uses a SysTick timer configured by default (1000 Hz)
 */
struct _SOOL_FSMStruct {

	struct _SOOL_FSM_StateStruct	_state;

	uint8_t (*SwitchToState)(SOOL_FSM *fsm, uint8_t state_id);
	void 	(*SetMinStateDuration)(SOOL_FSM *fsm, uint32_t ms);		/* useful when states are switched via push-buttons */
	uint8_t (*GetCurrentState)(SOOL_FSM *fsm);						/* to be used in main's while(1) to choose proper handler */
	uint32_t (*GetStateDuration)(SOOL_FSM *fsm);
	uint8_t (*GetStateTransitionFlag)(SOOL_FSM *fsm); 				/* useful for handling events after state switch */
	uint8_t (*IsTerminalConditionFulfilled)(SOOL_FSM *fsm, uint8_t predicate);

};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/**
 * Creates a simple FSM instance which uses a SysTick timer configured by default (1000 Hz)
 * @param init_state_id - ID of an initial state
 * @param execute_on_entry - 0 prevents the initial state's on-entry action execution
 * @param min_duration_ms - minimum duration of a state (in milliseconds)
 * @return FSM object
 */
extern SOOL_FSM SOOL_Workflow_FSM_Init(uint8_t init_state_id, uint8_t execute_on_entry, uint32_t min_duration_ms);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#endif /* FINITESTATEMACHINE_H_ */
