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

static void SOOL_TimerInputCapture_EnableNVIC(volatile SOOL_TimerInputCapture *tim_ic_ptr);
static void SOOL_TimerInputCapture_DisableNVIC(volatile SOOL_TimerInputCapture *tim_ic_ptr);

static uint16_t SOOL_TimerInputCapture_GetSavedCounterVal(const volatile SOOL_TimerInputCapture *tim_ic_ptr);
static uint8_t SOOL_TimerInputCapture_InterruptHandler(volatile SOOL_TimerInputCapture *tim_ic_ptr);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// TODO:
///* TIM4_CH2 pin (PB.07) configuration */
//GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
//GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;

/**
 *
 *
 * @param TIMx: TIMx where x can be 1 to 17 to select the TIM peripheral.
 * @param prescaler: (-1) Specifies the prescaler value used to divide the TIM clock.
 * @param period: (-1) Specifies the period value to be loaded into the active Auto-Reload Register at the next update event.
 * @param enable_int_update: if true TIM_IT_Update is enabled
 * @param channel: Specifies the TIM channel. This parameter can be a value of @ref TIM_Channel
 * @param ic_polarity: Specifies the active edge of the input signal. This parameter can be a value of @ref TIM_Input_Capture_Polarity
 * @param enable_int_cc: if true TIM_IT_CCx is enabled
 * @return SOOL_TimerInputCapture `class` instance
 * @note prescaler and period parameters must be a number between 0x0000 and 0xFFFF.
 *
 */

// TODO
/**
 * SOOL_TimerInputCapture's initializer, this `class` uses SOOL_TimerBasic `class` as a base
 * and overrides SOOL_TimerBasic's Start().
 *
 * Usually the input capture pin is configured as GPIO_Mode_IN_FLOATING
 *
 * @param tim_basic_ptr - initialized (i.e. acquired from _Init()) SOOL_TimerBasic instance
 * @param channel - Specifies the TIM channel. This parameter can be a value of @ref TIM_Channel
          Channel of the timer's compare mode is closely related to wiring.
          Due to hard-coded ICSelection parameter, remapping is not available.
 * @param ic_polarity - whether to capture on rising or falling edge.
 * 		  Specifies the active edge of the input signal. This parameter can be a value of @ref TIM_Input_Capture_Polarity
 * 		  IMPORTANT: TIM_ICPolarity_BothEdge mode DOES NOT WORK (at least in STM32F103C8T6)
 * @param enable_int_cc - whether to generate interrupts on capture event, if true TIM_IT_CCx is enabled
 * @return
 */
volatile SOOL_TimerInputCapture SOOL_Periph_TIM_TimerInputCapture_Init(volatile SOOL_TimerBasic *tim_basic_ptr,
		uint16_t channel, uint16_t ic_polarity, FunctionalState enable_int_cc) {

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
	TIM_ICInit(tim_basic_ptr->_setup.TIMx, &tim_ic);

	/* Configure NVIC if needed */
	NVIC_InitTypeDef nvic;
	nvic.NVIC_IRQChannel = SOOL_Periph_TIM_GetIRQnType(tim_basic_ptr->_setup.TIMx, SOOL_PERIPH_TIM_IRQ_CC); // for safety moved outside `if`

	if ( enable_int_cc == ENABLE ) {

		nvic.NVIC_IRQChannelPreemptionPriority = 0;
		nvic.NVIC_IRQChannelSubPriority = 0;

		/* Some useful notes in Base class */
		nvic.NVIC_IRQChannelCmd = DISABLE;
		NVIC_Init(&nvic);

		/* IT_Update surely needs to be cleared when `NVIC_IRQChannelCmd == ENABLE` */
		//TIMx->SR = (uint16_t)~TIM_IT_Update;

	}

	/* Enable interrupts */
	TIM_ITConfig(tim_basic_ptr->_setup.TIMx, tim_it_cc, enable_int_cc);

	/* Save class' fields */
	timer._setup.TIM_Channel_x = channel;
	timer._setup.TIM_IT_CCx = tim_it_cc;
	timer._setup.TIM_FLAG_CCxOF = tim_flag_cc_of;
	timer._setup.NVIC_IRQ_channel = nvic.NVIC_IRQChannel;

	/* Save base */
	timer.base = *tim_basic_ptr;

	/* Set initial state */
	timer._state.transition_counter_val = 0;
	timer._state.transition_detected = 0;

	/* Set function pointers */
	timer.Start = SOOL_TimerInputCapture_Start;
	timer.Stop = SOOL_TimerInputCapture_Stop;
	// TODO
	//timer.ReinitIC
	//timer.DisableIC
	timer.EnableNVIC = SOOL_TimerInputCapture_EnableNVIC;
	timer.DisableNVIC = SOOL_TimerInputCapture_DisableNVIC;
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

static void SOOL_TimerInputCapture_EnableNVIC(volatile SOOL_TimerInputCapture *tim_ic_ptr) {
	SOOL_Periph_NVIC_Enable(tim_ic_ptr->_setup.NVIC_IRQ_channel);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void SOOL_TimerInputCapture_DisableNVIC(volatile SOOL_TimerInputCapture *tim_ic_ptr) {
	SOOL_Periph_NVIC_Disable(tim_ic_ptr->_setup.NVIC_IRQ_channel);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint16_t SOOL_TimerInputCapture_GetSavedCounterVal(const volatile SOOL_TimerInputCapture *tim_ic_ptr) {
	return (tim_ic_ptr->_state.transition_counter_val);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t SOOL_TimerInputCapture_InterruptHandler(volatile SOOL_TimerInputCapture *tim_ic_ptr) {

	/* Check if update interrupt flag of the timer is set:
	 * 		o check IT flag
	 * 		o check whether OC channel is enabled
	 */
	if (TIM_GetITStatus(tim_ic_ptr->base._setup.TIMx, tim_ic_ptr->_setup.TIM_IT_CCx) == RESET) {
		// nothing to do (different IRQn or another flag)
		return (0);
	}

	/* If Flag is set check whether InputCapture timer's channel is enabled */
	if ( !SOOL_Periph_TIMCompare_IsCaptureCompareChannelEnabled(tim_ic_ptr->base._setup.TIMx, tim_ic_ptr->_setup.TIM_Channel_x) ) {
		return (0);
	}

	/* RM, p.384
	 * In order to handle the overcapture, it is recommended to read the data before the
	 * overcapture flag. This is to avoid missing an overcapture which could happen after reading
	 * the flag and before reading the data.
	 */

	/* Update Timer's state */
	/*If channel CCy is configured as *input*:
	CCRy is the counter value transferred by the last input capture y event (ICy). The
	TIMx_CCRy register is read-only and cannot be programmed. */
	tim_ic_ptr->_state.transition_counter_val = SOOL_Periph_TIMCompare_GetCCR(tim_ic_ptr->base._setup.TIMx, tim_ic_ptr->_setup.TIM_Channel_x);
	tim_ic_ptr->_state.transition_detected = 1;

	/* Clear the overcapture flag */
	tim_ic_ptr->base._setup.TIMx->SR = (uint16_t)~tim_ic_ptr->_setup.TIM_FLAG_CCxOF;

	/* Clear IT pending bit */
	tim_ic_ptr->base._setup.TIMx->SR = (uint16_t)~tim_ic_ptr->_setup.TIM_IT_CCx; // TIM_ClearITPendingBit(timer->_TIMx, TIM_IT_Update);
	return (1);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
