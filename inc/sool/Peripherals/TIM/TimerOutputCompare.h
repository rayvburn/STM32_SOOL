/*
 * TimerOutputCompare.h
 *
 *  Created on: 29.06.2019
 *      Author: user
 */

#ifndef INC_SOOL_PERIPHERALS_TIM_TIMEROUTPUTCOMPARE_H_
#define INC_SOOL_PERIPHERALS_TIM_TIMEROUTPUTCOMPARE_H_

#include "sool/Peripherals/TIM/TimerBasic.h"

/* Forward declaration */
struct _SOOL_TimerOutputCompareStruct;
typedef struct _SOOL_TimerOutputCompareStruct SOOL_TimerOutputCompare;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

struct _SOOL_TimerOutputCompareStateStruct {
	//uint8_t  match_detected;	// DEPRECATED?
};

struct _SOOL_TimerOutputCompareSetupStruct {
	TIM_OCInitTypeDef oc_config;// initializing structure
//	uint16_t TIM_CCMRx_OCxFE;	// Fast Enable (in fact TIM_CCMRy_OCxFE, where y = 1,2, x = 1,2,3,4)
//	uint8_t  enable_int;		// whether to enable interrupts
	uint16_t TIM_IT_CCx; 		// channel compare ID
	uint16_t TIM_Channel_x;		// acquisition of the CCRx register content
	uint8_t NVIC_IRQ_channel;
};

/**
 * Composition of SOOL_TimerBasic and SOOL_TimerOutputCompare class' contents
 * Unluckily there is no way to access base `class` members directly.
 * IMPORTANT: in case of derived classes which are interrupt-related
 * one must remember to call base's interrupt handlers too
 */
struct _SOOL_TimerOutputCompareStruct {

	// base section --------------------------------------------------------
	SOOL_TimerBasic base;

	// derived section -----------------------------------------------------
	struct _SOOL_TimerOutputCompareStateStruct _state;
	struct _SOOL_TimerOutputCompareSetupStruct _setup;

	void (*Start)(volatile SOOL_TimerOutputCompare*);
	void (*Stop)(volatile SOOL_TimerOutputCompare*);

	// DEPRECATED due to interrupt driven architecture, there is no need to check flag,
	// it's better to run a certain function during ISR
	//uint8_t (*DidMatch)(const volatile SOOL_TimerOutputCompare*); // to check whether interrupt was generated

	void (*SetPulse)(volatile SOOL_TimerOutputCompare*, uint16_t); // for PWM pulse-width adjustments
	void (*ReinitOC)(volatile SOOL_TimerOutputCompare*);
	void (*DisableOC)(volatile SOOL_TimerOutputCompare*);

	/**
	 * EnableNVIC can be called after timer's interrupt handler was placed in IRQHandler
	 * function and when there is a certainty that all objects driven by timer interrupts
	 * were already put in proper IRQHandlers too
	 * @param Pointer to SOOL_TimerOutputCompare instance
	 */
	void (*EnableNVIC)(volatile SOOL_TimerOutputCompare*);
	void (*DisableNVIC)(volatile SOOL_TimerOutputCompare*);

	uint8_t (*_InterruptHandler)(volatile SOOL_TimerOutputCompare*);

};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

extern volatile SOOL_TimerOutputCompare SOOL_Periph_TIM_TimerOutputCompare_InitFull(volatile SOOL_TimerBasic *tim_basic_ptr,
		uint16_t channel, uint16_t oc_mode, uint16_t pulse,	FunctionalState enable_int_cc,// OC 'common'
		uint16_t idle_state, uint16_t polarity, uint16_t output_state,					// OC 'positive'
		uint16_t idle_state_n, uint16_t polarity_n, uint16_t output_state_n);

extern volatile SOOL_TimerOutputCompare SOOL_Periph_TIM_TimerOutputCompare_Init(volatile SOOL_TimerBasic *tim_basic_ptr,
		uint16_t channel, uint16_t oc_mode, uint16_t pulse, FunctionalState enable_int_cc,
		uint16_t idle_state, uint16_t polarity, uint16_t output_state);

extern volatile SOOL_TimerOutputCompare SOOL_Periph_TIM_TimerOutputCompare_InitComplementary(volatile SOOL_TimerBasic *tim_basic_ptr,
		uint16_t channel, uint16_t oc_mode, uint16_t pulse, FunctionalState enable_int_cc,
		uint16_t idle_state_n, uint16_t polarity_n, uint16_t output_state_n);

#endif /* INC_SOOL_PERIPHERALS_TIM_TIMEROUTPUTCOMPARE_H_ */
