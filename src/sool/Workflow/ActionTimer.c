/*
 * ActionTimer.c
 *
 *  Created on: 21.05.2019
 *      Author: user
 */

#include "sool/Workflow/ActionTimer.h"
#include <sool/Workflow/Time_common.h> // computeTimeDifference()

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void ActionTimer_SetStartTime(SOOL_ActionTimer *action_timer, const uint32_t time);
static void ActionTimer_SetEndTime(SOOL_ActionTimer *action_timer, const uint32_t time);
static uint8_t ActionTimer_IsActive(SOOL_ActionTimer *action_timer);
static uint32_t ActionTimer_GetTimeDiff(SOOL_ActionTimer *action_timer);

// helper method
static void ActionTimer_ComputeTimeDifference(SOOL_ActionTimer *action_timer);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

SOOL_ActionTimer SOOL_Workflow_ActionTimer_Init() {

	/* Object to be returned */
	SOOL_ActionTimer action_timer;

	/* Set public methods */
	action_timer.GetTimeDiff = ActionTimer_GetTimeDiff;
	action_timer.IsActive = ActionTimer_IsActive;
	action_timer.SetEndTime = ActionTimer_SetEndTime;
	action_timer.SetStartTime = ActionTimer_SetStartTime;

	/* Initialize private fields */
	action_timer._timer.active = 0;
	action_timer._timer.finished = 1;
	action_timer._timer.time_diff = 0;
	action_timer._timer.time_end = 0;
	action_timer._timer.time_start = 0;

	return (action_timer);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void ActionTimer_SetStartTime(SOOL_ActionTimer *action_timer, const uint32_t time) {

	action_timer->_timer.time_start = time;
	action_timer->_timer.time_diff = 0;
	action_timer->_timer.time_end = 0;
	action_timer->_timer.active = 1;
	action_timer->_timer.finished = 0;

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void ActionTimer_SetEndTime(SOOL_ActionTimer *action_timer, const uint32_t time) {

	action_timer->_timer.time_end = time;
	action_timer->_timer.active = 0;
	action_timer->_timer.finished = 1;

	ActionTimer_ComputeTimeDifference(action_timer);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t ActionTimer_IsActive(SOOL_ActionTimer *action_timer) {
	return (action_timer->_timer.active);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint32_t ActionTimer_GetTimeDiff(SOOL_ActionTimer *action_timer) {

	if ( !action_timer->_timer.finished ) {
		/* Wait for action to finish */
		return (0);
	}
	return (action_timer->_timer.time_diff);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void ActionTimer_ComputeTimeDifference(SOOL_ActionTimer *action_timer) {

//	/* Check if a timer's overflow has occurred */
//	if ( action_timer->_timer.time_end < action_timer->_timer.time_start ) {
//
//		/* Overflow has occurred */
//		//action_timer->_timer.time_diff = (UINT32_MAX - action_timer->_timer.time_start) + action_timer->_timer.time_end;
//		action_timer->_timer.time_diff = (4294967295U - action_timer->_timer.time_start) + action_timer->_timer.time_end;
//
//	} else {
//
//		/* No overflow */
//		action_timer->_timer.time_diff = action_timer->_timer.time_end - action_timer->_timer.time_start;
//
//	}

	action_timer->_timer.time_diff = SOOL_Workflow_Common_ComputeTimeDifference(action_timer->_timer.time_start, action_timer->_timer.time_end);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
