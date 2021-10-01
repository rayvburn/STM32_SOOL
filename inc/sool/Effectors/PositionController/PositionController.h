/*
 * PositionController.h
 *
 *  Created on: 07.02.2020
 *      Author: user
 */

#ifndef INC_SOOL_EFFECTORS_POSITIONCONTROLLER_POSITIONCONTROLLER_H_
#define INC_SOOL_EFFECTORS_POSITIONCONTROLLER_POSITIONCONTROLLER_H_

#include <sool/Effectors/SoftStarter/SoftStarterCustom.h>
#include <sool/Sensors/Encoder/PositionCalculator.h>

// --------------------------------------------------------------

struct PositionControlAbsRequest {
	uint32_t pos_acc_start;
	uint32_t pos_stable_start;
	uint32_t pos_dec_start;
	uint32_t pos_goal;
    uint8_t upcounting;
	uint16_t speed_start;
	uint16_t speed_stable;
	uint16_t speed_end;
};

struct PositionControlRelRequest {
	uint32_t pos_acc_start;
	int64_t pos_acc_len;
	int64_t pos_dec_len;
	uint32_t pos_goal;
    uint8_t upcounting;
	uint16_t speed_start;
	uint16_t speed_stable;
	uint16_t speed_end;
};

// --------------------------------------------------------------

struct _SOOL_PositionControllerStruct;
typedef struct _SOOL_PositionControllerStruct SOOL_PositionController;

// --------------------------------------------------------------

struct _SOOL_PositionControllerConfigStruct {

	uint16_t pwm_start;				/// initial speed TIM_Pulse value (i.e. pulse width)
	uint16_t pwm_stable;			/// speed during the second stage of motion (i.e. pulse width)
	uint16_t pwm_goal;				/// speed at the end of a motion

	uint32_t goal_pos;				/// goal position (expressed in encoder ticks - absolute value)
	uint32_t soft_stop_start_pulse; /// an absolute value of encoder ticks, while the 3rd stage (deceleration) must begin

	uint8_t	upcounting;				/// whether encoder ticks will be increasing (1) or decreasing (0) during the active motion

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

struct _SOOL_PositionControllerStruct {

	// derived
	SOOL_SoftStarterCustom		base;

	// - - - - - - - - - - - - - - - - - - - - - - - - - -

	// configuration
	struct _SOOL_PositionControllerConfigStruct _config;
	struct _SOOL_PositionControllerStateStruct 	_state;

	/**
	 * @brief Configures motion pattern in 3 stages: acceleration, stable speed, deceleration.
	      An actual output can be achieved via @ref Get calls preceded by @ref Process
	 * @note Treat the plot of a velocity in regards to TIM pulses plot as a trapezoid
	 *    (acceleration ramp and deceleration ramp).
	 * @note The stoppage of the motor at the end of the motion in impossible with this algorithm.
	 *    One must consider the minimum PWM value that allows the motor to keep rotating (@ref pwm_goal)
	 *    and then, after finished motion, stop manually (immediately).
	 *
	 * @param controller_ptr
	 * @param current_pos: current position expressed as pulses number
	 * @param goal_pos: goal position expressed as pulses number
	 * @param pwm_start: TIM pulse value at the start of the acceleration
	 * @param pwm_stable: TIM pulse value after finished acceleration and at the start of deceleration
	 * @param pwm_goal: TIM pulse value at the goal position
	 * @param soft_start_end_pulse: absolute pulse number where the acceleration should finish
	 * @param soft_stop_start_pulse: absolute pulse number where the deceleration should begin
	 * @param upcounting: whether the encoder pulses will increase during the following motion (0 if will be decreasing)
	 * @return 1 if motion configured properly and will be executed, 0 otherwise
	 */
	uint8_t (*ConfigMove)(SOOL_PositionController* controller_ptr, uint32_t current_pos, uint32_t goal_pos,
			uint16_t pwm_start, uint16_t pwm_stable, uint16_t pwm_goal,
			uint32_t soft_start_end_pulse, uint32_t soft_stop_start_pulse, uint8_t upcounting);

    /**
     * @brief See ConfigMove for details
     * 
     * Argument list is replaced by PositionControlAbsRequest struct
     */
    uint8_t (*ConfigMoveStructAbs)(SOOL_PositionController* controller_ptr, struct PositionControlAbsRequest* ctrl_req);

    /**
     * @brief See ConfigMove for details
     * 
     * Argument list is replaced by PositionControlRelRequest struct
     */
    uint8_t (*ConfigMoveStructRel)(SOOL_PositionController* controller_ptr, struct PositionControlRelRequest* ctrl_req);

	/**
	 * @brief Tries to calculate a new pulse value based on the current position @ref current_pos
	 * @param controller_ptr
	 * @param current_pos: current number of encoder pulses
	 * @return 1 if the value was modified, 0 otherwise
	 */
	uint8_t (*Process)(SOOL_PositionController* controller_ptr, uint32_t current_pos);

	/**
	 * @brief Useful when unusual situation happened - must stop the controller operation.
	 *    One can still use a `SoftStart` interface achievable via @ref base `class`
	 *    and configure a new motion. The `PositionController`'s Process call will
	 *    produce appropriate output according to the programmed `SoftStart` class
	 * @param controller_ptr
	 * @param state: 1 aborts the lastly programmed PositionController motion
	 * @note Abort is internally resetted (@ref state == 0) in the @ref ConfigMove
	 */
	void (*Abort)(SOOL_PositionController* controller_ptr, uint8_t state);

	/**
	 * @brief Returns the current status of a programmed motion.
	 * @param controller_ptr
	 * @return Number connected with @ref SOOL_PositionControllerStatus enum
	 */
	uint8_t (*GetMotionStatus)(SOOL_PositionController* controller_ptr);

	/**
	 * @brief Returns an output variable produced by the `controller`
	 * @param controller_ptr
	 * @return Pulse value prepared for a timer
	 */
	uint16_t (*GetOutput)(SOOL_PositionController* controller_ptr);

};

// --------------------------------------------------------------

/**
 * @brief A `constructor` of a `PositionController` class, which must be interfaced
 *   with some kind of encoder or other sensor.
 *   Beware that at the moment @ref SOOL_SoftStarterCustom is not prepared to process
 *   negative position values, although it can be easily interfaced with an encoder connected
 *   to a hardware timer (TIM_CNT register is 16-bit - uint16_t).
 *   The `PositionController` consists of an internal state machine, whose status
 *   can be achieved via the @ref GetMotionStatus.
 *   Provides a soft-start and soft-stop procedure.
 *
 * @note See @ref ConfigMove method documentation for further details and usage instructions.
 * @return An instance of @ref SOOL_PositionController
 */
extern SOOL_PositionController SOOL_Effector_PositionController_Initialize();

// --------------------------------------------------------------

#endif /* INC_SOOL_EFFECTORS_POSITIONCONTROLLER_POSITIONCONTROLLER_H_ */
