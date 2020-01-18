/*
 * ActionTimerResumable.c
 *
 *  Created on: 15.10.2019
 *      Author: user
 */

#include "sool/Workflow/ActionTimerResumable.h"
#include <sool/Workflow/Time_common.h> // computeTimeDifference()

static void 		ActionTimerResumable_AddStartTime(SOOL_ActionTimerResumable *timer_ptr, const uint32_t timestamp);
static void 		ActionTimerResumable_AddEndTime(SOOL_ActionTimerResumable *timer_ptr, const uint32_t timestamp);
static uint8_t		ActionTimerResumable_IsActive(SOOL_ActionTimerResumable *timer_ptr);
static uint32_t 	ActionTimerResumable_CalculateTimeDiff(SOOL_ActionTimerResumable *timer_ptr, const uint32_t timestamp);
static uint32_t 	ActionTimerResumable_CalculateTimeDiffTotalFly(SOOL_ActionTimerResumable *timer_ptr, const uint32_t timestamp);
static uint32_t 	ActionTimerResumable_CalculateTimeDiffTotal(SOOL_ActionTimerResumable *timer_ptr);
static void 		ActionTimerResumable_Reset(SOOL_ActionTimerResumable *timer_ptr);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

SOOL_ActionTimerResumable SOOL_Workflow_ActionTimerResumable_Init() {

	SOOL_ActionTimerResumable timer;

	// initialize vectors
	timer._timer.start_v = SOOL_Memory_Vector_Uint32_Init();
	timer._timer.end_v = SOOL_Memory_Vector_Uint32_Init();

	// internal state
	timer._timer.active = 0;

	// update member functions pointers
	timer.AddEndTime = ActionTimerResumable_AddEndTime;
	timer.AddStartTime = ActionTimerResumable_AddStartTime;
	timer.CalculateTimeDiff = ActionTimerResumable_CalculateTimeDiff;
	timer.CalculateTimeDiffTotalFly = ActionTimerResumable_CalculateTimeDiffTotalFly;
	timer.CalculateTimeDiffTotal = ActionTimerResumable_CalculateTimeDiffTotal;
	timer.IsActive = ActionTimerResumable_IsActive;
	timer.Reset = ActionTimerResumable_Reset;

	return (timer);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void ActionTimerResumable_AddStartTime(SOOL_ActionTimerResumable *timer_ptr, const uint32_t timestamp) {
	timer_ptr->_timer.active = 1;
	timer_ptr->_timer.start_v.Add(&timer_ptr->_timer.start_v, timestamp);
}
static void ActionTimerResumable_AddEndTime(SOOL_ActionTimerResumable *timer_ptr, const uint32_t timestamp) {
	timer_ptr->_timer.active = 0;
	timer_ptr->_timer.end_v.Add(&timer_ptr->_timer.end_v, timestamp);
}
static uint8_t ActionTimerResumable_IsActive(SOOL_ActionTimerResumable *timer_ptr) {
	return (timer_ptr->_timer.active);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint32_t ActionTimerResumable_CalculateTimeDiff(SOOL_ActionTimerResumable *timer_ptr, const uint32_t timestamp) {

	if ( !ActionTimerResumable_IsActive(timer_ptr) ) {
		return (0);
	}

	uint32_t start = timer_ptr->_timer.start_v.Get(&timer_ptr->_timer.start_v, timer_ptr->_timer.start_v._info.size - 1);
	return (SOOL_Workflow_Common_ComputeTimeDifference(start, timestamp));

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint32_t ActionTimerResumable_CalculateTimeDiffTotalFly(SOOL_ActionTimerResumable *timer_ptr, const uint32_t timestamp) {

	if ( !ActionTimerResumable_IsActive(timer_ptr) ) {
		return (0);
	}

	// sum up all components (vector elements)
	uint32_t time_elapsed = 0;

	// iterate over all but last `pairs`
	for ( uint16_t i = 0; i < (timer_ptr->_timer.start_v._info.size - 1); i++ ) {
		time_elapsed += SOOL_Workflow_Common_ComputeTimeDifference(timer_ptr->_timer.start_v.Get(&timer_ptr->_timer.start_v, i),
																   timer_ptr->_timer.end_v.Get(&timer_ptr->_timer.end_v, i));
	}

	uint32_t start = timer_ptr->_timer.start_v.Get(&timer_ptr->_timer.start_v, timer_ptr->_timer.start_v._info.size - 1);
	time_elapsed += SOOL_Workflow_Common_ComputeTimeDifference(start, timestamp);

	return (time_elapsed);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint32_t ActionTimerResumable_CalculateTimeDiffTotal(SOOL_ActionTimerResumable *timer_ptr) {

	// helper variable storing sum of time differences
	uint32_t time_elapsed = 0;

	if ( timer_ptr->_timer.start_v._info.size != timer_ptr->_timer.end_v._info.size ) {
		return (0); // error
	}

	// sum up all components (vector elements)
	for ( uint16_t i = 0; i < timer_ptr->_timer.start_v._info.size; i++ ) {
		time_elapsed += SOOL_Workflow_Common_ComputeTimeDifference(timer_ptr->_timer.start_v.Get(&timer_ptr->_timer.start_v, i),
																   timer_ptr->_timer.end_v.Get(&timer_ptr->_timer.end_v, i));
	}

	return (time_elapsed);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void ActionTimerResumable_Reset(SOOL_ActionTimerResumable *timer_ptr) {

	timer_ptr->_timer.active = 0;

	// keep removing 0-index elements until there is an empty vector
	while (timer_ptr->_timer.start_v._info.size) {
		timer_ptr->_timer.start_v.Remove(&timer_ptr->_timer.start_v, 0);
	}

	while (timer_ptr->_timer.end_v._info.size) {
		timer_ptr->_timer.end_v.Remove(&timer_ptr->_timer.end_v, 0);
	}

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
