/*
 * PID.h
 *
 *  Created on: 19.06.2020
 *      Author: user
 */

#ifndef INC_SOOL_EFFECTORS_PID_PID_H_
#define INC_SOOL_EFFECTORS_PID_PID_H_

#include <stdint.h>
#include "PID_common.h"

/// NOTE: this is br3ttb's `Arduino-PID-Library` adapted for SOOL coding standards
/// https://github.com/br3ttb/Arduino-PID-Library

struct _SOOL_PID_SetupStruct {
	float kp;                  // * (P)roportional Tuning Parameter
	float ki;                  // * (I)ntegral Tuning Parameter
	float kd;                  // * (D)erivative Tuning Parameter

	// * Pointers to the Input, Output, and Setpoint variables
	//   This creates a hard link between the variables and the
    //   PID, freeing the user from having to constantly tell us
	//   what these values are.  with pointers we'll just know.
	float output;

	float output_sum, last_input;
	float out_min, out_max;
};

/* Forward declaration */
typedef struct _SOOL_PID_Struct SOOL_PID;

struct _SOOL_PID_Struct {
	struct _SOOL_PID_SetupStruct 	_setup;
	struct _SOOL_PID_ConfigStruct	_config; // common

	// * sets PID to either Manual (0) or Auto (non-0)
	void (*SetMode)(SOOL_PID *pid_ptr, enum PIDMode mode);

	// * performs the PID calculation.  it should be
	//   called every time loop() cycles. ON/OFF and
	//   calculation frequency can be set using SetMode
	//   SetSampleTime respectively
	uint8_t (*Compute)(SOOL_PID *pid_ptr, float input, float setpoint);

	// * clamps the output to a specific range. 0-255 by default, but
	//   it's likely the user will want to change this depending on
	//   the application
	void (*SetOutputLimits)(SOOL_PID *pid_ptr, float min, float max);

	//available but not commonly used functions ********************************************************
	// * While most users will set the tunings once in the
	//   constructor, this function gives the user the option
	//   of changing tunings during runtime for Adaptive control
	// * overload for specifying proportional mode
	void (*SetTunings)(SOOL_PID *pid_ptr, float kp, float ki, float kd);
	void (*SetTuningsAdv)(SOOL_PID *pid_ptr, float kp, float ki, float kd, enum PIDProportional p_on);

	// * Sets the Direction, or "Action" of the controller. DIRECT
	//   means the output will increase when error is positive. REVERSE
	//   means the opposite.  it's very unlikely that this will be needed
	//   once it is set in the constructor.
	void (*SetControllerDirection)(SOOL_PID *pid_ptr, enum PIDDirection direction);

	// * sets the frequency, in Milliseconds, with which
	//   the PID calculation is performed.  default is 100
	void (*SetSampleTime)(SOOL_PID *pid_ptr, uint32_t new_sample_time);

	float (*GetOutput)(SOOL_PID *pid_ptr);
};

// NOTE: requires systick to be configured @ 1000 Hz
// * constructor.  links the PID to the Input, Output, and
//   Setpoint.  Initial tuning parameters are also set here
extern SOOL_PID SOOL_Effector_PID_Init(float kp, float ki, float kd, int controller_direction);

// * constructor.  links the PID to the Input, Output, and
//   Setpoint.  Initial tuning parameters are also set here.
//   (overload for specifying proportional mode)
extern SOOL_PID SOOL_Effector_PID_InitAdv(float kp, float ki, float kd, int p_on, int controller_direction);

#endif /* INC_SOOL_EFFECTORS_PID_PID_H_ */
