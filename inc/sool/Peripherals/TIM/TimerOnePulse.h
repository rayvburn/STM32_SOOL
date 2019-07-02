/*
 * TimerOnePulse.h
 *
 *  Created on: 30.06.2019
 *      Author: user
 */

#ifndef INC_SOOL_PERIPHERALS_TIM_TIMERONEPULSE_H_
#define INC_SOOL_PERIPHERALS_TIM_TIMERONEPULSE_H_

#include "TimerOutputCompare.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/* Forward declaration */
struct _SOOL_TimerOnePulseStruct;
typedef struct _SOOL_TimerOnePulseStruct SOOL_TimerOnePulse;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

//struct _SOOL_TimerOnePulseStateStruct {
//
//};

struct _SOOL_TimerOnePulseSetupStruct {

	uint16_t delay_time;
	uint8_t trig_immediately;

};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/**
 * Composition of SOOL_TimerOutputCompare and SOOL_TimerOnePulse class' contents
 * Unluckily there is no way to access base `class` members directly.
 * IMPORTANT: in case of derived classes which are interrupt-related
 * one must remember to call base's interrupt handlers too
 */
struct _SOOL_TimerOnePulseStruct {

	// base section --------------------------------------------------------
	SOOL_TimerOutputCompare base;

	// derived section -----------------------------------------------------
//	struct _SOOL_TimerOnePulseStateStruct _state;
	struct _SOOL_TimerOnePulseSetupStruct _setup;

	/**
	 * Does not start counter
	 * Does not call Prepare() internally
	 * @param
	 */
	void (*EnableOPMode)(volatile SOOL_TimerOnePulse*);

	/**
	 * Does not restart the counter
	 * @param
	 */
	void (*DisableOPMode)(volatile SOOL_TimerOnePulse*);

	/**
	 * Changes trig_immediately flag
	 * @param
	 * @param
	 */
	void (*SetImmediateStart)(volatile SOOL_TimerOnePulse*, FunctionalState);

	/**
	 * Prepares CNT (counter) value according to TIM_Period (TIMx_ARR), changes TIMx_CCRy (delay) too.
	 * Useful in non-slave mode.
	 * @param
	 */
	void (*Prepare)(volatile SOOL_TimerOnePulse*);

	/**
	 * Calls few `class` functions and generates pulse in a typical way (`wrapper`).
	 * Timer's NVIC channel needs to be enabled before.
	 * @param
	 */
	void (*GeneratePulse)(volatile SOOL_TimerOnePulse*);

	/**
	 * Another `wrapper` for a typical utilization.
	 * To restore previous timer configuration DisableOPMode() must be called first
	 * AND TimerOutputCompare's Start() function must called next (enables counter).
	 * Calling DisableOPMode() (or this function) before pulse's end will not produce
	 * a proper signal.
	 * @param
	 */
	void (*RestorePrevMode)(volatile SOOL_TimerOnePulse*);

	/**
	 * Update event interrupt handler
	 * @param
	 * @return
	 */
	uint8_t (*_InterruptHandler)(volatile SOOL_TimerOnePulse*);

};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

extern volatile SOOL_TimerOnePulse SOOL_Periph_TIM_TimerOnePulse_Init(volatile SOOL_TimerOutputCompare *timer_oc_ptr,
		uint16_t delay_time, FunctionalState trig_immediately);
extern volatile SOOL_TimerOnePulse SOOL_Periph_TIM_TimerOnePulse_InitSlave(volatile SOOL_TimerOutputCompare *timer_oc_ptr,
		uint16_t delay_time, FunctionalState trig_immediately, uint16_t slave_mode, uint16_t input_trigger);

#endif /* INC_SOOL_PERIPHERALS_TIM_TIMERONEPULSE_H_ */
