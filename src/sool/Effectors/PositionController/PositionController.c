/*
 * PositionController.c
 *
 *  Created on: 07.02.2020
 *      Author: user
 */

#include <sool/Effectors/PositionController/PositionController.h>

static uint8_t 	PositionController_ConfigMove(SOOL_PositionController* controller_ptr, int64_t current_pos, int64_t goal_pos,
											  uint16_t pwm_start, uint16_t pwm_stable, uint16_t pwm_goal,
											  uint32_t soft_start_end_pulse, uint32_t soft_stop_start_pulse,
											  uint8_t upcounting);
static void 	PositionController_Abort(SOOL_PositionController* controller_ptr, uint8_t state);
static uint8_t 	PositionController_Process(SOOL_PositionController* controller_ptr, int64_t current_pos);
static uint8_t 	PositionController_GetMotionStatus(SOOL_PositionController* controller_ptr);
static uint16_t PositionController_GetOutput(SOOL_PositionController* controller_ptr);

// helpers
static void PositionController_SelectNextState(SOOL_PositionController* controller_ptr);

static uint8_t PositionController_HandleAcceleration(SOOL_PositionController* controller_ptr, int64_t current_pos);
static uint8_t PositionController_HandleStableSpeed(SOOL_PositionController* controller_ptr, int64_t current_pos);
static uint8_t PositionController_HandleDeceleration(SOOL_PositionController* controller_ptr, int64_t current_pos);
static uint8_t PositionController_HandleFinished(SOOL_PositionController* controller_ptr, int64_t current_pos);

static void PositionController_ShrinkRanges(int64_t current_pos, int64_t goal_pos,
											uint32_t* soft_start_end_pulse_ptr, uint32_t* soft_stop_start_pulse_ptr);

// --------------------------------------------------------------

SOOL_PositionController SOOL_Effector_PositionController_Initialize() {

	SOOL_PositionController controller;

	controller.ConfigMove = PositionController_ConfigMove;
	controller.GetMotionStatus = PositionController_GetMotionStatus;
	controller.Process = PositionController_Process;
	controller.GetOutput = PositionController_GetOutput;
	controller.Abort = PositionController_Abort;

	controller._config.goal_pos = 0;
	controller._config.pwm_goal = 0;
	controller._config.pwm_stable = 0;
	controller._config.pwm_start = 0;
//	controller._config.soft_start_end_pulse = 0;
	controller._config.soft_stop_start_pulse = 0;

	controller.base = SOOL_Effector_SoftStarterCustom_Initialize(20, 90, 5000);
	controller._state.aborted = 0;
	controller._state.stage_active = SOOL_POSITION_CONTROLLER_FINISHED;

	return (controller);

}

// --------------------------------------------------------------


static uint8_t PositionController_ConfigMove(SOOL_PositionController* controller_ptr, int64_t current_pos, int64_t goal_pos,
			uint16_t pwm_start, uint16_t pwm_stable, uint16_t pwm_goal, uint32_t soft_start_end_pulse, uint32_t soft_stop_start_pulse,
			uint8_t upcounting)
{

	// already at the goal position!
	if ( current_pos == goal_pos ) {
		return (0);
	}

	// useful for potential shrinking (DEPRECATED?)
	uint32_t soft_start_end_pulse_mod = soft_start_end_pulse;
	uint32_t soft_stop_start_pulse_mod = soft_stop_start_pulse;

	// UP-COUNTING or DOWN-COUNTING - 2 possibilities available
	// evaluate motion feasibility;
	// shrink the soft-start and soft-stop ranges if necessary (DEPRECATED);
	// APPLIES BOTH FOR no `overflow` and `overflow` cases;
	// FIXME: separate variables for easier debugging
	uint32_t stage1 = SOOL_Sensor_Encoder_PositionCalculator_ComputeDiff(current_pos, soft_start_end_pulse, upcounting);
	uint32_t stage2 = SOOL_Sensor_Encoder_PositionCalculator_ComputeDiff(soft_stop_start_pulse, goal_pos, upcounting);
	uint32_t stage_total = SOOL_Sensor_Encoder_PositionCalculator_ComputeDiff(current_pos, goal_pos, upcounting);
	if ( (stage1 + stage2) > stage_total )
//	if ( (SOOL_Sensor_Encoder_PositionCalculator_ComputeDiff(current_pos, soft_start_end_pulse, upcounting) +
//		  SOOL_Sensor_Encoder_PositionCalculator_ComputeDiff(soft_stop_start_pulse, goal_pos, upcounting))  >
//		 SOOL_Sensor_Encoder_PositionCalculator_ComputeDiff(current_pos, goal_pos, upcounting) )
	{
		// shrink?
		// PositionController_ShrinkRanges(current_pos, goal_pos,
		//								   &soft_start_end_pulse_mod, &soft_stop_start_pulse_mod);
		return (0);
	}

	// update internal configuration structure
	controller_ptr->_config.goal_pos = goal_pos;
	controller_ptr->_config.pwm_goal = pwm_goal;
	controller_ptr->_config.pwm_stable = pwm_stable;
	controller_ptr->_config.pwm_start  = pwm_start;
	controller_ptr->_config.soft_stop_start_pulse  = soft_stop_start_pulse_mod;
	controller_ptr->_config.upcounting = upcounting;

	// arrange soft-start according to the given set of parameters
	if ( !controller_ptr->base.Reconfigure(&controller_ptr->base, controller_ptr->_config.pwm_start,
		  controller_ptr->_config.pwm_stable,
		  SOOL_Sensor_Encoder_PositionCalculator_ComputeDiff(current_pos, soft_start_end_pulse, upcounting)) ) // first stage (acceleration)
	{
		// check the error cause
		// 1st reason: equal PWM values - no SoftStarter operation needed (at least in the `acceleration` stage
		if ( pwm_stable == pwm_start ) {

			// NOTE: `pwm_start` will be the output during the `acceleration` stage
			controller_ptr->base.Start(&controller_ptr->base, current_pos);
			// jump straight into the stable speed `sub-state` and wait for the moment,
			// when deceleration should be triggered;
			// starting with ACCELERATION with these initial conditions will fail -
			// Process will never return 1
			controller_ptr->_state.stage_active = (uint8_t)SOOL_POSITION_CONTROLLER_STABLE;
			return (1);
		} else {
			// unhandled case(s)
			return (0);
		}
	}

	// initialize
	controller_ptr->base.Start(&controller_ptr->base, current_pos);
	controller_ptr->_state.stage_active = (uint8_t)SOOL_POSITION_CONTROLLER_ACCELERATION;
	return (1);

}

// --------------------------------------------------------------

static void PositionController_Abort(SOOL_PositionController* controller_ptr, uint8_t state) {

	switch ( state ) {

		case(1):
			controller_ptr->_state.aborted = 1;
			controller_ptr->_state.stage_active = 0; // finished
			break;
		case(0):
			controller_ptr->_state.aborted = 0;
			break;
		default:
			break;

	}

}

// --------------------------------------------------------------

static uint8_t PositionController_Process(SOOL_PositionController* controller_ptr, int64_t current_pos) {

	// save the operation status
	uint8_t status = 2;

	// evaluate the current state of the internal FSM
	switch ( controller_ptr->_state.stage_active ) {

		case(SOOL_POSITION_CONTROLLER_ACCELERATION):
			status = PositionController_HandleAcceleration(controller_ptr, current_pos);
			break;

		case(SOOL_POSITION_CONTROLLER_STABLE):
			status = PositionController_HandleStableSpeed(controller_ptr, current_pos);
			break;

		case(SOOL_POSITION_CONTROLLER_DECELERATION):
			status = PositionController_HandleDeceleration(controller_ptr, current_pos);
			break;

		case(SOOL_POSITION_CONTROLLER_FINISHED):
			// in fact - `idling`
			// lack of `SelectNextState` -> do not allow switch to `acceleration` after reaching the goal and waiting for the new one (` == 0 `))
			status = PositionController_HandleFinished(controller_ptr, current_pos);
			break;

		default:
			break;

	}

	return (status);

}

// --------------------------------------------------------------

static void PositionController_SelectNextState(SOOL_PositionController* controller_ptr) {

	if ( ++controller_ptr->_state.stage_active > 3 ) {
		controller_ptr->_state.stage_active = 0;
	}

}

// --------------------------------------------------------------

static uint8_t PositionController_GetMotionStatus(SOOL_PositionController* controller_ptr) {

	if ( controller_ptr->_state.aborted ) {

		// workaround
		if ( controller_ptr->base.IsFinished(&controller_ptr->base) ) {
			return ((uint16_t)SOOL_POSITION_CONTROLLER_FINISHED);
		} else {
			return ((uint16_t)SOOL_POSITION_CONTROLLER_ACCELERATION); // arbitrarily chosen, prevents the motion from being incorrectly interpreted (as finished)
		}

	}
	return ((uint8_t)controller_ptr->_state.stage_active);

}

// --------------------------------------------------------------

static uint16_t PositionController_GetOutput(SOOL_PositionController* controller_ptr) {
	return (controller_ptr->base.Get(&controller_ptr->base));
}

// --------------------------------------------------------------

static uint8_t PositionController_HandleAcceleration(SOOL_PositionController* controller_ptr, int64_t current_pos) {

	if ( controller_ptr->base.Process(&controller_ptr->base, current_pos) ) {

		// check whether the internal FSM operation of the PositionController should be continued;
		// `aborted` status still allows to change speed via Process calls but abandons
		// acceleration-stable speed-deceleration-finished pattern
		if ( controller_ptr->_state.aborted ) {
			return (1);
		}

		if ( controller_ptr->base.IsFinished(&controller_ptr->base) ) {

			PositionController_SelectNextState(controller_ptr); // go to STABLE
			// go further with the same speed

		}
		return (1);

	}
	return (0);

}

// --------------------------------------------------------------

static uint8_t PositionController_HandleStableSpeed(SOOL_PositionController* controller_ptr, int64_t current_pos) {

	// check whether the internal FSM operation of the PositionController should be continued;
	// `aborted` status still allows to change speed via Process calls but abandons
	// acceleration-stable speed-deceleration-finished pattern
	if ( controller_ptr->_state.aborted ) {
		return (controller_ptr->base.Process(&controller_ptr->base, current_pos));
	}

	// evaluate whether to start decelerating
	if ( current_pos == controller_ptr->_config.soft_stop_start_pulse ) {

		// calculate how many `ticks` should deceleration take
		// difference between current position and the goal position
		int64_t duration = SOOL_Sensor_Encoder_PositionCalculator_ComputeDiff(current_pos, controller_ptr->_config.goal_pos, controller_ptr->_config.upcounting);

		// desired action
		if ( !controller_ptr->base.Reconfigure(&controller_ptr->base,
			  controller_ptr->_config.pwm_stable, controller_ptr->_config.pwm_goal, duration) ) {

			// FIXME: error handling
			// safe slow-down on ERROR, assuming desired duration of the motion
			// controller_ptr->base.Reconfigure(&controller_ptr->base, controller_ptr->base.Get(&controller_ptr->base), controller_ptr->_config.pwm_goal, duration);

		}

		PositionController_SelectNextState(controller_ptr); // go to DECELERATION

	}

	return (0); // `pulse` value will never be changed

}

// --------------------------------------------------------------

static uint8_t PositionController_HandleDeceleration(SOOL_PositionController* controller_ptr, int64_t current_pos) {

	if ( controller_ptr->base.Process(&controller_ptr->base, current_pos) ) {

		// check whether the internal FSM operation of the PositionController should be continued;
		// `aborted` status still allows to change speed via Process calls but abandons
		// acceleration-stable speed-deceleration-finished pattern
		if ( controller_ptr->_state.aborted ) {
			return (1);
		}

		if ( controller_ptr->base.IsFinished(&controller_ptr->base) ) {

			PositionController_SelectNextState(controller_ptr); // FINISHED
			// motion finished

		}
		return (1);

	}
	return (0);

}

// --------------------------------------------------------------

static uint8_t PositionController_HandleFinished(SOOL_PositionController* controller_ptr, int64_t current_pos) {

	// check whether the internal FSM operation of the PositionController should be continued;
	// `aborted` status still allows to change speed via Process calls but abandons
	// acceleration-stable speed-deceleration-finished pattern
	if ( controller_ptr->_state.aborted ) {
		return (controller_ptr->base.Process(&controller_ptr->base, current_pos));
	}

	return (0);

}

// --------------------------------------------------------------

static void PositionController_ShrinkRanges(int64_t current_pos, int64_t goal_pos,
			uint32_t* soft_start_end_pulse_ptr, uint32_t* soft_stop_start_pulse_ptr)
{



}
