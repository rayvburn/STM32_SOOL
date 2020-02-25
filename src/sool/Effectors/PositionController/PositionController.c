/*
 * PositionController.c
 *
 *  Created on: 07.02.2020
 *      Author: user
 */

#include <sool/Effectors/PositionController/PositionController.h>

static uint8_t 	PositionController_ConfigMove(SOOL_PositionController* controller_ptr,
		uint32_t current_pos, uint32_t goal_pos,
		uint16_t pwm_start, uint16_t pwm_stable, uint16_t pwm_goal,
		uint32_t soft_start_end_pulse, uint32_t soft_stop_start_pulse, uint8_t upcounting);
static void 	PositionController_Abort(SOOL_PositionController* controller_ptr, uint8_t state);
static uint8_t 	PositionController_Process(SOOL_PositionController* controller_ptr, uint32_t current_pos);
static uint8_t 	PositionController_GetMotionStatus(SOOL_PositionController* controller_ptr);
static uint16_t PositionController_GetOutput(SOOL_PositionController* controller_ptr);

// helpers
static void PositionController_SelectNextState(SOOL_PositionController* controller_ptr);

static uint8_t PositionController_HandleAcceleration(SOOL_PositionController* controller_ptr, uint32_t current_pos);
static uint8_t PositionController_HandleStableSpeed(SOOL_PositionController* controller_ptr, uint32_t current_pos);
static uint8_t PositionController_HandleDeceleration(SOOL_PositionController* controller_ptr, uint32_t current_pos);
static uint8_t PositionController_HandleFinished(SOOL_PositionController* controller_ptr, uint32_t current_pos);

static uint8_t PositionController_RearrangeStages(uint32_t current_pos, uint32_t goal_pos,
				uint32_t* soft_start_end_pulse_ptr, uint32_t* soft_stop_start_pulse_ptr,
				uint32_t stage1, uint32_t stage2, uint32_t stage_total, uint8_t upcounting);

static uint8_t PositionController_ShrinkStages(uint32_t current_pos, uint32_t goal_pos,
				uint32_t* soft_start_end_pulse_ptr, uint32_t* soft_stop_start_pulse_ptr,
				uint32_t stage1, uint32_t stage2, uint32_t stage_total, uint8_t upcounting,
				uint16_t pwm_start, uint16_t* pwm_stable_ptr, uint16_t pwm_end);

static uint8_t PositionController_HardCodeShortStages(uint32_t stage_total, uint32_t* soft_start_end_pulse_ptr, uint32_t* soft_stop_start_pulse_ptr,
		uint16_t* pwm_stable_ptr, uint32_t current_pos, uint32_t goal_pos, uint16_t pwm_start, uint8_t upcounting);
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
	controller._config.soft_stop_start_pulse = 0;

	controller.base = SOOL_Effector_SoftStarterCustom_Initialize(20, 90, 5000); // arbitrary values
	controller._state.aborted = 0;
	controller._state.stage_active = SOOL_POSITION_CONTROLLER_FINISHED;

	return (controller);

}

// --------------------------------------------------------------

static uint8_t PositionController_ConfigMove(SOOL_PositionController* controller_ptr,
		uint32_t current_pos, uint32_t goal_pos,
		uint16_t pwm_start, uint16_t pwm_stable, uint16_t pwm_goal,
		uint32_t soft_start_end_pulse, uint32_t soft_stop_start_pulse, uint8_t upcounting)
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
	uint32_t stage1 = SOOL_Sensor_Encoder_PositionCalculator_ComputeDiff(current_pos, soft_start_end_pulse, upcounting);
	uint32_t stage2 = SOOL_Sensor_Encoder_PositionCalculator_ComputeDiff(soft_stop_start_pulse, goal_pos, upcounting);
	uint32_t stage_total = SOOL_Sensor_Encoder_PositionCalculator_ComputeDiff(current_pos, goal_pos, upcounting);

	// check validity
	if ( (stage1 + stage2) > stage_total ) {

		// try to rearrange, save status
		uint8_t status = 0;
		status = PositionController_RearrangeStages(current_pos, goal_pos, &soft_start_end_pulse_mod, &soft_stop_start_pulse_mod, stage1, stage2, stage_total, upcounting);

		// investigate the status
		switch ( status ) {

			case(0): // completely infeasible motion - abort
				return (0);
				break;

			case(1): // try to shrink (max speed will be decreased)
				if ( !PositionController_ShrinkStages(current_pos, goal_pos, &soft_start_end_pulse_mod,
					  &soft_stop_start_pulse_mod, stage1, stage2, stage_total, upcounting,
					  pwm_start, &pwm_stable, pwm_goal) )
				{
					return (0);
				}
				break;

			case(2): // rearranged successfully - process further
				break;

			default:
				break;

		}

	}

	// update internal configuration structure
	controller_ptr->_config.goal_pos = goal_pos;
	controller_ptr->_config.pwm_goal = pwm_goal;
	controller_ptr->_config.pwm_stable = pwm_stable;
	controller_ptr->_config.pwm_start  = pwm_start;
	controller_ptr->_config.soft_stop_start_pulse  = soft_stop_start_pulse_mod;
	controller_ptr->_config.upcounting = upcounting;

	// just in case of some leftovers
	PositionController_Abort(controller_ptr, 0);

	// arrange soft-start according to the given set of parameters
	if ( !controller_ptr->base.Reconfigure(&controller_ptr->base, controller_ptr->_config.pwm_start,
		  controller_ptr->_config.pwm_stable,
		  SOOL_Sensor_Encoder_PositionCalculator_ComputeDiff(current_pos, soft_start_end_pulse_mod, upcounting)) ) // first stage (acceleration)
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

static uint8_t PositionController_Process(SOOL_PositionController* controller_ptr, uint32_t current_pos) {

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

static uint8_t PositionController_HandleAcceleration(SOOL_PositionController* controller_ptr, uint32_t current_pos) {

	// a totally typical operation here
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

	} else if ( controller_ptr->base.IsFinished(&controller_ptr->base) ) {

		// sometimes a 0-length ramp will be requested and `Process`
		// will never return TRUE during that stage execution

		// abort anyway
		if ( controller_ptr->_state.aborted ) {
			return (1);
		}

		PositionController_SelectNextState(controller_ptr); // go to STABLE
		// go further with the same speed

	}
	return (0);

}

// --------------------------------------------------------------

static uint8_t PositionController_HandleStableSpeed(SOOL_PositionController* controller_ptr, uint32_t current_pos) {

	// check whether the internal FSM operation of the PositionController should be continued;
	// `aborted` status still allows to change speed via Process calls but abandons
	// acceleration-stable speed-deceleration-finished pattern
	if ( controller_ptr->_state.aborted ) {
		return (controller_ptr->base.Process(&controller_ptr->base, current_pos));
	}

	// TODO: safety in case of some `lag`? - consider `_config.upcounting` and `inequality`
	// evaluate whether to start decelerating
	if ( current_pos == controller_ptr->_config.soft_stop_start_pulse ) {

		// calculate how many `ticks` should deceleration take
		// difference between current position and the goal position
		int64_t duration = SOOL_Sensor_Encoder_PositionCalculator_ComputeDiff(current_pos, controller_ptr->_config.goal_pos, controller_ptr->_config.upcounting);

		// desired action
		if ( !controller_ptr->base.Reconfigure(&controller_ptr->base,
			  controller_ptr->_config.pwm_stable, controller_ptr->_config.pwm_goal, duration) ) {

			// FIXME: error handling, although it is very unlikely that the motion verified
			// at the configuration stage will not be feasible at that point
			//
			// safe slow-down on ERROR, assuming desired duration of the motion
			PositionController_Abort(controller_ptr, 1);
			controller_ptr->base.Reconfigure(&controller_ptr->base, controller_ptr->base.Get(&controller_ptr->base), controller_ptr->_config.pwm_goal, duration);
			// never got it in the debugger, but at least the motor will (almost?) stop safely

		}

		PositionController_SelectNextState(controller_ptr); // go to DECELERATION

	}

	return (0); // `pulse` value will never be changed

}

// --------------------------------------------------------------

static uint8_t PositionController_HandleDeceleration(SOOL_PositionController* controller_ptr, uint32_t current_pos) {

	// FIXME: this looks the same as HandleAcceleration
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

	} else if ( controller_ptr->base.IsFinished(&controller_ptr->base) ) {

		// sometimes a 0-length ramp will be requested and `Process`
		// will never return TRUE during that stage execution

		// abort anyway
		if ( controller_ptr->_state.aborted ) {
			return (1);
		}

		PositionController_SelectNextState(controller_ptr); // go to STABLE
		// go further with the same speed

	}
	return (0);

}

// --------------------------------------------------------------

static uint8_t PositionController_HandleFinished(SOOL_PositionController* controller_ptr, uint32_t current_pos) {

	// check whether the internal FSM operation of the PositionController should be continued;
	// `aborted` status still allows to change speed via Process calls but abandons
	// acceleration-stable speed-deceleration-finished pattern
	if ( controller_ptr->_state.aborted ) {
		return (controller_ptr->base.Process(&controller_ptr->base, current_pos));
	}

	return (0);

}

// --------------------------------------------------------------

static uint8_t PositionController_RearrangeStages(uint32_t current_pos, uint32_t goal_pos,
				uint32_t* soft_start_end_pulse_ptr, uint32_t* soft_stop_start_pulse_ptr,
				uint32_t stage1, uint32_t stage2, uint32_t stage_total, uint8_t upcounting)
{

	// if a sum of ranges:
	// 	1) from `start` to `soft_start_end` (stage1)
	//	2) from `soft_stop_start` to `goal` (stage2)
	// is bigger than a range from `start` to `goal` (stage_total)
	// let's shift the `soft_start_end` and `soft_stop_start` equally
	// so the `steady_speed` state will be omitted but the movement
	// will be feasible;
	// do not allow shifts which would increase the ramp both increment and decrement `lengths` triple

	uint16_t soft_start_end = (uint16_t)*soft_start_end_pulse_ptr;
	uint16_t soft_stop_beg = (uint16_t)*soft_stop_start_pulse_ptr;
	int64_t diff = (stage1 + stage2) - stage_total;
	// must be positive (`diff` counted-in twice here)
	if ( diff <= 0 ) {
		return (0);
	}

	// check whether the shrinkage is not too aggressive
	if ( ((stage1 / 2) <= diff) || ((stage2 / 2) <= diff) ) {
		return (1);
	}

	// counting direction
	if ( upcounting ) {
		// temporary
		soft_start_end = SOOL_Sensor_Encoder_PositionCalculator_ComputeGoal(soft_start_end, (int32_t)(-diff));
		soft_stop_beg = SOOL_Sensor_Encoder_PositionCalculator_ComputeGoal(soft_stop_beg, (int32_t)(+diff));
	} else {
		// temporary
		soft_start_end = SOOL_Sensor_Encoder_PositionCalculator_ComputeGoal(soft_start_end, (int32_t)(+diff));
		soft_stop_beg = SOOL_Sensor_Encoder_PositionCalculator_ComputeGoal(soft_stop_beg, (int32_t)(-diff));
	}

	// evaluate ranges once again now
	uint32_t stage1_mod = SOOL_Sensor_Encoder_PositionCalculator_ComputeDiff(current_pos, soft_start_end, upcounting);
	uint32_t stage2_mod = SOOL_Sensor_Encoder_PositionCalculator_ComputeDiff(soft_stop_beg, goal_pos, upcounting);

	// check if ranges got shrunk enough
	if ( stage1_mod + stage2_mod <= stage_total ) {
		// found
		*soft_start_end_pulse_ptr = soft_start_end;
		*soft_stop_start_pulse_ptr = soft_stop_beg;
		return (2);
	}

	// no luck
	return (0);

}

// --------------------------------------------------------------

/// @brief Will cut the `pwm_stable` speed as much as is needed to shrink the stages 1 and 3,
/// to fit all 3 stages into the range between `current_pos` and `goal_pos`
static uint8_t PositionController_ShrinkStages(uint32_t current_pos, uint32_t goal_pos,
				uint32_t* soft_start_end_pulse_ptr, uint32_t* soft_stop_start_pulse_ptr,
				uint32_t stage1, uint32_t stage2, uint32_t stage_total, uint8_t upcounting,
				uint16_t pwm_start, uint16_t* pwm_stable_ptr, uint16_t pwm_end) {

	struct _SOOL_SoftStarterConfigStruct config_start, config_stop;
	struct _SOOL_SoftStarterSetupStruct setup_start, setup_stop;
	struct _SOOL_SoftStarterStateStruct state_start, state_stop;

	// evaluate feasibility of separate stages
	if ( !SOOL_Effector_SoftStarter_Reconfigure(&config_start, &setup_start, &state_start, pwm_start, *pwm_stable_ptr, stage1) ) {
		return (0);
	}
	if ( !SOOL_Effector_SoftStarter_Reconfigure(&config_stop, &setup_stop, &state_stop, *pwm_stable_ptr, pwm_end, stage2) ) {
		return (0);
	}

	// calculate maximum speed for shortened stages
	int32_t pwm_stage1 = *pwm_stable_ptr;
	int32_t pwm_stage2 = *pwm_stable_ptr;

	// calculate new duration of shortened stages
	int64_t duration_stage1 = stage1;
	int64_t duration_stage2 = stage2;

	// modified value of the PWM pulse for the `stable` stage
	uint16_t pwm_stable_mod = *pwm_stable_ptr;

	// movement has to be very short - let's hard-code that case, some stages may be abandoned
	if ( stage_total <= 3 ) {
		return (PositionController_HardCodeShortStages(stage_total, soft_start_end_pulse_ptr, soft_stop_start_pulse_ptr, pwm_stable_ptr, current_pos, goal_pos, pwm_start, upcounting));
	}

	// `stage_total` is bigger than 3
	// change until the `intersection point` is found;
	// this while loop works for cases where (`stage_total >= 2`), see the related `small_range` flag
	while ( (duration_stage1 + duration_stage2) > (stage_total) ) { /* previously: (stage_total - 2) ) { */ // `length` condition; `-2` is a margin

		// NOTE: the trapezoidal pattern of motion ( rotational speed(pulses) function )
		// allows to assume that the `setup_start.pulse_change` is positive while
		// `setup_stop.pulse_change` is negative
		pwm_stage1 -= setup_start.pulse_change;
		pwm_stage2 += setup_stop.pulse_change;

		duration_stage1 -= setup_start.time_change_gap;
		duration_stage2 -= setup_stop.time_change_gap;

		// check whether it is safe to apply some margin
		// TODO: case - one stage of some small length and the other with 0?;
		// HERE: both stages are similarly lengthened
		if ( duration_stage1 < 1 && duration_stage2 < 1 ) {
			return (0);
			break;
		}

	}

	// set `pwm_stable` as the bigger `pwm_stageX`
	if ( pwm_stage1 < pwm_stage2 ) {
		pwm_stable_mod = pwm_stage2;
	} else {
		pwm_stable_mod = pwm_stage1;
	}

	// evaluate length of modified stages in regards to total motion `length`
	if ( (duration_stage1 + duration_stage2) > stage_total ) {
		return (0);
	}

	// update the pointers with modified values;
	// consider counting direction
	if ( upcounting ) {
		*soft_start_end_pulse_ptr  = SOOL_Sensor_Encoder_PositionCalculator_ComputeGoal(current_pos, +duration_stage1);
		*soft_stop_start_pulse_ptr = SOOL_Sensor_Encoder_PositionCalculator_ComputeGoal(goal_pos, 	 -duration_stage2);
	} else {
		*soft_start_end_pulse_ptr  = SOOL_Sensor_Encoder_PositionCalculator_ComputeGoal(current_pos, -duration_stage1);
		*soft_stop_start_pulse_ptr = SOOL_Sensor_Encoder_PositionCalculator_ComputeGoal(goal_pos, 	 +duration_stage2);
	}

	// `pwm stable` - update
	*pwm_stable_ptr = pwm_stable_mod;

	return (1);

}

// --------------------------------------------------------------

/// brief Forces motion execution with hard-coded parameters. Not the best solution
/// but at least the goal position should be reached successfully.
static uint8_t PositionController_HardCodeShortStages(uint32_t stage_total, uint32_t* soft_start_end_pulse_ptr, uint32_t* soft_stop_start_pulse_ptr,
		uint16_t* pwm_stable_ptr, uint32_t current_pos, uint32_t goal_pos, uint16_t pwm_start, uint8_t upcounting) {

	// NOTE: see the `margin` in the `while` loop above
	if ( stage_total == 1 ) {

		*pwm_stable_ptr = pwm_start;
		*soft_start_end_pulse_ptr = current_pos;
		*soft_stop_start_pulse_ptr = goal_pos;
		return (1);

	} else if ( stage_total == 2 ) {

		*pwm_stable_ptr = pwm_start;
		*soft_start_end_pulse_ptr = current_pos;
		if ( upcounting ) {
			*soft_stop_start_pulse_ptr = SOOL_Sensor_Encoder_PositionCalculator_ComputeGoal(goal_pos, -1);
			return (1);
		} else {
			*soft_stop_start_pulse_ptr = SOOL_Sensor_Encoder_PositionCalculator_ComputeGoal(goal_pos, +1);
			return (1);
		}

	} else if ( stage_total == 3 ) {

		*pwm_stable_ptr = pwm_start;
		if ( upcounting ) {
			*soft_start_end_pulse_ptr  = SOOL_Sensor_Encoder_PositionCalculator_ComputeGoal(current_pos, +1);
			*soft_stop_start_pulse_ptr = SOOL_Sensor_Encoder_PositionCalculator_ComputeGoal(goal_pos, -1);
			return (1);
		} else {
			*soft_start_end_pulse_ptr  = SOOL_Sensor_Encoder_PositionCalculator_ComputeGoal(current_pos, -1);
			*soft_stop_start_pulse_ptr = SOOL_Sensor_Encoder_PositionCalculator_ComputeGoal(goal_pos, +1);
			return (1);
		}

	} else {

		// unsupported case
		return (0);

	}

}
