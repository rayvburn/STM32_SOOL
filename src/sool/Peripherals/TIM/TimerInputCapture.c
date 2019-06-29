/*
 * TimerInputCapture.c
 *
 *  Created on: 28.06.2019
 *      Author: user
 */

#include "sool/Peripherals/TIM/TimerInputCapture.h"
#include "sool/Peripherals/TIM/TimerCompare_common.h"

static void SOOL_TimerInputCapture_Start(volatile SOOL_TimerInputCapture *tim_ic_ptr);
static void SOOL_TimerInputCapture_Stop(volatile SOOL_TimerInputCapture *tim_ic_ptr);
static uint16_t SOOL_TimerInputCapture_GetSavedCounterVal(const volatile SOOL_TimerInputCapture *tim_ic_ptr);
static uint8_t SOOL_TimerInputCapture_InterruptHandler(volatile SOOL_TimerInputCapture *tim_ic_ptr);

// helper
static uint16_t SOOL_TimerInputCapture_GetCaptureRegisterValue(const volatile SOOL_TimerInputCapture *tim_ic_ptr);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/**
 * SOOL_TimerInputCapture's initializer, this `class` uses SOOL_TimerBasic `class` as a base
 * and overrides SOOL_TimerBasic's start()
 *
 * @param TIMx: TIMx where x can be 1 to 17 to select the TIM peripheral.
 * @param prescaler: (-1) Specifies the prescaler value used to divide the TIM clock.
 * @param period: (-1) Specifies the period value to be loaded into the active Auto-Reload Register at the next update event.
 * @param channel: Specifies the TIM channel. This parameter can be a value of @ref TIM_Channel
 * @param ic_polarity: Specifies the active edge of the input signal. This parameter can be a value of @ref TIM_Input_Capture_Polarity
 * @return SOOL_TimerInputCapture `class` instance
 * @note prescaler and period parameters must be a number between 0x0000 and 0xFFFF.
 */
volatile SOOL_TimerInputCapture SOOL_Periph_TIM_TimerInputCapture_Init(TIM_TypeDef* TIMx,
		uint16_t prescaler, uint16_t period, uint16_t channel, uint16_t ic_polarity) {

	/* TimerBasic `constructor` - time base must be configured and basic
	 * timer activity must be run to allow input capture mode work properly */
	volatile SOOL_TimerBasic tim_basic = SOOL_Periph_TIM_TimerBasic_Init(TIMx, prescaler, period, SOOL_PERIPH_TIM_IRQ_CC);

	/* Object to be returned from the initializer */
	volatile SOOL_TimerInputCapture timer;

	/* Variables useful for configuration */
	uint16_t tim_it_cc = 0x00; 		// not valid value by default
	uint16_t tim_flag_cc_of = 0x00;	// not valid value by default

	/* Select proper variable values based on a given channel identifier */
	switch (channel) {

	case(TIM_Channel_1):	// TI1
		tim_it_cc = TIM_IT_CC1;
		tim_flag_cc_of = TIM_FLAG_CC1OF;
		break;

	case(TIM_Channel_2):	// TI2
		tim_it_cc = TIM_IT_CC2;
		tim_flag_cc_of = TIM_FLAG_CC2OF;
		break;

	case(TIM_Channel_3):	// TI3
		tim_it_cc = TIM_IT_CC3;
		tim_flag_cc_of = TIM_FLAG_CC3OF;
		break;

	case(TIM_Channel_4):	// TI4
		tim_it_cc = TIM_IT_CC4;
		tim_flag_cc_of = TIM_FLAG_CC4OF;
		break;

	}

	/* Configure Input capture mode */
	// RM0008, p. 384, "General-purpose timers (TIM2 to TIM5)"
	TIM_ICInitTypeDef tim_ic;
	TIM_ICStructInit(&tim_ic); // default values

	tim_ic.TIM_Channel = channel;
	tim_ic.TIM_ICFilter = 0xA;
	tim_ic.TIM_ICPolarity = ic_polarity;
	tim_ic.TIM_ICPrescaler = 0; // p. 384: "we wish the capture to be performed at each valid transition, so the prescaler is disabled"
	tim_ic.TIM_ICSelection = TIM_ICSelection_DirectTI; // 'remapping' possible
	TIM_ICInit(TIMx, &tim_ic);

	/* Enable interrupts */
	TIM_ITConfig(TIMx, tim_it_cc, ENABLE);

	/* Save class' fields */
	timer._setup.TIM_Channel_X = channel;
	timer._setup.TIM_IT_CCx = tim_it_cc;
	timer._setup.TIM_FLAG_CCxOF = tim_flag_cc_of;

	/* Save base */
	timer.base = tim_basic;

	/* Set initial state */
	timer._state.transition_counter_val = 0;
	timer._state.transition_detected = 0;

	/* Set function pointers */
	timer.Start = SOOL_TimerInputCapture_Start;
	timer.Stop = SOOL_TimerInputCapture_Stop;
	timer.GetSavedCounterVal = SOOL_TimerInputCapture_GetSavedCounterVal;
	timer._InterruptHandler = SOOL_TimerInputCapture_InterruptHandler;

	return (timer);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void SOOL_TimerInputCapture_Start(volatile SOOL_TimerInputCapture *tim_ic_ptr) {

	tim_ic_ptr->_state.transition_counter_val = 0;
	tim_ic_ptr->_state.transition_detected = 0;
	tim_ic_ptr->base.Start(&tim_ic_ptr->base);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void SOOL_TimerInputCapture_Stop(volatile SOOL_TimerInputCapture *tim_ic_ptr) {

	// this calls unmodified version of base class, it is redefined just for consistence
	tim_ic_ptr->base.Stop(&tim_ic_ptr->base);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint16_t SOOL_TimerInputCapture_GetSavedCounterVal(const volatile SOOL_TimerInputCapture *tim_ic_ptr) {
	return (tim_ic_ptr->_state.transition_counter_val);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t SOOL_TimerInputCapture_InterruptHandler(volatile SOOL_TimerInputCapture *tim_ic_ptr) {

	/* Check if update interrupt flag of the timer is set */
	if (TIM_GetITStatus(tim_ic_ptr->base._setup.TIMx, tim_ic_ptr->_setup.TIM_IT_CCx) == RESET) {
		// nothing to do (different IRQn or another flag)
		return (0);
	}

	/* RM, p.384
	 * In order to handle the overcapture, it is recommended to read the data before the
	 * overcapture flag. This is to avoid missing an overcapture which could happen after reading
	 * the flag and before reading the data.
	 */

	/* Update Timer's state */
	//tim_ic_ptr->_state.transition_counter_val = SOOL_TimerInputCapture_GetCaptureRegisterValue(tim_ic_ptr);
	tim_ic_ptr->_state.transition_counter_val = SOOL_Periph_TIM_GetCCR(tim_ic_ptr->base._setup.TIMx, tim_ic_ptr->_setup.TIM_Channel_X);
	tim_ic_ptr->_state.transition_detected = 1;

	/* Clear the overcapture flag */
	tim_ic_ptr->base._setup.TIMx->SR = (uint16_t)~tim_ic_ptr->_setup.TIM_FLAG_CCxOF;

	/* Clear IT pending bit */
	tim_ic_ptr->base._setup.TIMx->SR = (uint16_t)~tim_ic_ptr->_setup.TIM_IT_CCx; // TIM_ClearITPendingBit(timer->_TIMx, TIM_IT_Update);
	return (1);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

//static uint16_t SOOL_TimerInputCapture_GetCaptureRegisterValue(const volatile SOOL_TimerInputCapture *tim_ic_ptr) {
//
//	switch (tim_ic_ptr->_setup.TIM_Channel_X) {
//
//	case(TIM_Channel_1):
//		return (tim_ic_ptr->tim_basic._TIMx->CCR1);
//		break;
//	case(TIM_Channel_2):
//		return (tim_ic_ptr->tim_basic._TIMx->CCR2);
//		break;
//	case(TIM_Channel_3):
//		return (tim_ic_ptr->tim_basic._TIMx->CCR3);
//		break;
//	case(TIM_Channel_4):
//		return (tim_ic_ptr->tim_basic._TIMx->CCR4);
//		break;
//	default:
//		return (0);
//	}
//
//}
