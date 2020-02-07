/*
 * TimerEncoder.c
 *
 *  Created on: 30.01.2020
 *      Author: user
 */

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#include <sool/Peripherals/TIM/TimerEncoder.h>

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint16_t TimerEncoder_GetCount(volatile SOOL_TimerEncoder* tim_ptr);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

SOOL_TimerEncoder SOOL_Periph_TIM_TimerEncoder_Init(TIM_TypeDef* TIMx, uint16_t TIM_EncoderMode,
        uint16_t TIM_IC1Polarity, uint16_t TIM_IC2Polarity, FunctionalState enable_int_update) {

	/* New instance */
	SOOL_TimerEncoder encoder_interface;

	/* Configure timer (TimerBasic) */
	// `prescaler` parameter does not matter here, `period` is maximum possible
	// to allow long counting
	encoder_interface.base = SOOL_Periph_TIM_TimerBasic_Init(TIMx, 2, 0xFFFF, enable_int_update);

	/* Configure Input capture mode */
	// FIXME: it might be that the `TIM_EncoderInterfaceConfig` overwrites settings of TIM_IC - debug this
	TIM_ICInitTypeDef tim_ic;
	TIM_ICStructInit(&tim_ic); // default values

	// TIM_Channel 1
	tim_ic.TIM_Channel = TIM_Channel_1;
	tim_ic.TIM_ICFilter = 0xA;
	tim_ic.TIM_ICPolarity = TIM_IC1Polarity;
	tim_ic.TIM_ICPrescaler = 0;
	tim_ic.TIM_ICSelection = TIM_ICSelection_DirectTI; // 'remapping' possible
	TIM_ICInit(TIMx, &tim_ic);

	// TIM_Channel 2
	tim_ic.TIM_Channel = TIM_Channel_2;
	tim_ic.TIM_ICPolarity = TIM_IC2Polarity;
	TIM_ICInit(TIMx, &tim_ic);

	/* Configure encoder interface (`std_periph` function) */
	TIM_EncoderInterfaceConfig(TIMx, TIM_EncoderMode, TIM_IC1Polarity, TIM_IC2Polarity);

	/* Save structure fields */
	encoder_interface.GetCount = TimerEncoder_GetCount;

	/* Return prepared structure (a.k.a. `class instance`) */
	return (encoder_interface);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void SOOL_Periph_TIM_TimerEncoder_Startup(volatile SOOL_TimerEncoder* encoder_ptr) {

	// only the IT_Update interrupt could be enabled
	if ( encoder_ptr->base._setup.TIMx->DIER & (uint16_t)TIM_IT_Update ) {
		encoder_ptr->base.EnableNVIC(&encoder_ptr->base);
	}

	encoder_ptr->base.Start(&encoder_ptr->base);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint16_t TimerEncoder_GetCount(volatile SOOL_TimerEncoder* tim_ptr) {
	return (tim_ptr->base._setup.TIMx->CNT);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
