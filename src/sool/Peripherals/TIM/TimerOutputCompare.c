/*
 * TimerOutputCompare.c
 *
 *  Created on: 29.06.2019
 *      Author: user
 */

#include "sool/Peripherals/TIM/TimerOutputCompare.h"
#include "sool/Peripherals/TIM/TimerCompare_common.h"
#include "sool/Peripherals/NVIC/NVIC.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// redefinition in case of some further changes, if some mods will be performed
// then API will not change
static void SOOL_TimerOC_Start(volatile SOOL_TimerOutputCompare *tim_oc_ptr);
static void SOOL_TimerOC_Stop(volatile SOOL_TimerOutputCompare *tim_oc_ptr);

static void SOOL_TimerOC_SetPulse(volatile SOOL_TimerOutputCompare *tim_oc_ptr, uint16_t pulse); // for PWM pulse-width adjustments
static void SOOL_TimerOC_ReinitOC(volatile SOOL_TimerOutputCompare *tim_oc_ptr);
static void SOOL_TimerOC_DisableOC(volatile SOOL_TimerOutputCompare *tim_oc_ptr);

static void SOOL_TimerOC_EnableNVIC(volatile SOOL_TimerOutputCompare *tim_oc_ptr);
static void SOOL_TimerOC_DisableNVIC(volatile SOOL_TimerOutputCompare *tim_oc_ptr);

static uint8_t SOOL_TimerOC_InterruptHandler(volatile SOOL_TimerOutputCompare *tim_oc_ptr);

// helper
static volatile SOOL_TimerOutputCompare SOOL_TimerOC_InitializeClass(volatile SOOL_TimerBasic *tim_basic_ptr,	// time-base-related
		uint16_t channel, uint16_t oc_mode, uint16_t pulse,	FunctionalState enable_int, // OC 'common'
		uint16_t idle_state, uint16_t polarity, uint16_t output_state,					// OC 'positive'
		uint16_t idle_state_n, uint16_t polarity_n, uint16_t output_state_n);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/**
 *
 * @param TIMx: TIMx where x can be 1 to 17 to select the TIM peripheral.
 * @param prescaler: (-1) Specifies the prescaler value used to divide the TIM clock. This parameter can be a number between 0x0000 and 0xFFFF
 * @param period: (-1) Specifies the period value to be loaded into the active Auto-Reload Register at the next update event.
 *        @note This parameter must be a number between 0x0000 and 0xFFFF.
 * @param enable_int_update: if true TIM_IT_Update is enabled
 * @param channel: Specifies the TIM channel.
 *        @note This parameter can be a value of @ref TIM_Channel
 * @param oc_mode: Specifies the TIM mode. This parameter can be a value of @ref TIM_Output_Compare_and_PWM_modes
 *        @note Difference between PWM1 and PWM2 is active state polarity (PWM2 is complementary to PWM1)
 * @param pulse: Specifies the pulse value to be loaded into the Capture Compare Register. This parameter can be a number between 0x0000 and 0xFFFF
 * @param enable_int_cc: if true TIM_IT_CCx is enabled
 * @param idle_state: Specifies the TIM Output Compare pin state during Idle state. This parameter can be a value of @ref TIM_Output_Compare_Idle_State
 *	  	  @note This parameter is valid only for TIM1 and TIM8.
 * @param polarity: Specifies the output polarity. This parameter can be a value of @ref TIM_Output_Compare_Polarity
 * @param output_state: Specifies the TIM Output Compare state. This parameter can be a value of @ref TIM_Output_Compare_state
 * @param idle_state_n: Specifies the TIM Output Compare pin state during Idle state. This parameter can be a value of @ref TIM_Output_Compare_N_Idle_State
 *		  @note This parameter is valid only for TIM1 and TIM8.
 * @param polarity_n: Specifies the complementary output polarity. This parameter can be a value of @ref TIM_Output_Compare_N_Polarity
 *        @note This parameter is valid only for TIM1 and TIM8.
 * @param output_state_n: Specifies the TIM complementary Output Compare state. This parameter can be a value of @ref TIM_Output_Compare_N_state
 * 	   	  @note This parameter is valid only for TIM1 and TIM8.
 * @return SOOL_TimerOutputCompare instance
 */
volatile SOOL_TimerOutputCompare SOOL_Periph_TIM_TimerOutputCompare_InitFull(volatile SOOL_TimerBasic *tim_basic_ptr,
		uint16_t channel, uint16_t oc_mode, uint16_t pulse,	FunctionalState enable_int_cc,// OC 'common'
		uint16_t idle_state, uint16_t polarity, uint16_t output_state,					// OC 'positive'
		uint16_t idle_state_n, uint16_t polarity_n, uint16_t output_state_n) {

	volatile SOOL_TimerOutputCompare timer = SOOL_TimerOC_InitializeClass(tim_basic_ptr,
			channel, oc_mode, pulse, enable_int_cc,
			idle_state, polarity, output_state,
			idle_state_n, polarity_n, output_state_n);

	return (timer);

}

/**
 * For details see @ref SOOL_Periph_TIM_TimerOutputCompare_InitFull
 * @param tim_basic
 * @param oc_mode
 * @param pulse
 * @param enable_int_cc
 * @param idle_state
 * @param polarity
 * @param output_state
 * @return
 */
volatile SOOL_TimerOutputCompare SOOL_Periph_TIM_TimerOutputCompare_Init(volatile SOOL_TimerBasic *tim_basic_ptr,
		uint16_t channel, uint16_t oc_mode, uint16_t pulse, FunctionalState enable_int_cc,
		uint16_t idle_state, uint16_t polarity, uint16_t output_state) {

	volatile SOOL_TimerOutputCompare timer = SOOL_TimerOC_InitializeClass(tim_basic_ptr,
			channel, oc_mode, pulse, enable_int_cc,
			idle_state, polarity, output_state,
			TIM_OCNIdleState_Reset, TIM_OCPolarity_High, TIM_OutputNState_Disable);

	return (timer);

}

/**
 * For details see @ref SOOL_Periph_TIM_TimerOutputCompare_InitFull
 * @param tim_basic
 * @param oc_mode
 * @param pulse
 * @param enable_int_cc
 * @param idle_state_n
 * @param polarity_n
 * @param output_state_n
 * @return
 */
volatile SOOL_TimerOutputCompare SOOL_Periph_TIM_TimerOutputCompare_InitComplementary(volatile SOOL_TimerBasic *tim_basic_ptr,
		uint16_t channel, uint16_t oc_mode, uint16_t pulse, FunctionalState enable_int_cc,
		uint16_t idle_state_n, uint16_t polarity_n, uint16_t output_state_n) {

	volatile SOOL_TimerOutputCompare timer = SOOL_TimerOC_InitializeClass(tim_basic_ptr,
			channel, oc_mode, pulse, enable_int_cc,
			TIM_OCIdleState_Reset, TIM_OCPolarity_High, TIM_OutputState_Disable,
			idle_state_n, polarity_n, output_state_n);

	return (timer);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void SOOL_TimerOC_Start(volatile SOOL_TimerOutputCompare *tim_oc_ptr) {
	tim_oc_ptr->base.Start(&tim_oc_ptr->base);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void SOOL_TimerOC_Stop(volatile SOOL_TimerOutputCompare *tim_oc_ptr) {
	tim_oc_ptr->base.Stop(&tim_oc_ptr->base);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void SOOL_TimerOC_SetPulse(volatile SOOL_TimerOutputCompare *tim_oc_ptr, uint16_t pulse) {
	// for PWM pulse-width adjustments
	/* Set the Capture Compare Register value */
	SOOL_Periph_TIMCompare_SetCCR(tim_oc_ptr->base._setup.TIMx, tim_oc_ptr->_setup.TIM_Channel_x, pulse);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void SOOL_TimerOC_ReinitOC(volatile SOOL_TimerOutputCompare *tim_oc_ptr) {

	// OCx init enables channel
	switch (tim_oc_ptr->_setup.TIM_Channel_x) {

	case(TIM_Channel_1):
		TIM_OC1Init(tim_oc_ptr->base._setup.TIMx, &tim_oc_ptr->_setup.oc_config);
		break;

	case(TIM_Channel_2):
		TIM_OC2Init(tim_oc_ptr->base._setup.TIMx, &tim_oc_ptr->_setup.oc_config);
		break;

	case(TIM_Channel_3):
		TIM_OC3Init(tim_oc_ptr->base._setup.TIMx, &tim_oc_ptr->_setup.oc_config);
		break;

	case(TIM_Channel_4):
		TIM_OC4Init(tim_oc_ptr->base._setup.TIMx, &tim_oc_ptr->_setup.oc_config);
		break;

	}

	// enable interrupt
//	TIM_ITConfig(tim_oc_ptr->base._setup.TIMx, tim_oc_ptr->_setup.TIM_IT_CCx, ENABLE); // can cause conflicts with InputCompare when used simultaneously

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void SOOL_TimerOC_DisableOC(volatile SOOL_TimerOutputCompare *tim_oc_ptr) {

	// disable interrupt
//	TIM_ITConfig(tim_oc_ptr->base._setup.TIMx, tim_oc_ptr->_setup.TIM_IT_CCx, DISABLE); // can cause conflicts with InputCompare when used simultaneously

	// some bits are writable only when channel is OFF
	SOOL_Periph_TIMCompare_DisableChannel(tim_oc_ptr->base._setup.TIMx, tim_oc_ptr->_setup.TIM_Channel_x);

	// set reset values to TIMx_CCMRx and TIMx_CCER registers
	tim_oc_ptr->base._setup.TIMx->CCMR1 = 0x00;
	tim_oc_ptr->base._setup.TIMx->CCMR2 = 0x00;
	tim_oc_ptr->base._setup.TIMx->CCER  = 0x00;

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void SOOL_TimerOC_EnableNVIC(volatile SOOL_TimerOutputCompare *tim_oc_ptr) {
	SOOL_Periph_NVIC_Enable(tim_oc_ptr->_setup.NVIC_IRQ_channel);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void SOOL_TimerOC_DisableNVIC(volatile SOOL_TimerOutputCompare *tim_oc_ptr) {
	SOOL_Periph_NVIC_Disable(tim_oc_ptr->_setup.NVIC_IRQ_channel);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t SOOL_TimerOC_InterruptHandler(volatile SOOL_TimerOutputCompare *tim_oc_ptr) {

	/* Check if update interrupt flag of the timer is set:
	 * 		o check IT flag
	 * 		o check whether OC channel is enabled
	 */
	if ( TIM_GetITStatus(tim_oc_ptr->base._setup.TIMx, tim_oc_ptr->_setup.TIM_IT_CCx) == RESET ) {
		// nothing to do (different IRQn or another flag)
		return (0);
	}

	if ( !SOOL_Periph_TIMCompare_IsCaptureCompareChannelEnabled(tim_oc_ptr->base._setup.TIMx, tim_oc_ptr->_setup.TIM_Channel_x) ) {
		return (0);
	}

	/* Clear IT pending bit */
	tim_oc_ptr->base._setup.TIMx->SR = (uint16_t)~tim_oc_ptr->_setup.TIM_IT_CCx; // TIM_ClearITPendingBit(timer->_TIMx, TIM_IT_Update);
	return (1);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static volatile SOOL_TimerOutputCompare SOOL_TimerOC_InitializeClass(volatile SOOL_TimerBasic *tim_basic_ptr,	// time-base-related
		uint16_t channel, uint16_t oc_mode, uint16_t pulse,	FunctionalState enable_int, // OC 'common'
		uint16_t idle_state, uint16_t polarity, uint16_t output_state,					// OC 'positive'
		uint16_t idle_state_n, uint16_t polarity_n, uint16_t output_state_n)			// OC 'negative'
{

	/* Object to be returned from the initializer */
	volatile SOOL_TimerOutputCompare timer;

	/* Output Compare mode configuration */
	TIM_OCInitTypeDef tim_oc;
	TIM_OCStructInit(&tim_oc); // default values

	// common
	tim_oc.TIM_OCMode = oc_mode;
	tim_oc.TIM_Pulse = pulse;

	// P
	tim_oc.TIM_OCIdleState = idle_state;
	tim_oc.TIM_OCPolarity = polarity;
	tim_oc.TIM_OutputState = output_state;

	// N
	tim_oc.TIM_OCNIdleState = idle_state_n;
	tim_oc.TIM_OCNPolarity = polarity_n;
	tim_oc.TIM_OutputNState = output_state_n;

	/* Based on a given channel, initialize a proper TIM_OCx and CC interruts */
	uint16_t tim_it_cc = 0x00; 		// not valid value by default

	// output compare mode flags
//	uint16_t tim_ocxm, tim_ocxce, tim_ocxpe = 0x00;
//	uint16_t tim_ocxfe = 0x00;		// fast enable

	switch (channel) {

	case(TIM_Channel_1):
		tim_it_cc = TIM_IT_CC1;
//		tim_ocxfe = TIM_CCMR1_OC1FE;
		TIM_OC1Init(tim_basic_ptr->_setup.TIMx, &tim_oc);
		break;

	case(TIM_Channel_2):
		tim_it_cc = TIM_IT_CC2;
//		tim_ocxfe = TIM_CCMR1_OC2FE;
		TIM_OC2Init(tim_basic_ptr->_setup.TIMx, &tim_oc);
		break;

	case(TIM_Channel_3):
		tim_it_cc = TIM_IT_CC3;
//		tim_ocxfe = TIM_CCMR2_OC3FE;
		TIM_OC3Init(tim_basic_ptr->_setup.TIMx, &tim_oc);
		break;

	case(TIM_Channel_4):
		tim_it_cc = TIM_IT_CC4;
//		tim_ocxfe = TIM_CCMR2_OC4FE;
		TIM_OC4Init(tim_basic_ptr->_setup.TIMx, &tim_oc);
		break;

	}

	/* Configure NVIC if needed */
	NVIC_InitTypeDef nvic;
	nvic.NVIC_IRQChannel = SOOL_Periph_TIM_GetIRQnType(tim_basic_ptr->_setup.TIMx, SOOL_PERIPH_TIM_IRQ_CC); // for safety moved outside `if`

	if ( enable_int == ENABLE ) {

		nvic.NVIC_IRQChannelPreemptionPriority = 0;
		nvic.NVIC_IRQChannelSubPriority = 0;

		/* Some useful notes in Base class */
		nvic.NVIC_IRQChannelCmd = DISABLE;
		NVIC_Init(&nvic);

		/* IT_Update surely needs to be cleared when `NVIC_IRQChannelCmd == ENABLE` */
		//TIMx->SR = (uint16_t)~TIM_IT_Update;

	}

	/* Enable interrupts */
	TIM_ITConfig(tim_basic_ptr->_setup.TIMx, tim_it_cc, enable_int);

	/* Save class' fields */
	timer._setup.oc_config = tim_oc;
	timer._setup.TIM_Channel_x = channel;
	timer._setup.TIM_IT_CCx = tim_it_cc;
	timer._setup.NVIC_IRQ_channel = nvic.NVIC_IRQChannel;
//	timer._setup.TIM_CCMRx_OCxFE = tim_ocxfe;
//	timer._setup.enable_int = enable_int; // FIXME: this could cause conflicts with InputCompare

	/* Save base */
	timer.base = *tim_basic_ptr;

	/* Set initial state */
	//timer.

	/* Set function pointers */
	timer.DisableNVIC = SOOL_TimerOC_DisableNVIC;
	timer.EnableNVIC = SOOL_TimerOC_EnableNVIC;
	timer.DisableOC = SOOL_TimerOC_DisableOC;
	timer.ReinitOC = SOOL_TimerOC_ReinitOC;
	timer.SetPulse = SOOL_TimerOC_SetPulse;
	timer.Start = SOOL_TimerOC_Start;
	timer.Stop = SOOL_TimerOC_Stop;
	timer._InterruptHandler = SOOL_TimerOC_InterruptHandler;

	return (timer);

}
