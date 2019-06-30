/*
 * TimerOutputCompare.c
 *
 *  Created on: 29.06.2019
 *      Author: user
 */

#include "sool/Peripherals/TIM/TimerOutputCompare.h"
#include "sool/Peripherals/TIM/TimerCompare_common.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// redefinition in case of some further changes, if some mods will be performed
// then API will not change
static void SOOL_TimerOC_Start(volatile SOOL_TimerOutputCompare *tim_oc_ptr);
static void SOOL_TimerOC_Stop(volatile SOOL_TimerOutputCompare *tim_oc_ptr);

static void SOOL_TimerOC_SetPulse(volatile SOOL_TimerOutputCompare *tim_oc_ptr, uint16_t pulse); // for PWM pulse-width adjustments
static void SOOL_TimerOC_ReinitOC(volatile SOOL_TimerOutputCompare *tim_oc_ptr);
static void SOOL_TimerOC_DisableOC(volatile SOOL_TimerOutputCompare *tim_oc_ptr);
static uint8_t SOOL_TimerOC_InterruptHandler(volatile SOOL_TimerOutputCompare *tim_oc_ptr);

// helper
static volatile SOOL_TimerOutputCompare SOOL_TimerOC_InitializeClass(TIM_TypeDef* TIMx,
		uint16_t prescaler, uint16_t period, uint16_t channel,
		uint16_t oc_mode, uint16_t pulse,
		uint16_t idle_state, uint16_t polarity, uint16_t output_state,
		uint16_t idle_state_n, uint16_t polarity_n, uint16_t output_state_n);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/**
 *
 * @param TIMx: TIMx where x can be 1 to 17 to select the TIM peripheral.
 * @param prescaler: (-1) Specifies the prescaler value used to divide the TIM clock. This parameter can be a number between 0x0000 and 0xFFFF
 * @param period: (-1) Specifies the period value to be loaded into the active Auto-Reload Register at the next update event.
          @note This parameter must be a number between 0x0000 and 0xFFFF.
 * @param channel: Specifies the TIM channel.
          @note This parameter can be a value of @ref TIM_Channel
 * @param oc_mode: Specifies the TIM mode. This parameter can be a value of @ref TIM_Output_Compare_and_PWM_modes
 * @param pulse: Specifies the pulse value to be loaded into the Capture Compare Register. This parameter can be a number between 0x0000 and 0xFFFF
 * @param idle_state: Specifies the TIM Output Compare pin state during Idle state. This parameter can be a value of @ref TIM_Output_Compare_Idle_State
		  @note This parameter is valid only for TIM1 and TIM8.
 * @param polarity: Specifies the output polarity. This parameter can be a value of @ref TIM_Output_Compare_Polarity
 * @param output_state: Specifies the TIM Output Compare state. This parameter can be a value of @ref TIM_Output_Compare_state
 * @param idle_state_n: Specifies the TIM Output Compare pin state during Idle state. This parameter can be a value of @ref TIM_Output_Compare_N_Idle_State
		  @note This parameter is valid only for TIM1 and TIM8.
 * @param polarity_n: Specifies the complementary output polarity. This parameter can be a value of @ref TIM_Output_Compare_N_Polarity
          @note This parameter is valid only for TIM1 and TIM8.
 * @param output_state_n: Specifies the TIM complementary Output Compare state. This parameter can be a value of @ref TIM_Output_Compare_N_state
   	   	  @note This parameter is valid only for TIM1 and TIM8.
 * @return
 */
volatile SOOL_TimerOutputCompare SOOL_Periph_TIM_TimerOutputCompare_InitFull(TIM_TypeDef* TIMx,
		uint16_t prescaler, uint16_t period, uint16_t channel,
		uint16_t oc_mode, uint16_t pulse,
		uint16_t idle_state, uint16_t polarity, uint16_t output_state,
		uint16_t idle_state_n, uint16_t polarity_n, uint16_t output_state_n) {

	volatile SOOL_TimerOutputCompare timer = SOOL_TimerOC_InitializeClass(TIMx, prescaler, period, channel,
			oc_mode, pulse, idle_state, polarity, output_state, idle_state_n, polarity_n, output_state_n);
	return (timer);

}

/**
 *
 * @param TIMx: TIMx where x can be 1 to 17 to select the TIM peripheral.
 * @param prescaler: (-1) Specifies the prescaler value used to divide the TIM clock. This parameter can be a number between 0x0000 and 0xFFFF
 * @param period: (-1) Specifies the period value to be loaded into the active Auto-Reload Register at the next update event.
          @note This parameter must be a number between 0x0000 and 0xFFFF.
 * @param channel: Specifies the TIM channel.
          @note This parameter can be a value of @ref TIM_Channel
 * @param oc_mode: Specifies the TIM mode. This parameter can be a value of @ref TIM_Output_Compare_and_PWM_modes
 * @param pulse: Specifies the pulse value to be loaded into the Capture Compare Register. This parameter can be a number between 0x0000 and 0xFFFF
 * @param idle_state: Specifies the TIM Output Compare pin state during Idle state. This parameter can be a value of @ref TIM_Output_Compare_Idle_State
		  @note This parameter is valid only for TIM1 and TIM8.
 * @param polarity: Specifies the output polarity. This parameter can be a value of @ref TIM_Output_Compare_Polarity
 * @param output_state: Specifies the TIM Output Compare state. This parameter can be a value of @ref TIM_Output_Compare_state
 * @return
 */
volatile SOOL_TimerOutputCompare SOOL_Periph_TIM_TimerOutputCompare_Init(TIM_TypeDef* TIMx,
		uint16_t prescaler, uint16_t period, uint16_t channel,
		uint16_t oc_mode, uint16_t pulse,
		uint16_t idle_state, uint16_t polarity, uint16_t output_state) {

	volatile SOOL_TimerOutputCompare timer = SOOL_TimerOC_InitializeClass(TIMx, prescaler, period, channel,
			oc_mode, pulse, idle_state, polarity, output_state, TIM_OCNIdleState_Reset,
			TIM_OCPolarity_High, TIM_OutputNState_Disable);

	return (timer);

}

/**
 *
 * @param TIMx: TIMx where x can be 1 to 17 to select the TIM peripheral.
 * @param prescaler: (-1) Specifies the prescaler value used to divide the TIM clock. This parameter can be a number between 0x0000 and 0xFFFF
 * @param period: (-1) Specifies the period value to be loaded into the active Auto-Reload Register at the next update event.
          @note This parameter must be a number between 0x0000 and 0xFFFF.
 * @param channel: Specifies the TIM channel.
          @note This parameter can be a value of @ref TIM_Channel
 * @param oc_mode: Specifies the TIM mode. This parameter can be a value of @ref TIM_Output_Compare_and_PWM_modes
 * @param pulse: Specifies the pulse value to be loaded into the Capture Compare Register. This parameter can be a number between 0x0000 and 0xFFFF
 * @param idle_state_n: Specifies the TIM Output Compare pin state during Idle state. This parameter can be a value of @ref TIM_Output_Compare_N_Idle_State
		  @note This parameter is valid only for TIM1 and TIM8.
 * @param polarity_n: Specifies the complementary output polarity. This parameter can be a value of @ref TIM_Output_Compare_N_Polarity
          @note This parameter is valid only for TIM1 and TIM8.
 * @param output_state_n: Specifies the TIM complementary Output Compare state. This parameter can be a value of @ref TIM_Output_Compare_N_state
   	   	  @note This parameter is valid only for TIM1 and TIM8.
 * @return
 */
volatile SOOL_TimerOutputCompare SOOL_Periph_TIM_TimerOutputCompare_InitComplementary(TIM_TypeDef* TIMx,
		uint16_t prescaler, uint16_t period, uint16_t channel,
		uint16_t oc_mode, uint16_t pulse,
		uint16_t idle_state_n, uint16_t polarity_n, uint16_t output_state_n) {

	volatile SOOL_TimerOutputCompare timer = SOOL_TimerOC_InitializeClass(TIMx, prescaler, period, channel,
			oc_mode, pulse, TIM_OCIdleState_Reset, TIM_OCPolarity_High, TIM_OutputState_Disable,
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
	SOOL_Periph_TIM_SetCCR(tim_oc_ptr->base._setup.TIMx, tim_oc_ptr->_setup.TIM_Channel_x, pulse);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void SOOL_TimerOC_ReinitOC(volatile SOOL_TimerOutputCompare *tim_oc_ptr) {

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
	TIM_ITConfig(tim_oc_ptr->base._setup.TIMx, tim_oc_ptr->_setup.TIM_IT_CCx, ENABLE);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void SOOL_TimerOC_DisableOC(volatile SOOL_TimerOutputCompare *tim_oc_ptr) {

	// disable interrupt
	TIM_ITConfig(tim_oc_ptr->base._setup.TIMx, tim_oc_ptr->_setup.TIM_IT_CCx, DISABLE);

	// some bits are writable only when channel is OFF
	/* Disable the Channel x: Reset the CCxE Bit */
	switch (tim_oc_ptr->_setup.TIM_Channel_x) {

	case(TIM_Channel_1):
		tim_oc_ptr->base._setup.TIMx->CCER &= (uint16_t)(~((uint16_t)TIM_CCER_CC1E));
		break;

	case(TIM_Channel_2):
		tim_oc_ptr->base._setup.TIMx->CCER &= (uint16_t)(~((uint16_t)TIM_CCER_CC2E));
		break;

	case(TIM_Channel_3):
		tim_oc_ptr->base._setup.TIMx->CCER &= (uint16_t)(~((uint16_t)TIM_CCER_CC3E));
		break;

	case(TIM_Channel_4):
		tim_oc_ptr->base._setup.TIMx->CCER &= (uint16_t)(~((uint16_t)TIM_CCER_CC4E));
		break;

	}

	// set reset values to TIMx_CCMRx and TIMx_CCER registers
	tim_oc_ptr->base._setup.TIMx->CCMR1 = 0x00;
	tim_oc_ptr->base._setup.TIMx->CCMR2 = 0x00;
	tim_oc_ptr->base._setup.TIMx->CCER  = 0x00;

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t SOOL_TimerOC_InterruptHandler(volatile SOOL_TimerOutputCompare *tim_oc_ptr) {

	/* Check if update interrupt flag of the timer is set:
	 * 		o check whether OC channel is enabled
	 * 		o check IT flag
	 */
	if ( !SOOL_Periph_TIM_IsCaptureCompareChannelEnabled(tim_oc_ptr->base._setup.TIMx, tim_oc_ptr->_setup.TIM_Channel_x)
		 || TIM_GetITStatus(tim_oc_ptr->base._setup.TIMx, tim_oc_ptr->_setup.TIM_IT_CCx) == RESET)
	{
		// nothing to do (different IRQn or another flag)
		return (0);
	}

	/* Clear IT pending bit */
	tim_oc_ptr->base._setup.TIMx->SR = (uint16_t)~tim_oc_ptr->_setup.TIM_IT_CCx; // TIM_ClearITPendingBit(timer->_TIMx, TIM_IT_Update);
	return (1);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static volatile SOOL_TimerOutputCompare SOOL_TimerOC_InitializeClass(TIM_TypeDef* TIMx,	// timer
		uint16_t prescaler, uint16_t period, uint16_t channel,					// time-base-related
		uint16_t oc_mode, uint16_t pulse,										// OC 'common'
		uint16_t idle_state, uint16_t polarity, uint16_t output_state,			// OC 'positive'
		uint16_t idle_state_n, uint16_t polarity_n, uint16_t output_state_n)	// OC 'negative'
{

	/* TimerBasic `constructor` - time base must be configured and basic
	 * timer activity must be run to allow output compare mode work properly */
	volatile SOOL_TimerBasic tim_basic = SOOL_Periph_TIM_TimerBasic_Init(TIMx, prescaler, period, SOOL_PERIPH_TIM_IRQ_CC);

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

	switch (channel) {

	case(TIM_Channel_1):
		tim_it_cc = TIM_IT_CC1;
		TIM_OC1Init(TIMx, &tim_oc);
		break;

	case(TIM_Channel_2):
		tim_it_cc = TIM_IT_CC2;
		TIM_OC2Init(TIMx, &tim_oc);
		break;

	case(TIM_Channel_3):
		tim_it_cc = TIM_IT_CC3;
		TIM_OC3Init(TIMx, &tim_oc);
		break;

	case(TIM_Channel_4):
		tim_it_cc = TIM_IT_CC4;
		TIM_OC4Init(TIMx, &tim_oc);
		break;

	}

	/* Enable interrupts */
	TIM_ITConfig(TIMx, tim_it_cc, ENABLE);

	/* Save class' fields */
	timer._setup.oc_config = tim_oc;
	timer._setup.TIM_Channel_x = channel;
	timer._setup.TIM_IT_CCx = tim_it_cc;

	/* Save base */
	timer.base = tim_basic;

	/* Set initial state */
	//timer.

	/* Set function pointers */
	timer.DisableOC = SOOL_TimerOC_DisableOC;
	timer.ReinitOC = SOOL_TimerOC_ReinitOC;
	timer.SetPulse = SOOL_TimerOC_SetPulse;
	timer.Start = SOOL_TimerOC_Start;
	timer.Stop = SOOL_TimerOC_Stop;
	timer._InterruptHandler = SOOL_TimerOC_InterruptHandler;

	return (timer);

}
