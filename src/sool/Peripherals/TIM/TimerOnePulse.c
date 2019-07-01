/*
 * TimerOnePulse.c
 *
 *  Created on: 30.06.2019
 *      Author: user
 */

#include "sool/Peripherals/TIM/TimerOnePulse.h"
#include "sool/Peripherals/TIM/TimerCompare_common.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void SOOL_TimerOP_EnableOPMode(volatile SOOL_TimerOnePulse *timer_op_ptr);
static void SOOL_TimerOP_DisableOPMode(volatile SOOL_TimerOnePulse *timer_op_ptr);

static void SOOL_TimerOP_Prepare(volatile SOOL_TimerOnePulse *timer_op_ptr);
static uint8_t SOOL_TimerOP_InterruptHandler(volatile SOOL_TimerOnePulse *timer_op_ptr);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// helper
static volatile SOOL_TimerOnePulse SOOL_TimerOP_InitiatizeClass(volatile SOOL_TimerOutputCompare *timer_oc_ptr,
		uint16_t delay_time, FunctionalState trig_immediately,
		FunctionalState enable_slave_mode, uint16_t slave_mode, uint16_t input_trigger);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// @note Counter stops after each UEV (Update event, TIM_IT_Update flag setting)
// @note See Fig. 92 from RM0008, tPULSE is defined by (TIMx_ARR - TIMx_CCR1)
//       which is symbolically equal to (TIM_Period - TIM_Pulse)
// @note Generating the waveform can be done in !output compare mode! or !PWM mode!
volatile SOOL_TimerOnePulse SOOL_Periph_TIM_TimerOnePulse_Init(volatile SOOL_TimerOutputCompare *timer_oc_ptr,
		uint16_t delay_time, FunctionalState trig_immediately) {

	/* Instance of OnePulse `class` */
	volatile SOOL_TimerOnePulse timer = SOOL_TimerOP_InitiatizeClass(timer_oc_ptr, delay_time,
										trig_immediately, DISABLE, 0, 0);

	return (timer);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

volatile SOOL_TimerOnePulse SOOL_Periph_TIM_TimerOnePulse_InitSlave(volatile SOOL_TimerOutputCompare *timer_oc_ptr,
		uint16_t delay_time, FunctionalState trig_immediately,
		uint16_t slave_mode, uint16_t input_trigger) {

	/* Instance of OnePulse `class` */
	volatile SOOL_TimerOnePulse timer = SOOL_TimerOP_InitiatizeClass(timer_oc_ptr, delay_time,
										trig_immediately, ENABLE, slave_mode, input_trigger);

	return (timer);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// useful only in non-slave mode
static void SOOL_TimerOP_Prepare(volatile SOOL_TimerOnePulse *timer_op_ptr) {

	// ********************
	/*
	 * A pulse can be correctly generated only if the compare value is different from the counter
	 * initial value. Before starting (when the timer is waiting for the trigger), the configuration must
	 * be:
	 * 	o In upcounting: CNT < CCRx <= ARR (in particular, 0 < CCRx)
	 * 	o In downcounting: CNT > CCRx
	 * ***
	 * NOTE: upcounting is default (hard-coded) mode for TimerBasic (base class of TimerOC class)
	 */
	/*
	 * The OPM waveform is defined by writing the compare registers (taking into account the
	 * clock frequency and the counter prescaler).
	 */
	// ********************


	/* The tDELAY is defined by the value written in the TIMx_CCRy register. */
	// tDELAY is time from triggering event to start of a pulse
	SOOL_Periph_TIM_SetCCR(timer_op_ptr->base.base._setup.TIMx, timer_op_ptr->base._setup.TIM_Channel_x,
						   timer_op_ptr->_setup.delay_time);


	/* Check whether to set CNT (counter) accordingly to allow pulse start at the next ( + 1) clock
	 * cycle or at the (DELAY + 1) clock pulse from now if delay is non-zero. */
	if ( timer_op_ptr->_setup.trig_immediately == ENABLE ) {

		/* The tPULSE is defined by the difference between the auto-reload value and the compare
		 * value (TIMx_ARR - TIMx_CCR1).
		 * ***
		 * Symbolically equal to: (TIM_Period - TIM_Pulse)
		 * Here - tDELAY is considered too */

		// adjust TIM_CNT to force pulse start at the next clock cycle
		timer_op_ptr->base.base._setup.TIMx->CNT = (uint16_t)
				(timer_op_ptr->base.base._setup.TIMx->ARR - timer_op_ptr->base._setup.oc_config.TIM_Pulse -
				 timer_op_ptr->_setup.delay_time - 1);

	} else {

		timer_op_ptr->base.base._setup.TIMx->CNT = (uint16_t)0;

	}

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void SOOL_TimerOP_EnableOPMode(volatile SOOL_TimerOnePulse *timer_op_ptr) {

	/* "You select One-pulse mode by setting the OPM bit in the TIMx_CR1 register.
	 * This makes the counter stop automatically at the next update event UEV."
	 * ***
	 * Configure the OPM Mode, OPMode_Single hard-coded here, Repetitive is available too */
	/* Set the OPM Bit */
	timer_op_ptr->base.base._setup.TIMx->CR1 |= (uint16_t)TIM_OPMode_Single;  // equal to TIM_CR1_OPM;

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void SOOL_TimerOP_DisableOPMode(volatile SOOL_TimerOnePulse *timer_op_ptr) {

	/* Reset the OPM Bit */
	timer_op_ptr->base.base._setup.TIMx->CR1 &= (uint16_t)~((uint16_t)TIM_CR1_OPM);

	/* Reset counter */
	timer_op_ptr->base.base._setup.TIMx->CNT = (uint16_t)0x0000;

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t SOOL_TimerOP_InterruptHandler(volatile SOOL_TimerOnePulse *timer_op_ptr) {

	/* Check if update interrupt flag of the timer is set */
	if ( TIM_GetITStatus(timer_op_ptr->base.base._setup.TIMx, TIM_IT_Update) == RESET) {
		// not this timer overflowed (different IRQn)
		return (0);
	}

	/* Check if OnePulse mode is selected */
	if ( (((uint16_t)timer_op_ptr->base.base._setup.TIMx->CR1) & ((uint16_t)TIM_OPMode_Single)) == 0 ) {
		return (0);
	}

	/* Clear IT pending bit */
	timer_op_ptr->base.base._setup.TIMx->SR = (uint16_t)~TIM_IT_Update; // TIM_ClearITPendingBit(timer->_TIMx, TIM_IT_Update);
	return (1);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// timer output compare must be initialized before passing here
// slave mode is not tested

// immediately after delay_time has elapsed! when false a (FullCounterCYCLE - tPULSE - tDELAY)
// will elapse until pulse generation

// enables one pulse mode

static volatile SOOL_TimerOnePulse SOOL_TimerOP_InitiatizeClass(volatile SOOL_TimerOutputCompare *timer_oc_ptr,
		uint16_t delay_time, FunctionalState trig_immediately,
		FunctionalState enable_slave_mode, uint16_t slave_mode, uint16_t input_trigger)

{

	// RM0008, p. 327, Section 14.3.15 "One-pulse mode"
	/* Instance of OnePulse `class` */
	volatile SOOL_TimerOnePulse timer;

	/* Check correctness of OutputCompare timer (CCRx <= ARR)
	 * NOTE: VALID FOR UPCOUNTING ONLY! */
	if ( SOOL_Periph_TIM_GetCCR(timer_oc_ptr->base._setup.TIMx, timer_oc_ptr->_setup.TIM_Channel_x) >
		 timer_oc_ptr->base._setup.TIMx->ARR ) {

		// timer not configured properly, hopefully will throw some error
		return (timer);

	}

	/* Copy Timer running in Output Compare Mode (base `class`) */
	timer.base = *timer_oc_ptr;

	/* Make sure timer is disabled during configuration procedure */
	timer.base.Stop(&timer.base);

	/* Save `class` fields */
	timer._setup.delay_time = delay_time;
	timer._setup.trig_immediately = trig_immediately;

	/* Save member functions */
	timer.DisableOPMode = SOOL_TimerOP_DisableOPMode;
	timer.EnableOPMode = SOOL_TimerOP_EnableOPMode;
	timer.Prepare = SOOL_TimerOP_Prepare;
	timer._InterruptHandler = SOOL_TimerOP_InterruptHandler;

	/* Prepare for pulse generation */
	SOOL_TimerOP_EnableOPMode(&timer);
	SOOL_TimerOP_Prepare(&timer);

	// TODO: experimental, not tested
	if ( enable_slave_mode == ENABLE ) {

		/* Input Trigger selection */
		TIM_SelectInputTrigger(timer.base.base._setup.TIMx, input_trigger);

		/* Slave Mode selection: Trigger Mode */
		TIM_SelectSlaveMode(timer.base.base._setup.TIMx, slave_mode);

	}

	return (timer);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
