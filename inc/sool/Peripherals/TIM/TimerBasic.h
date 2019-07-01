/*
 * TimerBasic.h
 *
 *  Created on: 20.05.2019
 *      Author: user
 */

#ifndef INC_SOOL_PERIPHERALS_TIM_TIMERBASIC_H_
#define INC_SOOL_PERIPHERALS_TIM_TIMERBASIC_H_

#include "Timer_common.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/* Forward declaration */
struct _SOOL_TimerBasicStruct;
typedef struct _SOOL_TimerBasicStruct SOOL_TimerBasic;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

struct _SOOL_TimerBasicSetupStruct {
	TIM_TypeDef* 	TIMx;
	uint8_t 		NVIC_IRQ_channel;
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/** Provides basic periodical events handling feature */
struct _SOOL_TimerBasicStruct {

	struct _SOOL_TimerBasicSetupStruct _setup;
	void (*Start)(volatile SOOL_TimerBasic*);
	void (*Stop)(volatile SOOL_TimerBasic*);

	/**
	 * EnableNVIC can be called after timer's interrupt handler was placed in IRQHandler
	 * function and when there is a certainty that all objects driven by timer interrupts
	 * were already put in proper IRQHandlers too
	 * @param Pointer to SOOL_TimerBasic instance
	 */
	void (*EnableNVIC)(volatile SOOL_TimerBasic*);
	void (*DisableNVIC)(volatile SOOL_TimerBasic*);
	uint8_t (*_InterruptHandler)(volatile SOOL_TimerBasic*);

};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/**
 * A generic timer which supports only update interrupts; initializes time base,
 * does NOT enable the TIMx peripheral
 * @param TIMx - for STM32F103C8T6 it may be TIM1, TIM2, TIM3 or TIM4
 * @param prescaler - clock divider (decremented value is loaded into TimeBaseInit structure)
 * @param period - period (decremented value is loaded into TimeBaseInit structure)
 * @param enable_int_update - specifies whether to enable TIM_IT_Update interrupt for TIMx
 * @return SOOL_TimerBasic instance
 */
extern volatile SOOL_TimerBasic SOOL_Periph_TIM_TimerBasic_Init(TIM_TypeDef* TIMx, uint16_t prescaler,
		uint16_t period, FunctionalState enable_int_update);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#endif /* INC_SOOL_PERIPHERALS_TIM_TIMERBASIC_H_ */
