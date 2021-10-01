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
static uint8_t  PositionController_ConfigMoveStructAbs(SOOL_PositionController* controller_ptr,
        struct PositionControlAbsRequest* ctrl_req);
static uint8_t  PositionController_ConfigMoveStructRel(SOOL_PositionController* controller_ptr,
        struct PositionControlRelRequest* ctrl_req);
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

static struct _SOOL_SoftStarterSetupStruct PositionController_FindStageSetup(uint32_t stage_length,
		struct _SOOL_SoftStarterSetupStruct setup_opposite, uint32_t stage_opposite_length);
// --------------------------------------------------------------

SOOL_PositionController SOOL_Effector_PositionController_Initialize() {

	SOOL_PositionController controller;

	controller.ConfigMove = PositionController_ConfigMove;
    controller.ConfigMoveStructAbs = PositionController_ConfigMoveStructAbs;
    controller.ConfigMoveStructRel = PositionController_ConfigMoveStructRel;
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
	// printf("[SOOL_Effector_PositionController_Initialize] dummy ctor data\r\n\r\n");
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
		// printf("[PositionController_ConfigMove] equal positions - current = goal\r\n");
		return (0);
	}

	// useful for potential shrinking (DEPRECATED?)
	uint32_t soft_start_end_pulse_mod = soft_start_end_pulse;
	uint32_t soft_stop_start_pulse_mod = soft_stop_start_pulse;
	// printf("[PositionController_ConfigMove] Initial: stage1_soft_start_end_pos %d, stage3_soft_stop_start_pos %d, upcounting %d\r\n",
	// 	(int)soft_start_end_pulse,
	// 	(int)soft_stop_start_pulse,
	// 	(int)upcounting
	// );

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
				// printf("[PositionController_ConfigMove] completely infeasible motion - abort\r\n");
				return (0);
				break;

			case(1): // try to shrink (max speed will be decreased)
				if ( !PositionController_ShrinkStages(current_pos, goal_pos, &soft_start_end_pulse_mod,
					  &soft_stop_start_pulse_mod, stage1, stage2, stage_total, upcounting,
					  pwm_start, &pwm_stable, pwm_goal) )
				{
					// printf("[PositionController_ConfigMove] initial config not valid, shrunk not successful\r\n");
					return (0);
				} else {
					// printf("[PositionController_ConfigMove] shrink after rearrangement - successful: "
                    //     "acc_end_pulse %d dec_start_pulse %d speed_stable %d\r\n",
                    //     soft_start_end_pulse_mod,
                    //     soft_stop_start_pulse_mod,
                    //     pwm_stable
                    // );
				}
				break;

			case(2): // rearranged successfully - process further
				// printf("[PositionController_ConfigMove] rearranged successfully / stage1 %d, stage2 %d, stage_total %d\r\n",
                //     (int)stage1,
                //     (int)stage2,
                //     (int)stage_total
                // );
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

	// printf("[PositionController_ConfigMove] \r\n"
	// 	"\t\tController Config: goal_pos %d, pwm_start %d, pwm_stable %d, pwm_goal %d, soft_stop_start_pulse %d, upcounting %d\r\n",
	// 	(int)controller_ptr->_config.goal_pos,
	// 	(int)controller_ptr->_config.pwm_start,
	// 	(int)controller_ptr->_config.pwm_stable,
	// 	(int)controller_ptr->_config.pwm_goal,
	// 	(int)controller_ptr->_config.soft_stop_start_pulse,
	// 	(int)controller_ptr->_config.upcounting
	// );

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
			// printf("[PositionController_ConfigMove] (branch)\033[33m total path length: %d, stage1 (%d - %d), stage2 (%d - %d), stage3 (%d - %d) \033[0m\r\n",
			// 	(int)(
            //         (soft_start_end_pulse_mod - current_pos) + 
            //         (soft_stop_start_pulse_mod - soft_start_end_pulse_mod) + 
            //         (goal_pos - soft_stop_start_pulse_mod)
            //         ),
			// 	(int)(current_pos),
			// 	(int)(soft_start_end_pulse_mod),
			// 	(int)(soft_start_end_pulse_mod),
			// 	(int)(soft_stop_start_pulse_mod),
			// 	(int)(soft_stop_start_pulse_mod),
			// 	(int)(goal_pos)
			// );
			return (1);
		} else {
			// unhandled case(s)
			// printf("[PositionController_ConfigMove] Fail before finish\r\n");
			return (0);
		}
	}

	// printf("[PositionController_ConfigMove]\033[33m total path length: %d, stage1 (%d - %d), stage2 (%d - %d), stage3 (%d - %d) \033[0m\r\n",
	// 	(int)(goal_pos - current_pos),
	// 	(int)(current_pos),
	// 	(int)(soft_start_end_pulse_mod),
	// 	(int)(soft_start_end_pulse_mod),
	// 	(int)(soft_stop_start_pulse_mod),
	// 	(int)(soft_stop_start_pulse_mod),
	// 	(int)(goal_pos)
	// );

	// initialize
	controller_ptr->base.Start(&controller_ptr->base, current_pos);
	controller_ptr->_state.stage_active = (uint8_t)SOOL_POSITION_CONTROLLER_ACCELERATION;
	return (1);

}

// --------------------------------------------------------------

static uint8_t PositionController_ConfigMoveStructAbs(
    SOOL_PositionController* controller_ptr,
    struct PositionControlAbsRequest* ctrl_req
) {
    return PositionController_ConfigMove(
        controller_ptr,
        ctrl_req->pos_acc_start,
        ctrl_req->pos_goal,
        ctrl_req->speed_start,
        ctrl_req->speed_stable,
        ctrl_req->speed_end,
        ctrl_req->pos_stable_start,
        ctrl_req->pos_dec_start,
        ctrl_req->upcounting
    );
}

// --------------------------------------------------------------

static uint8_t PositionController_ConfigMoveStructRel(
    SOOL_PositionController* controller_ptr,
    struct PositionControlRelRequest* ctrl_req
) {
    // printf("PositionController_ConfigMoveStructRel IN\r\n");
    uint32_t pos_stable_start = SOOL_Sensor_Encoder_PositionCalculator_ComputeGoal(ctrl_req->pos_acc_start, ctrl_req->pos_acc_len);
    uint32_t pos_dec_start = SOOL_Sensor_Encoder_PositionCalculator_ComputeGoal(ctrl_req->pos_goal, ctrl_req->pos_dec_len);
    // printf("PositionController_ConfigMoveStructRel pos_stable_start %d, pos_dec_start %d\r\n", pos_stable_start, pos_dec_start);
    return PositionController_ConfigMove(
        controller_ptr,
        ctrl_req->pos_acc_start,
        ctrl_req->pos_goal,
        ctrl_req->speed_start,
        ctrl_req->speed_stable,
        ctrl_req->speed_end,
        pos_stable_start,
        pos_dec_start,
        ctrl_req->upcounting
    );
}

// --------------------------------------------------------------

static void PositionController_Abort(SOOL_PositionController* controller_ptr, uint8_t state) {

	switch ( state ) {

		case(1):
			controller_ptr->_state.aborted = 1;
			controller_ptr->_state.stage_active = SOOL_POSITION_CONTROLLER_FINISHED;
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

	// //printf("[PositionController_Process] current_pos %d,  status is %d\r\n", current_pos, status);
	return (status);

}

// --------------------------------------------------------------

static void PositionController_SelectNextState(SOOL_PositionController* controller_ptr) {

	if ( ++controller_ptr->_state.stage_active > 3 ) {
		controller_ptr->_state.stage_active = 0;
	}

	// printf("[PositionController_SelectNextState] switching to `stage_active` %d\r\n",
	// 	controller_ptr->_state.stage_active
	// );

}

// --------------------------------------------------------------

static uint8_t PositionController_GetMotionStatus(SOOL_PositionController* controller_ptr) {

	if ( controller_ptr->_state.aborted ) {

		// workaround
		if ( controller_ptr->base.IsFinished(&controller_ptr->base) ) {
			// printf("[PositionController_GetMotionStatus] aborted but SOOL_POSITION_CONTROLLER_FINISHED\r\n");
			return ((uint16_t)SOOL_POSITION_CONTROLLER_FINISHED);
		} else {
			// printf("[PositionController_GetMotionStatus] aborted but controller did not finish -> SOOL_POSITION_CONTROLLER_ACCELERATION\r\n");
			return ((uint16_t)SOOL_POSITION_CONTROLLER_ACCELERATION); // arbitrarily chosen, prevents the motion from being incorrectly interpreted (as finished)
		}

	}
	if (controller_ptr->_state.stage_active == 0) {
		// // printf("[PositionController_GetMotionStatus] stage_active SOOL_POSITION_CONTROLLER_FINISHED\r\n");
	} else if (controller_ptr->_state.stage_active == 1) {
		// // printf("[PositionController_GetMotionStatus] stage_active SOOL_POSITION_CONTROLLER_ACCELERATION\r\n");
	} else if (controller_ptr->_state.stage_active == 2) {
		// // printf("[PositionController_GetMotionStatus] stage_active SOOL_POSITION_CONTROLLER_STABLE\r\n");
	} else if (controller_ptr->_state.stage_active == 3) {
		// // printf("[PositionController_GetMotionStatus] stage_active SOOL_POSITION_CONTROLLER_DECELERATION\r\n");
	} else {
		// printf("[PositionController_GetMotionStatus] stage_active unknown\r\n");
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
			// printf("[PositionController_HandleAcceleration] Process is OK, but controller's state is being set to aborted\r\n");
			return (1);
		}

		if ( controller_ptr->base.IsFinished(&controller_ptr->base) ) {

			// printf("[PositionController_HandleAcceleration] Process is OK, but finished! will switch to stable and go further with the same speed\r\n");
			PositionController_SelectNextState(controller_ptr); // go to STABLE
			// go further with the same speed

		}
		return (1);

	} else if ( controller_ptr->base.IsFinished(&controller_ptr->base) ) {

		// sometimes a 0-length ramp will be requested and `Process`
		// will never return TRUE during that stage execution

		// abort anyway
		if ( controller_ptr->_state.aborted ) {
			// printf("[PositionController_HandleAcceleration] Process is FALSE, but motion finished! controller's state is being set to aborted\r\n");
			return (1);
		}

		// printf("[PositionController_HandleAcceleration] Process is FALSE, but motion finished! will switch to stable and go further with the same speed\r\n");
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
		// printf("[PositionController_HandleStableSpeed] controller's state is aborted, calling Process and returning its value\r\n");
		return (controller_ptr->base.Process(&controller_ptr->base, current_pos));
	}

	// TODO: safety in case of some `lag`? - consider `_config.upcounting` and `inequality`
	// evaluate whether to start decelerating
	if ( (controller_ptr->_config.upcounting && current_pos == controller_ptr->_config.soft_stop_start_pulse)
		|| (!controller_ptr->_config.upcounting && current_pos == controller_ptr->_config.soft_stop_start_pulse) ) {
		// printf("[PositionController_HandleStableSpeed] current_pos is bigger or equal to the soft_stop_start_pos (i.e. 3rd stage start)\r\n");

		// calculate how many `ticks` should deceleration take
		// difference between current position and the goal position
		int64_t duration = SOOL_Sensor_Encoder_PositionCalculator_ComputeDiff(current_pos, controller_ptr->_config.goal_pos, controller_ptr->_config.upcounting);
		// printf("[PositionController_HandleStableSpeed] deceleration stage should take %d `ticks`\r\n", (int)duration);

		// desired action
		if ( !controller_ptr->base.Reconfigure(&controller_ptr->base,
			  controller_ptr->_config.pwm_stable, controller_ptr->_config.pwm_goal, duration) ) {

			// FIXME: error handling, although it is very unlikely that the motion verified
			// at the configuration stage will not be feasible at that point
			//
			// with very short movements there, the `duration == 0` case may occur and just skip to the next state then
			if ( duration == 0 ) {
				// printf("[PositionController_HandleStableSpeed] Reconfigure failed - duration = 0 (no further actions)\r\n");
				// get into the next state
			} else if ( controller_ptr->_config.pwm_stable == controller_ptr->_config.pwm_goal ) {
				// workaround - change PWM values slightly and run again
				if ( controller_ptr->_config.pwm_goal >= 1 ) {
					// printf("[PositionController_HandleStableSpeed] Reconfigure failed - pwm_stable equal to pwm_goal; changing pwm_goal to %d\r\n", controller_ptr->_config.pwm_goal);
					controller_ptr->_config.pwm_goal--;
				} else {
					// printf("[PositionController_HandleStableSpeed] Reconfigure failed - pwm_stable equal to pwm_goal; changing pwm_stable to %d\r\n", controller_ptr->_config.pwm_stable);
					controller_ptr->_config.pwm_stable++;
				}
				// TODO: error handling
				// printf("[PositionController_HandleStableSpeed] Reconfigure failed - calling Reconfigure again after some minor changes in desired config\r\n");
				controller_ptr->base.Reconfigure(&controller_ptr->base, controller_ptr->_config.pwm_stable, controller_ptr->_config.pwm_goal, duration);
			} else {
				// printf("[PositionController_HandleStableSpeed] Reconfigure failed - requested safe slow-down on ERROR (calling abort and Reconfigure)\r\n");
				// safe slow-down on ERROR, assuming desired duration of the motion
				PositionController_Abort(controller_ptr, 1);
				controller_ptr->base.Reconfigure(&controller_ptr->base, controller_ptr->base.Get(&controller_ptr->base), controller_ptr->_config.pwm_goal, duration);
				// never got it in the debugger, but at least the motor will (almost?) stop safely
			}
		}
		controller_ptr->base.Start(&controller_ptr->base, current_pos);
		// printf("[PositionController_HandleStableSpeed] switching to DECELERATION\r\n");
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
			// printf("[PositionController_HandleDeceleration] Process is OK, but controller's state is being set to aborted\r\n");
			return (1);
		}

		if ( controller_ptr->base.IsFinished(&controller_ptr->base) ) {

			// printf("[PositionController_HandleDeceleration] Process is OK, but finished! will switch to finished\r\n");
			PositionController_SelectNextState(controller_ptr); // FINISHED
			// motion finished

		}
		return (1);

	} else if ( controller_ptr->base.IsFinished(&controller_ptr->base) ) {

		// sometimes a 0-length ramp will be requested and `Process`
		// will never return TRUE during that stage execution

		// abort anyway
		if ( controller_ptr->_state.aborted ) {
			// printf("[PositionController_HandleDeceleration] Process is FALSE, but motion finished! controller's state is being set to aborted\r\n");
			return (1);
		}

		// printf("[PositionController_HandleDeceleration] Process is FALSE, but motion finished! will switch to NEXT STATE\r\n");
		PositionController_SelectNextState(controller_ptr); // go to FINISHED

	}
	return (0);

}

// --------------------------------------------------------------

static uint8_t PositionController_HandleFinished(SOOL_PositionController* controller_ptr, uint32_t current_pos) {

	// check whether the internal FSM operation of the PositionController should be continued;
	// `aborted` status still allows to change speed via Process calls but abandons
	// acceleration-stable speed-deceleration-finished pattern
	if ( controller_ptr->_state.aborted ) {
		// printf("[PositionController_HandleFinished] controller's state aborted (calling Process and returning its value)\r\n");
		return (controller_ptr->base.Process(&controller_ptr->base, current_pos));
	} else {
		// //printf("[PositionController_HandleFinished] controller's state NOT set to aborted (typical operation)\r\n");
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
		// printf("[PositionController_RearrangeStages] Fail  /  (stage1 %d + stage2 %d) - stage_total %d = diff %d\r\n",
		// 	(int)stage1,
		// 	(int)stage2,
		// 	(int)diff
		// );
		return (0);
	}

	// check whether the shrinkage is not too aggressive
	if ( ((stage1 / 2) <= diff) || ((stage2 / 2) <= diff) ) {
		// printf("[PositionController_RearrangeStages] Too aggressive  /  stage1/2 %d + stage2/2 %d, diff %d\r\n",
		// 	(int)(stage1/2),
		// 	(int)(stage2/2),
		// 	(int)diff
		// );
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
		// printf("[PositionController_RearrangeStages] Successful  /  stage1_mod %d + stage1_mod %d, stage_total %d\r\n",
		// 	(int)(stage1_mod),
		// 	(int)(stage2_mod),
		// 	(int)(stage_total)
		// );
		return (2);
	}

	// printf("[PositionController_RearrangeStages] Failure2  /  stage1_mod %d + stage1_mod %d, stage_total %d\r\n",
	// 	(int)(stage1_mod),
	// 	(int)(stage2_mod),
	// 	(int)(stage_total)
	// );

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

	// flags
	uint8_t start_failed, stop_failed = 0;
	// evaluate feasibility of separate stages
	if ( !SOOL_Effector_SoftStarter_Reconfigure(&config_start, &setup_start, &state_start, pwm_start, *pwm_stable_ptr, stage1) ) {
		// investigate these cases, which are likely not a problem;
		// these conditions are copied from the SoftStarter's Reconfigure method (placed here in the inverted form)
		if ( !(pwm_start == *pwm_stable_ptr) || (stage1 == 0) ) {
			// printf("[PositionController_ShrinkStages] Fail 1\r\n");
			return (0);
		}
		start_failed = 1;

	}
	if ( !SOOL_Effector_SoftStarter_Reconfigure(&config_stop, &setup_stop, &state_stop, *pwm_stable_ptr, pwm_end, stage2) ) {
		// investigate these cases, which are likely not a problem;
		if ( !(*pwm_stable_ptr == pwm_end) || (stage2 == 0) ) {
			// printf("[PositionController_ShrinkStages] Fail 2\r\n");
			return (0);
		}
		stop_failed = 1;
	}

	// unable to find proper values
	if ( start_failed && stop_failed ) {
		// printf("[PositionController_ShrinkStages] Fail 3\r\n");
		return (0);
	}

	// movement has to be very short - let's hard-code that case, some stages may be abandoned
	if ( stage_total <= 3 ) {
		return (PositionController_HardCodeShortStages(stage_total, soft_start_end_pulse_ptr, soft_stop_start_pulse_ptr, pwm_stable_ptr, current_pos, goal_pos, pwm_start, upcounting));
	} else {
		// try to find setup values, maintaining a symmetry of the motion
		if ( start_failed && !stop_failed ) {
            // printf("[PositionController_ShrinkStages] PositionController_FindStageSetup (1)\r\n");
			setup_start = PositionController_FindStageSetup(stage1, setup_stop, stage2);
		} else if ( !start_failed && stop_failed ) {
            // printf("[PositionController_ShrinkStages] PositionController_FindStageSetup (2)\r\n");
			setup_stop = PositionController_FindStageSetup(stage2, setup_start, stage1);
		}
	}

	// calculate maximum speed for shortened stages
	int32_t pwm_stage1 = *pwm_stable_ptr;
	int32_t pwm_stage2 = *pwm_stable_ptr;

	// calculate new duration of shortened stages
	int64_t duration_stage1 = stage1;
	int64_t duration_stage2 = stage2;

	// modified value of the PWM pulse for the `stable` stage
	uint16_t pwm_stable_mod = *pwm_stable_ptr;

    // flag that indicate that some recovery was used to find feasible motion; prevents from being stuck in while
    uint8_t recovery_activated = 0;
	// `stage_total` is bigger than 3
	// change until the `intersection point` is found;
	// this while loop works for cases where (`stage_total >= 2`), see the related `small_range` flag
	while ( (duration_stage1 + duration_stage2) > (stage_total) ) { /* previously: (stage_total - 2) ) { */ // `length` condition; `-2` is a margin

        // Scale speeds - disabled ATM
		// NOTE: the trapezoidal pattern of motion ( rotational speed(pulses) function )
		// allows to assume that the `setup_start.pulse_change` is positive while
		// `setup_stop.pulse_change` is negative
		// pwm_stage1 -= setup_start.pulse_change;
		// pwm_stage2 += setup_stop.pulse_change;

		duration_stage1 -= setup_start.time_change_gap;
		duration_stage2 -= setup_stop.time_change_gap;
        // printf("[PositionController_ShrinkStages] WHILE: intersection pt search | stage1: pwm %d, duration %d | stage2: pwm %d, duration %d | stage_total %d\r\n",
        //     (int)pwm_stage1,
        //     (int)duration_stage1,
        //     (int)pwm_stage2,
        //     (int)duration_stage2,
        //     (int)stage_total
        // );

        // do not allow negative duration; check whether it is safe to apply some margin
		// TODO: case - one stage of some small length and the other with 0?;
		// HERE: both stages are similarly lengthened
        uint8_t duration_stage1_lowest_possible = (duration_stage1 - setup_start.time_change_gap) <= 0;
        uint8_t duration_stage2_lowest_possible = (duration_stage2 - setup_stop.pulse_change) <= 0;
        
        // remake to the symmetry of the motion despite the required assymetry and try again
        // potentially extend the "increment" of the shorter region to make motion feasible
        if (!recovery_activated && duration_stage1_lowest_possible && !duration_stage2_lowest_possible) {
            // printf("[PositionController_ShrinkStages] WHILE: intersection pt search (1)\r\n");
            setup_stop = PositionController_FindStageSetup(stage2, setup_start, stage1);
            duration_stage1 = stage1;
	        duration_stage2 = stage2;
            // @update pwm_stage1 and pwm_stage2 if changed in loop
            recovery_activated = 1;
        } else if (!recovery_activated && !duration_stage1_lowest_possible && duration_stage2_lowest_possible) {
            // printf("[PositionController_ShrinkStages] WHILE: intersection pt search (2)\r\n");
            setup_start = PositionController_FindStageSetup(stage1, setup_stop, stage2);
            duration_stage1 = stage1;
	        duration_stage2 = stage2;
            // @update pwm_stage1 and pwm_stage2 if changed in loop
            recovery_activated = 1;
        } else if (duration_stage1_lowest_possible || duration_stage2_lowest_possible) {
            if (recovery_activated) {
                // printf("[PositionController_ShrinkStages] WHILE: intersection pt search (3) - Ultimate recovery\r\n");
                // cannot find proper solution -> ignore velocity profile and make it simple as that
                duration_stage1 = stage_total / 2;
	            duration_stage2 = stage_total / 2 + (stage_total % 2);
                // @update pwm_stage1 and pwm_stage2 if changed in loop
            } else {
                // printf("[PositionController_ShrinkStages] Fail 4\r\n");
                return (0);
                break;
            }
        }
	}

	// set `pwm_stable` as the bigger `pwm_stageX`
	if ( pwm_stage1 < pwm_stage2 ) {
		pwm_stable_mod = pwm_stage2;
        // printf("[PositionController_ShrinkStages] speed DEC (%d) used as speed stable (vs ACC %d)\r\n", (int)pwm_stage2, (int)pwm_stage1);
	} else {
		pwm_stable_mod = pwm_stage1;
        // printf("[PositionController_ShrinkStages] speed ACC (%d) used as speed stable (vs DEC %d)\r\n", (int)pwm_stage1, (int)pwm_stage2);
	}

	// evaluate length of modified stages in regards to total motion `length`;
	// do not allow negative duration values
	if ( (duration_stage1 >= 0) && (duration_stage2 >= 0) && (duration_stage1 + duration_stage2) > stage_total ) {
		// printf("[PositionController_ShrinkStages] Fail 5\r\n");
		return (0);
	}

	// printf("[PositionController_ShrinkStages] current_pos %d, goal_pos %d \r\n"
	// 	"\t\tBEFORE: stage1_end_pos %d, stage3_start_pos %d / stage1 %d, stage2 %d, stage_total %d, pwm_stable %d \r\n",
	// 		(int)current_pos, (int)goal_pos,
	// 		(int)(*soft_start_end_pulse_ptr), (int)(*soft_stop_start_pulse_ptr),
    //         (int)stage1, (int)stage2, (int)stage_total, (int)(*pwm_stable_ptr)
	// );

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

	// printf("\t\tAFTER: stage1_end_pos %d, stage3_start_pos %d / stage1 %d, stage2 %d, stage_total %d, pwm_stable %d \r\n",
	// 		(int)current_pos, (int)goal_pos,
	// 		(int)(*soft_start_end_pulse_ptr), (int)(*soft_stop_start_pulse_ptr), (int)duration_stage1, (int)duration_stage2, (int)(*pwm_stable_ptr)
	// );

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

// --------------------------------------------------------------

static struct _SOOL_SoftStarterSetupStruct PositionController_FindStageSetup(uint32_t stage_length,
		struct _SOOL_SoftStarterSetupStruct setup_opposite, uint32_t stage_opposite_length)
{

	struct _SOOL_SoftStarterSetupStruct setup;

	// maintain symmetry of the stages
	int32_t factor = (stage_length * 100)/(stage_opposite_length); // multiplied for higher accuracy
	
    if (abs(factor) < 100) {
        // (int)(<1) -> 0
        int8_t sign = factor > 0 ? +1 : -1;
        factor = (int32_t)sign * 1; 
    } else {
        // valid integer
        factor /= 100;
    }

	setup.pulse_change = setup_opposite.pulse_change * factor * (-1); // acceleration vs deceleration / deceleration vs acceleration - pulse change will be opposite
	setup.time_change_gap = setup_opposite.time_change_gap * factor;
    // printf("PositionController_FindStageSetup:"
    //     "\r\n\t\tOpposite: stage_len %d pulse_change %d time_change_gap %d | "
    //     "\r\n\t\tSetup: stage_len %d, factor_init %d, factor_div %d, pulse_change %d, time_change_gap %d\r\n",
    //     (int)stage_opposite_length,
    //     (int)setup_opposite.pulse_change,
    //     (int)setup_opposite.time_change_gap,
    //     (int)stage_length,
    //     (int)((stage_length * 100)/(stage_opposite_length)),
    //     (int)factor,
    //     (int)setup.pulse_change,
    //     (int)setup.time_change_gap
    // );

   

	return (setup);

}
