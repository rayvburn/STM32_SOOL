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

extern volatile SOOL_TimerBasic SOOL_Periph_TIM_TimerBasic_Init(TIM_TypeDef* TIMx, uint16_t prescaler, uint16_t period, SOOL_Periph_TIM_IRQnType irqn_type);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#endif /* INC_SOOL_PERIPHERALS_TIM_TIMERBASIC_H_ */
