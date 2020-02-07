/*
 * PositionController.c
 *
 *  Created on: 07.02.2020
 *      Author: user
 */

#include <sool/Effectors/PositionController/PositionController.h>

static uint8_t 	PositionController_ConfigMove(SOOL_PositionController* controller_ptr, int64_t current_pos, int64_t goal_pos,
											  uint16_t pwm_start, uint16_t pwm_stable, uint16_t pwm_goal,
											  uint32_t soft_start_end_pulse, uint32_t soft_stop_start_pulse);
static void 	PositionController_Abort(SOOL_PositionController* controller_ptr, uint8_t state);
static uint8_t 	PositionController_Process(SOOL_PositionController* controller_ptr, int64_t current_pos);
static uint8_t 	PositionController_GetMotionStatus(SOOL_PositionController* controller_ptr);
static uint16_t PositionController_GetOutput(SOOL_PositionController* controller_ptr);

// helpers
static void PositionController_SelectNextState(SOOL_PositionController* controller_ptr);


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
			uint16_t pwm_start, uint16_t pwm_stable, uint16_t pwm_goal, uint32_t soft_start_pulses, uint32_t soft_stop_pulses)
{

	// update internal configuration structure
	controller_ptr->_config.goal_pos = goal_pos;
	controller_ptr->_config.pwm_goal = pwm_goal;
	controller_ptr->_config.pwm_stable = pwm_stable;
	controller_ptr->_config.pwm_start  = pwm_start;
//	controller_ptr->_config.soft_start_end_pulse = soft_start_pulses;
	controller_ptr->_config.soft_stop_start_pulse  = soft_stop_pulses;

	// arrange acceleration according to the given set of parameters
	if ( !controller_ptr->base.Reconfigure(&controller_ptr->base,
									 	    controller_ptr->_config.pwm_start, controller_ptr->_config.pwm_stable,
											soft_start_pulses) )
	{
		return (0);
	}

	controller_ptr->base.Start(&controller_ptr->base, current_pos);
	controller_ptr->_state.stage_active = (uint8_t)SOOL_POSITION_CONTROLLER_ACCELERATION;
	return (1);

}

// --------------------------------------------------------------

static void PositionController_Abort(SOOL_PositionController* controller_ptr, uint8_t state) {

	switch ( state ) {

		case(1):
			controller_ptr->_state.aborted = 1;
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

	if ( controller_ptr->_state.aborted ) {
		return (0);
	}

	if ( controller_ptr->base.Process(&controller_ptr->base, current_pos) ) {
		if ( controller_ptr->base.IsFinished(&controller_ptr->base) ) {

			// the stage which has just finished
			switch ( controller_ptr->_state.stage_active ) {

				case(SOOL_POSITION_CONTROLLER_ACCELERATION):
					PositionController_SelectNextState(controller_ptr); // STABLE
					// go further with the same speed
					break;

				case(SOOL_POSITION_CONTROLLER_STABLE):
					// evaluate whether to start decelerating
					if ( current_pos == controller_ptr->_config.soft_stop_start_pulse ) {

						PositionController_SelectNextState(controller_ptr); // DECELERATION
						// TODO: error handling
						if ( !controller_ptr->base.Reconfigure(&controller_ptr->base,
							  controller_ptr->_config.pwm_stable, controller_ptr->_config.pwm_goal,
							  controller_ptr->_config.soft_stop_start_pulse) )
						{
							return (0);
						}

					}
					break;

				case(SOOL_POSITION_CONTROLLER_DECELERATION):
					PositionController_SelectNextState(controller_ptr); // FINISHED
					// motion finished
					break;

				case(SOOL_POSITION_CONTROLLER_FINISHED):
					// lack of `SelectNextState` -> do not allow switch to `acceleration` after reaching the goal and waiting for the new one (` == 0 `))
					break;

			}

		}
		return (1);
	}
	return (0);

}

// --------------------------------------------------------------

static void PositionController_SelectNextState(SOOL_PositionController* controller_ptr) {

	if ( ++controller_ptr->_state.stage_active > 3 ) {
		controller_ptr->_state.stage_active = 0;
	}

}

// --------------------------------------------------------------

static uint8_t PositionController_GetMotionStatus(SOOL_PositionController* controller_ptr) {
	return ((uint8_t)controller_ptr->_state.stage_active);
}

// --------------------------------------------------------------

static uint16_t PositionController_GetOutput(SOOL_PositionController* controller_ptr) {
	return (controller_ptr->base.Get(&controller_ptr->base));
}

// --------------------------------------------------------------
