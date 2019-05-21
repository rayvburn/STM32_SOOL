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

/** Provides basic periodical events handling feature */
struct _SOOL_TimerBasicStruct {
	TIM_TypeDef* 	_TIMx;
	void (*Start)(volatile SOOL_TimerBasic*);
	void (*Stop)(volatile SOOL_TimerBasic*);
	uint8_t (*_InterruptHandler)(volatile SOOL_TimerBasic*);
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

extern volatile SOOL_TimerBasic SOOL_Periph_TIM_TimerBasic_Init(TIM_TypeDef* TIMx, uint16_t prescaler, uint16_t period);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#endif /* INC_SOOL_PERIPHERALS_TIM_TIMERBASIC_H_ */
