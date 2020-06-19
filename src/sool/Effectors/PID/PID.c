/*
 * PID.c
 *
 *  Created on: 19.06.2020
 *      Author: user
 */

#include <sool/Effectors/PID/PID.h>
#include <sool/Peripherals/TIM/SystickTimer.h>

static uint8_t PID_Compute(SOOL_PID *pid_ptr, float input, float setpoint);
static void PID_SetTuningsAdv(SOOL_PID *pid_ptr, float kp, float ki, float kd, enum PIDProportional p_on);
static void PID_SetTunings(SOOL_PID *pid_ptr, float kp, float ki, float kd);
static void PID_SetSampleTime(SOOL_PID *pid_ptr, uint32_t new_sample_time);
static void PID_SetOutputLimits(SOOL_PID *pid_ptr, float min, float max);
static void PID_SetMode(SOOL_PID *pid_ptr, enum PIDMode mode);
static void PID_Initialize(SOOL_PID *pid_ptr);
static void PID_SetControllerDirection(SOOL_PID *pid_ptr, enum PIDDirection direction);
static float PID_GetOutput(SOOL_PID *pid_ptr);

// * constructor.  links the PID to the Input, Output, and
//   Setpoint.  Initial tuning parameters are also set here
// DEFAULT: POn -> P_ON_E
//
// https://github.com/br3ttb/Arduino-PID-Library/blob/master/PID_v1.cpp#L44
SOOL_PID SOOL_Effector_PID_Init(float kp, float ki, float kd, int controller_direction) {
	return (SOOL_Effector_PID_InitAdv(kp, ki, kd, PID_P_ON_E, controller_direction));
}

// * constructor.  links the PID to the Input, Output, and
//   Setpoint.  Initial tuning parameters are also set here.
//   (overload for specifying proportional mode)
//
// https://github.com/br3ttb/Arduino-PID-Library/blob/master/PID_v1.cpp#L20
SOOL_PID SOOL_Effector_PID_InitAdv(float kp, float ki, float kd, int p_on, int controller_direction) {

	// `class` instance
	SOOL_PID pid;

	pid._setup.output = 0.0f;

	pid._config.in_auto = 0;

    PID_SetOutputLimits(&pid, 0, 255);			//default output limit corresponds to
												//the arduino pwm limits

    pid._config.sample_time = 100;				//default Controller Sample Time is 0.1 seconds

    PID_SetControllerDirection(&pid, controller_direction);
    PID_SetTuningsAdv(&pid, kp, ki, kd, p_on);

    // force trigger at first execution of Compute
    pid._config.last_time = SOOL_Periph_TIM_SysTick_GetMillis() - pid._config.sample_time;

    // `methods`
    pid.Compute = PID_Compute;
    pid.GetOutput = PID_GetOutput;
    pid.SetControllerDirection = PID_SetControllerDirection;
    pid.SetMode = PID_SetMode;
    pid.SetOutputLimits = PID_SetOutputLimits;
    pid.SetSampleTime = PID_SetSampleTime;
    pid.SetTunings = PID_SetTunings;
    pid.SetTuningsAdv = PID_SetTuningsAdv;

    return (pid);
}

/* Compute() **********************************************************************
 *     This, as they say, is where the magic happens.  this function should be called
 *   every time "void loop()" executes.  the function will decide for itself whether a new
 *   pid Output needs to be computed.  returns true when the output is computed,
 *   false when nothing has been done.
 *
 *   https://github.com/br3ttb/Arduino-PID-Library/blob/master/PID_v1.cpp#L58
 **********************************************************************************/
static uint8_t PID_Compute(SOOL_PID *pid_ptr, float input, float setpoint) {

   if(!pid_ptr->_config.in_auto) return 0;
   uint32_t now = SOOL_Periph_TIM_SysTick_GetMillis();
   uint32_t timeChange = (now - pid_ptr->_config.last_time);
   if(timeChange >= pid_ptr->_config.sample_time)
   {
	  /*Compute all the working error variables*/
	  float error = setpoint - input;
	  float dInput = (input - pid_ptr->_setup.last_input);
	  pid_ptr->_setup.output_sum += (pid_ptr->_setup.ki * error);

	  /*Add Proportional on Measurement, if P_ON_M is specified*/
	  if(!pid_ptr->_config.p_on_e) pid_ptr->_setup.output_sum-= pid_ptr->_setup.kp * dInput;

	  if(pid_ptr->_setup.output_sum > pid_ptr->_setup.out_max) pid_ptr->_setup.output_sum = pid_ptr->_setup.out_max;
	  else if(pid_ptr->_setup.output_sum < pid_ptr->_setup.out_min) pid_ptr->_setup.output_sum = pid_ptr->_setup.out_min;

	  /*Add Proportional on Error, if P_ON_E is specified*/
	   float output;
	  if(pid_ptr->_config.p_on_e) output = pid_ptr->_setup.kp * error;
	  else output = 0;

	  /*Compute Rest of PID Output*/
	  output += pid_ptr->_setup.output_sum - pid_ptr->_setup.kd * dInput;

		if(output > pid_ptr->_setup.out_max) output = pid_ptr->_setup.out_max;
	  else if(output < pid_ptr->_setup.out_min) output = pid_ptr->_setup.out_min;
		pid_ptr->_setup.output = output;

	  /*Remember some variables for next time*/
		pid_ptr->_setup.last_input = input;
		pid_ptr->_config.last_time = now;
		return 1;
   }
   else return 0;

}

/* SetTunings(...)*************************************************************
 * This function allows the controller's dynamic performance to be adjusted.
 * it's called automatically from the constructor, but tunings can also
 * be adjusted on the fly during normal operation
 *
 * https://github.com/br3ttb/Arduino-PID-Library/blob/master/PID_v1.cpp#L102
 ******************************************************************************/
static void PID_SetTuningsAdv(SOOL_PID *pid_ptr, float kp, float ki, float kd, enum PIDProportional p_on) {
   if (kp<0 || ki<0 || kd<0) return;

   pid_ptr->_config.p_on = p_on;
   pid_ptr->_config.p_on_e = p_on == PID_P_ON_E;

   // not needed
   //dispKp = Kp; dispKi = Ki; dispKd = Kd;

   float SampleTimeInSec = ((float)pid_ptr->_config.sample_time)/1000;
   pid_ptr->_setup.kp = kp;
   pid_ptr->_setup.ki = ki * SampleTimeInSec;
   pid_ptr->_setup.kd = kd / SampleTimeInSec;

  if(pid_ptr->_config.controller_direction == PID_DIRECTION_REVERSE)
   {
	  pid_ptr->_setup.kp = (0 - pid_ptr->_setup.kp);
	  pid_ptr->_setup.ki = (0 - pid_ptr->_setup.ki);
	  pid_ptr->_setup.kd = (0 - pid_ptr->_setup.kd);
   }
}

/* SetTunings(...)*************************************************************
 * Set Tunings using the last-rembered POn setting
 *
 * https://github.com/br3ttb/Arduino-PID-Library/blob/master/PID_v1.cpp#L127
 ******************************************************************************/
static void PID_SetTunings(SOOL_PID *pid_ptr, float kp, float ki, float kd) {
	PID_SetTuningsAdv(pid_ptr, kp, ki, kd, pid_ptr->_config.p_on);
}

/* SetSampleTime(...) *********************************************************
 * sets the period, in Milliseconds, at which the calculation is performed
 *
 * https://github.com/br3ttb/Arduino-PID-Library/blob/master/PID_v1.cpp#L134
 ******************************************************************************/
static void PID_SetSampleTime(SOOL_PID *pid_ptr, uint32_t new_sample_time) {
   if (new_sample_time > 0)
   {
      float ratio  = (float)new_sample_time
                      / (float)pid_ptr->_config.sample_time;
      pid_ptr->_setup.ki *= ratio;
      pid_ptr->_setup.kd /= ratio;
      pid_ptr->_config.sample_time = (uint32_t)new_sample_time;
   }
}

/* SetOutputLimits(...)****************************************************
 *     This function will be used far more often than SetInputLimits.  while
 *  the input to the controller will generally be in the 0-1023 range (which is
 *  the default already,)  the output will be a little different.  maybe they'll
 *  be doing a time window and will need 0-8000 or something.  or maybe they'll
 *  want to clamp it from 0-125.  who knows.  at any rate, that can all be done
 *  here.
 *
 *  https://github.com/br3ttb/Arduino-PID-Library/blob/master/PID_v1.cpp#L154
 **************************************************************************/
static void PID_SetOutputLimits(SOOL_PID *pid_ptr, float min, float max) {
   if(min >= max) return;
   pid_ptr->_setup.out_min = min;
   pid_ptr->_setup.out_max = max;

   if(pid_ptr->_config.in_auto)
   {
	   if(pid_ptr->_setup.output > pid_ptr->_setup.out_max) pid_ptr->_setup.output = pid_ptr->_setup.out_max;
	   else if(pid_ptr->_setup.output < pid_ptr->_setup.out_min) pid_ptr->_setup.output = pid_ptr->_setup.out_min;

	   if(pid_ptr->_setup.output_sum > pid_ptr->_setup.out_max) pid_ptr->_setup.output_sum = pid_ptr->_setup.out_max;
	   else if(pid_ptr->_setup.output_sum < pid_ptr->_setup.out_min) pid_ptr->_setup.output_sum = pid_ptr->_setup.out_min;
   }
}

/* SetMode(...)****************************************************************
 * Allows the controller Mode to be set to manual (0) or Automatic (non-zero)
 * when the transition from manual to auto occurs, the controller is
 * automatically initialized
 *
 * https://github.com/br3ttb/Arduino-PID-Library/blob/master/PID_v1.cpp#L175
 ******************************************************************************/
static void PID_SetMode(SOOL_PID *pid_ptr, enum PIDMode mode) {
    uint8_t newAuto = (mode == PID_MODE_AUTOMATIC);
    if(newAuto && !pid_ptr->_config.in_auto)
    {  /*we just went from manual to auto*/
        PID_Initialize(pid_ptr);
    }
    pid_ptr->_config.in_auto = newAuto;
}

/* Initialize()****************************************************************
 *	does all the things that need to happen to ensure a bumpless transfer
 *  from manual to automatic mode.
 *
 *  https://github.com/br3ttb/Arduino-PID-Library/blob/master/PID_v1.cpp#L189
 ******************************************************************************/
static void PID_Initialize(SOOL_PID *pid_ptr) {
   pid_ptr->_setup.output_sum = pid_ptr->_setup.output;
   pid_ptr->_setup.last_input = 0.0f;
   if(pid_ptr->_setup.output_sum > pid_ptr->_setup.out_max) pid_ptr->_setup.output_sum = pid_ptr->_setup.out_max;
   else if(pid_ptr->_setup.output_sum < pid_ptr->_setup.out_min) pid_ptr->_setup.output_sum = pid_ptr->_setup.out_min;
}

/* SetControllerDirection(...)*************************************************
 * The PID will either be connected to a DIRECT acting process (+Output leads
 * to +Input) or a REVERSE acting process(+Output leads to -Input.)  we need to
 * know which one, because otherwise we may increase the output when we should
 * be decreasing.  This is called from the constructor.
 *
 * https://github.com/br3ttb/Arduino-PID-Library/blob/master/PID_v1.cpp#L203
 ******************************************************************************/
static void PID_SetControllerDirection(SOOL_PID *pid_ptr, enum PIDDirection direction) {
   if(pid_ptr->_config.in_auto && (uint8_t)direction != pid_ptr->_config.controller_direction)
   {
	   pid_ptr->_setup.kp = (0 - pid_ptr->_setup.kp);
	   pid_ptr->_setup.ki = (0 - pid_ptr->_setup.ki);
	   pid_ptr->_setup.kd = (0 - pid_ptr->_setup.kd);
   }
   pid_ptr->_config.controller_direction = (uint8_t)direction;
}

static float PID_GetOutput(SOOL_PID *pid_ptr) {
	return (pid_ptr->_setup.output);
}
