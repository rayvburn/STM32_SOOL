/*
 * BTS7960.c
 *
 *  Created on: 17.09.2019
 *      Author: user
 */

#include "sool/Effectors/BTS7960/BTS7960.h"

static uint8_t	SOOL_BTS7960_Stop(volatile SOOL_BTS7960* driver_ptr);
static uint8_t 	SOOL_BTS7960_SetDirection(volatile SOOL_BTS7960* driver_ptr, SOOL_BTS7960_Direction dir);
static uint8_t 	SOOL_BTS7960_SetSpeed(volatile SOOL_BTS7960* driver_ptr, SOOL_BTS7960_Direction dir, uint16_t speed);
static uint16_t SOOL_BTS7960_GetSpeed(volatile SOOL_BTS7960* driver_ptr, SOOL_BTS7960_Direction dir);
static uint8_t 	SOOL_BTS7960_GetOvercurrentStatus(volatile SOOL_BTS7960* driver_ptr, SOOL_BTS7960_Direction dir);

// --------------------------------------------------------------------------------------------------

volatile SOOL_BTS7960 SOOL_Effector_BTS7960_Init(volatile SOOL_TimerBasic timer_base,
												 volatile SOOL_TimerOutputCompare pwm_fwd,
												 volatile SOOL_TimerOutputCompare pwm_rev,
												 SOOL_PinSwitch en_fwd,
												 SOOL_PinSwitch en_rev,
												 volatile SOOL_Button fault_fwd,
												 volatile SOOL_Button fault_rev)
{

	/* New instance */
	volatile SOOL_BTS7960 driver;

	/* Save internal base classes */
	driver.base_tim = timer_base;
	driver.base_pwm_fwd = pwm_fwd;
	driver.base_pwm_rev = pwm_rev;
	driver.base_en_fwd = en_fwd;
	driver.base_en_rev = en_rev;
	driver.base_fault_fwd = fault_fwd;
	driver.base_fault_rev = fault_rev;

	/* Assign private functions to members */
	driver.Stop = SOOL_BTS7960_Stop;
	driver.SetDirection = SOOL_BTS7960_SetDirection;
	driver.SetSpeed = SOOL_BTS7960_SetSpeed;
	driver.GetSpeed = SOOL_BTS7960_GetSpeed;
	driver.GetOvercurrentStatus = SOOL_BTS7960_GetOvercurrentStatus;

	return (driver);

}

// --------------------------------------------------------------------------------------------------

void SOOL_Effector_BTS7960_Startup(volatile SOOL_BTS7960* driver_ptr) {

	/* Make sure the motor is stopped */
	driver_ptr->Stop(driver_ptr);

	/* Motor up fault detector */
	driver_ptr->base_fault_fwd.base.EnableEXTI(&driver_ptr->base_fault_fwd.base);
	driver_ptr->base_fault_fwd.base.EnableNVIC(&driver_ptr->base_fault_fwd.base);

	/* Motor down fault detector */
	driver_ptr->base_fault_rev.base.EnableEXTI(&driver_ptr->base_fault_rev.base);
	driver_ptr->base_fault_rev.base.EnableNVIC(&driver_ptr->base_fault_rev.base);

}

// --------------------------------------------------------------------------------------------------

static uint8_t SOOL_BTS7960_Stop(volatile SOOL_BTS7960* driver_ptr) {
	driver_ptr->base_en_fwd.SetLow(&driver_ptr->base_en_fwd);
	driver_ptr->base_en_rev.SetLow(&driver_ptr->base_en_rev);
	return (1);
}

// --------------------------------------------------------------------------------------------------

static uint8_t SOOL_BTS7960_SetDirection(volatile SOOL_BTS7960* driver_ptr, SOOL_BTS7960_Direction dir) {

	switch (dir) {

	case (BTS7960_FORWARD):
		driver_ptr->base_en_rev.SetLow(&driver_ptr->base_en_rev);
		driver_ptr->base_en_fwd.SetHigh(&driver_ptr->base_en_fwd);
		return (1);
		break;

	case(BTS7960_REVERSE):
		driver_ptr->base_en_fwd.SetLow(&driver_ptr->base_en_fwd);
		driver_ptr->base_en_rev.SetHigh(&driver_ptr->base_en_rev);
		return (1);
		break;

	default:
		return (0);
		break;

	}

}

// --------------------------------------------------------------------------------------------------

static uint8_t SOOL_BTS7960_SetSpeed(volatile SOOL_BTS7960* driver_ptr, SOOL_BTS7960_Direction dir, uint16_t speed) {

	// speed value range
	if ( !(speed >= 0 && speed < 0xFFFF) ) {
		return (0);
	}

	switch (dir) {

	case (BTS7960_FORWARD):
		driver_ptr->base_pwm_fwd.SetPulse(&driver_ptr->base_pwm_fwd, speed);
		return (1);
		break;

	case(BTS7960_REVERSE):
		driver_ptr->base_pwm_rev.SetPulse(&driver_ptr->base_pwm_rev, speed);
		return (1);
		break;

	default:
		return (0);
		break;

	}

}

// --------------------------------------------------------------------------------------------------

static uint16_t SOOL_BTS7960_GetSpeed(volatile SOOL_BTS7960* driver_ptr, SOOL_BTS7960_Direction dir) {

	switch (dir) {

	case (BTS7960_FORWARD):
		return (driver_ptr->base_pwm_fwd._setup.oc_config.TIM_Pulse);
		break;

	case(BTS7960_REVERSE):
		return (driver_ptr->base_pwm_rev._setup.oc_config.TIM_Pulse);
		break;

	default:
		return (0);
		break;

	}

}

// --------------------------------------------------------------------------------------------------

static uint8_t SOOL_BTS7960_GetOvercurrentStatus(volatile SOOL_BTS7960* driver_ptr, SOOL_BTS7960_Direction dir) {

	switch (dir) {

	case (BTS7960_FORWARD):
		return (driver_ptr->base_fault_fwd.GetPushedFlag(&driver_ptr->base_fault_fwd));
		break;

	case(BTS7960_REVERSE):
		return (driver_ptr->base_fault_rev.GetPushedFlag(&driver_ptr->base_fault_rev));
		break;

	default:
		return (0);
		break;

	}

}

// --------------------------------------------------------------------------------------------------
