/*
 * PositionController.h
 *
 *  Created on: 07.02.2020
 *      Author: user
 */

#ifndef INC_SOOL_EFFECTORS_POSITIONCONTROLLER_POSITIONCONTROLLER_H_
#define INC_SOOL_EFFECTORS_POSITIONCONTROLLER_POSITIONCONTROLLER_H_

#include <sool/Effectors/SoftStarter/SoftStarterCustom.h>

// --------------------------------------------------------------

struct _SOOL_PositionControllerStruct;
typedef struct _SOOL_PositionControllerStruct SOOL_PositionController;

// --------------------------------------------------------------

struct _SOOL_PositionControllerConfigStruct {

	uint16_t pwm_start;
	uint16_t pwm_stable;
	uint16_t pwm_goal;

	int64_t goal_pos;

//	uint32_t soft_start_end_pulse;
	uint32_t soft_stop_start_pulse;

};

// --------------------------------------------------------------

typedef enum {
	SOOL_POSITION_CONTROLLER_FINISHED = 0u,
	SOOL_POSITION_CONTROLLER_ACCELERATION,
	SOOL_POSITION_CONTROLLER_STABLE,
	SOOL_POSITION_CONTROLLER_DECELERATION,
} SOOL_PositionControllerStatus;


struct _SOOL_PositionControllerStateStruct {
	uint8_t stage_active; // 0 - finished. 1 - acceleration, 2 - stable going, 3 - deceleration
	uint8_t aborted;
};

// --------------------------------------------------------------

// must be interfaced with some kind of encoder of something
// BEWARE that at the moment SOOL_SoftStarterCustom is not prepared to process negative position values
// consists of an internal state machine
struct _SOOL_PositionControllerStruct {

	// derived
	SOOL_SoftStarterCustom		base;

	// configuration
	struct _SOOL_PositionControllerConfigStruct _config;
	struct _SOOL_PositionControllerStateStruct _state; // ?

	// @brief Configures motion pattern, actual output can be achieved via Get calls
	//    preceded by Process
	// @note ??? To softly stop at the goal position set the motion_smoothing_pulses equal to the difference between goal and current positions
	// @note Treat the velocity plot as a trapezoid (acceleration ramp and deceleration ramp)
	//
	// controller_ptr:
	// current_pos: current position expressed as pulses number
	// goal_pos: goal position expressed as pulses number
	// pwm_start: TIM pulse value at the start of the acceleration
	// pwm_stable: TIM pulse value after finished acceleration and at the start of deceleration
	// pwm_goal: TIM pulse value at the goal position
	// soft_start_end_pulse: absolute pulse number where the acceleration should finish
	// soft_stop_start_pulse: absolute pulse number where the deceleration should begin
	uint8_t (*ConfigMove)(SOOL_PositionController* controller_ptr, int64_t current_pos, int64_t goal_pos,
			uint16_t pwm_start, uint16_t pwm_stable, uint16_t pwm_goal, uint32_t soft_start_end_pulse,
			uint32_t soft_stop_start_pulse);

	// calculates pulse value
	uint8_t (*Process)(SOOL_PositionController* controller_ptr, int64_t current_pos);

	// ????
	// unusual situation happened - must stop the controller operation?
	void (*Abort)(SOOL_PositionController* controller_ptr, uint8_t state);

	uint8_t (*GetMotionStatus)(SOOL_PositionController* controller_ptr);
	uint16_t (*GetOutput)(SOOL_PositionController* controller_ptr);

};

// --------------------------------------------------------------

extern SOOL_PositionController SOOL_Effector_PositionController_Initialize();

// --------------------------------------------------------------

#endif /* INC_SOOL_EFFECTORS_POSITIONCONTROLLER_POSITIONCONTROLLER_H_ */
