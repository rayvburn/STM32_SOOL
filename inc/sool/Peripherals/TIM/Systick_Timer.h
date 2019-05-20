/*
 * Systick_Timer.h
 *
 *  Created on: 08.10.2018
 *      Author: user
 */

#ifndef SYSTICK_TIMER_H_
#define SYSTICK_TIMER_H_

#include "misc.h"

extern void 	SOOL_Periph_TIM_SysTick_DefaultConfig();
extern uint32_t SOOL_Periph_TIM_SysTick_GetSec();
extern uint32_t	SOOL_Periph_TIM_SysTick_GetTenthsOfSec();
extern uint32_t	SOOL_Periph_TIM_SysTick_GetHundredthsOfSec();

/* Core Interrupt Service Routine for SysTick */
extern void 	SysTick_Handler();

#endif /* SYSTICK_TIMER_H_ */
