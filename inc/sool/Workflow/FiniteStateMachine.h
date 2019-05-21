/*
 * FiniteStateMachine.h
 *
 *  Created on: 21.12.2018
 *      Author: user
 */

#ifndef FINITESTATEMACHINE_H_
#define FINITESTATEMACHINE_H_

#include <sool/Peripherals/TIM/SystickTimer.h> // DEPRECATED?
#include <stdint.h>


//extern uint8_t SOOL_FSM_AddState(const uint8_t state_id, uint8_t (*TransitionFunction)(...));

typedef enum {

	IDLE_BLINKING = 0,
	ARM_BULLET_ADJUSTMENTS,
	ACCELERATING,
	BULLET_ROLLING,
	BULLET_STOPPED,

} FSMStates;

extern void 		SFM_SwitchToState(FSMStates);
extern void			SFM_SetMinStateDuration(uint32_t tithings_of_sec);
extern uint8_t 		SFM_GetStateTransitionFlag();
extern FSMStates	SFM_GetLastState();
extern FSMStates	SFM_GetCurrentState();
extern uint8_t		SFM_IsTimingTerminalConditionFulfilled();

#endif /* FINITESTATEMACHINE_H_ */