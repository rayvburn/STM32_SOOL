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

static void SOOL_TimerOP_SetImmediateStart(volatile SOOL_TimerOnePulse *timer_op_ptr, FunctionalState state);
static void SOOL_TimerOP_Prepare(volatile SOOL_TimerOnePulse *timer_op_ptr);
static void SOOL_TimerOP_GeneratePulse(volatile SOOL_TimerOnePulse *timer_op_ptr);
static void SOOL_TimerOP_RestorePrevMode(volatile SOOL_TimerOnePulse *timer_op_ptr);
static uint8_t SOOL_TimerOP_InterruptHandler(volatile SOOL_TimerOnePulse *timer_op_ptr);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// helper
static volatile SOOL_TimerOnePulse SOOL_TimerOP_InitiatizeClass(volatile SOOL_TimerOutputCompare *timer_oc_ptr,
		uint16_t delay_time, FunctionalState trig_immediately,
		FunctionalState enable_slave_mode, uint16_t slave_mode, uint16_t input_trigger);

// DEPRECATED helper, may be useful when very fast output change must be generated (without tDELAY),
//			  but needs manual state change (toggle) on UPDT interrupt
//static void SOOL_Periph_TIMCompare_ForcedOCInit(volatile SOOL_TimerOnePulse *timer_op_ptr);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/**
 * Fully hardware-based single pulse generator.
 * Works with or without Update (TimerBasic) and CC (TimerOutputCompare) interrupts enabled.
 * Generating the waveform can be done in *output compare mode* or *PWM mode* (beware of that when initializing SOOL_TimerOutputCompare).
 *
 * @param timer_oc_ptr - SOOL_TimerOutputCompare instance which was already initialized (returned from _Init() function)
 * @param delay_time - additional delay (makes pulse a little longer), value incremented internally (by 1)
 * @param trig_immediately - when true counter is set to (FullCounterCYCLE - tPULSE - tDELAY - 2) before each pulse;
 *                           when false one will wait for a full timer cycle from 0 to overflow until pulse generation
 * @return SOOL_TimerOnePulse instance
 *
 * @note In datasheet they say that there is some limitation related to tDELAY (peripheral's inability to generate pulse
 *     	 on next clock cycle. It does occur indeed (at least with counter incremented each microsecond) so tDELAY
 *		 given in `constructor` is internally incremented (by 1).
 * @note Counter stops after each UEV (Update event, TIM_IT_Update flag setting) - needs to be started
 *       again (via base class' Start() for example or use RestorePrevMode()).
 * @note See Fig. 92 from RM0008, tPULSE is defined by (TIMx_ARR - TIMx_CCR1)
 * 		 which is symbolically equal to (TIM_Period - TIM_Pulse)
 */
volatile SOOL_TimerOnePulse SOOL_Periph_TIM_TimerOnePulse_Init(volatile SOOL_TimerOutputCompare *timer_oc_ptr,
		uint16_t delay_time, FunctionalState trig_immediately) {

	/* Instance of OnePulse `class` */
	volatile SOOL_TimerOnePulse timer = SOOL_TimerOP_InitiatizeClass(timer_oc_ptr, delay_time,
										trig_immediately, DISABLE, 0, 0);

	return (timer);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/**
 *
 * Slave mode is not tested at all.
 * For details see @ref SOOL_Periph_TIM_TimerOnePulse_Init.
 * @param timer_oc_ptr
 * @param delay_time
 * @param trig_immediately
 * @param slave_mode
 * @param input_trigger
 * @return
 */
volatile SOOL_TimerOnePulse SOOL_Periph_TIM_TimerOnePulse_InitSlave(volatile SOOL_TimerOutputCompare *timer_oc_ptr,
		uint16_t delay_time, FunctionalState trig_immediately,
		uint16_t slave_mode, uint16_t input_trigger) {

	/* Instance of OnePulse `class` */
	volatile SOOL_TimerOnePulse timer = SOOL_TimerOP_InitiatizeClass(timer_oc_ptr, delay_time,
										trig_immediately, ENABLE, slave_mode, input_trigger);

	return (timer);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void SOOL_TimerOP_SetImmediateStart(volatile SOOL_TimerOnePulse *timer_op_ptr, FunctionalState state) {
	timer_op_ptr->_setup.trig_immediately = (uint8_t)state;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void SOOL_TimerOP_Prepare(volatile SOOL_TimerOnePulse *timer_op_ptr) {

	// Some info from reference manual
	/*
	 * A pulse can be correctly generated only if the compare value is different from the counter
	 * initial value. Before starting (when the timer is waiting for the trigger), the configuration must
	 * be:
	 * 	o In upcounting: CNT < CCRx <= ARR (in particular, 0 < CCRx)
	 * 	o In downcounting: CNT > CCRx
	 * ***
	 * NOTE: upcounting is default (hard-coded) mode for TimerBasic (base class of TimerOC class)
	 *
	 * The OPM waveform is defined by writing the compare registers (taking into account the
	 * clock frequency and the counter prescaler).
	 */


	/* The tDELAY is defined by the value written in the TIMx_CCRy register. */
	// tDELAY is time from triggering event to start of a pulse
	SOOL_Periph_TIMCompare_SetCCR(timer_op_ptr->base.base._setup.TIMx, timer_op_ptr->base._setup.TIM_Channel_x,
						   timer_op_ptr->_setup.delay_time + 1); // unable to generate with `0` tDELAY


	/* Check whether to set CNT (counter) accordingly to allow pulse start at the next ( + 1) clock
	 * cycle or at the (DELAY + 1) clock pulse from now if delay is non-zero. */
	if ( timer_op_ptr->_setup.trig_immediately ) {

		/* The tPULSE is defined by the difference between the auto-reload value and the compare
		 * value (TIMx_ARR - TIMx_CCR1).
		 * ***
		 * Symbolically equal to: (TIM_Period - TIM_Pulse)
		 * Here - tDELAY is considered too */

		// adjust TIM_CNT to force pulse start at the next clock cycle
		timer_op_ptr->base.base._setup.TIMx->CNT = (uint16_t)
				(timer_op_ptr->base.base._setup.TIMx->ARR - timer_op_ptr->base._setup.oc_config.TIM_Pulse -
				 timer_op_ptr->_setup.delay_time - 1);

		// 1) ^ applies some delay but pulse is generated (both edges) fully by hardware
		// 2) below generates edge in an instant but needs manual toggle in ISR (DEPRECATED)

		// DEPRECATED, works but needs manual setting `inactive` during TIM_IT_Update handler
		//SOOL_Periph_TIMCompare_ForcedOCInit(timer_op_ptr);

	} else {

		timer_op_ptr->base.base._setup.TIMx->CNT = (uint16_t)0;

	}

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// handy wrapper
static void SOOL_TimerOP_GeneratePulse(volatile SOOL_TimerOnePulse *timer_op_ptr) {

	/* Disable OnePulse Mode (it seems that counter could not be enabled with OPMode enabled) */
	SOOL_TimerOP_DisableOPMode(timer_op_ptr);
	/* Stop the counter */
	timer_op_ptr->base.Stop(&timer_op_ptr->base);
	/* Update CCR and CNT registers to force proper pulse delay and its length (respectively) */
	SOOL_TimerOP_Prepare(timer_op_ptr);
	/* Start the counter */
	timer_op_ptr->base.Start(&timer_op_ptr->base);
	/* Enable OnePulse Mode */
	SOOL_TimerOP_EnableOPMode(timer_op_ptr);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// handy wrapper
static void SOOL_TimerOP_RestorePrevMode(volatile SOOL_TimerOnePulse *timer_op_ptr) {

	/* Disable OnePulse Mode (it seems that counter could not be enabled with OPMode enabled) */
	SOOL_TimerOP_DisableOPMode(timer_op_ptr);
	/* Start the counter */
	timer_op_ptr->base.Start(&timer_op_ptr->base);

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

	/* Restore the TIM_Pulse value of the TimerOutputCompare instance */
	SOOL_Periph_TIMCompare_SetCCR(timer_op_ptr->base.base._setup.TIMx, timer_op_ptr->base._setup.TIM_Channel_x,
						   timer_op_ptr->base._setup.oc_config.TIM_Pulse);

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

static volatile SOOL_TimerOnePulse SOOL_TimerOP_InitiatizeClass(volatile SOOL_TimerOutputCompare *timer_oc_ptr,
		uint16_t delay_time, FunctionalState trig_immediately,
		FunctionalState enable_slave_mode, uint16_t slave_mode, uint16_t input_trigger)

{

	// RM0008, p. 327, Section 14.3.15 "One-pulse mode"
	/* Instance of OnePulse `class` */
	volatile SOOL_TimerOnePulse timer;

	/* Check correctness of OutputCompare timer (CCRx <= ARR)
	 * NOTE: VALID FOR UPCOUNTING ONLY! */
	if ( SOOL_Periph_TIMCompare_GetCCR(timer_oc_ptr->base._setup.TIMx, timer_oc_ptr->_setup.TIM_Channel_x) >
		 timer_oc_ptr->base._setup.TIMx->ARR )
	{
		// timer not configured properly, hopefully will throw some error
		return (timer);
	}

	/* Copy Timer running in Output Compare Mode (base `class`) */
	timer.base = *timer_oc_ptr;

	/* Make sure timer is disabled during configuration procedure */
	timer.base.Stop(&timer.base);

	/* Save `class` fields */
	timer._setup.delay_time = delay_time;
	timer._setup.trig_immediately = (uint8_t)trig_immediately; // little memory save (unsigned int -> uint8_t)

	/* Save member functions */
	timer.DisableOPMode = SOOL_TimerOP_DisableOPMode;
	timer.EnableOPMode = SOOL_TimerOP_EnableOPMode;
	timer.SetImmediateStart = SOOL_TimerOP_SetImmediateStart;
	timer.Prepare = SOOL_TimerOP_Prepare;
	timer.GeneratePulse = SOOL_TimerOP_GeneratePulse;
	timer.RestorePrevMode = SOOL_TimerOP_RestorePrevMode;
	timer._InterruptHandler = SOOL_TimerOP_InterruptHandler;

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

//static void SOOL_Periph_TIMCompare_ForcedOCInit(volatile SOOL_TimerOnePulse *timer_op_ptr) {
//
//	switch (timer_op_ptr->base._setup.TIM_Channel_x) {
//
//	case(TIM_Channel_1):
//		TIM_ForcedOC1Config(timer_op_ptr->base.base._setup.TIMx, TIM_ForcedAction_Active);
//		break;
//	case(TIM_Channel_2):
//		TIM_ForcedOC2Config(timer_op_ptr->base.base._setup.TIMx, TIM_ForcedAction_Active);
//		break;
//	case(TIM_Channel_3):
//		TIM_ForcedOC3Config(timer_op_ptr->base.base._setup.TIMx, TIM_ForcedAction_Active);
//		break;
//	case(TIM_Channel_4):
//		TIM_ForcedOC4Config(timer_op_ptr->base.base._setup.TIMx, TIM_ForcedAction_Active);
//		break;
//	default:
//		break;
//	}
//
//}
