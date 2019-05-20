/*
 * TimerBasic.h
 *
 *  Created on: 20.05.2019
 *      Author: user
 */

#ifndef INC_SOOL_PERIPHERALS_TIM_TIMERBASIC_H_
#define INC_SOOL_PERIPHERALS_TIM_TIMERBASIC_H_

#include "Timer_common.h"

/** Provides basic periodical events handling feature */
typedef struct {
	TIM_TypeDef* TIMx;
	void (*Start)(volatile *SOOL_TimerBasic);
	void (*Stop)(volatile *SOOL_TimerBasic);
	void (*InterruptCustomFcn)(volatile *SOOL_TimerBasic); /* invoked from inside of _InterruptHandler */
	void (*_InterruptHandler)(volatile *SOOL_TimerBasic);
} SOOL_TimerBasic;

extern volatile SOOL_TimerBasic SOOL_Periph_TIM_TimerBasic_Init(TIM_TypeDef* TIMx, uint16_t prescaler, uint16_t period);

#endif /* INC_SOOL_PERIPHERALS_TIM_TIMERBASIC_H_ */
