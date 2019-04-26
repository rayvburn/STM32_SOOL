/*
 * Systick_Timer.h
 *
 *  Created on: 08.10.2018
 *      Author: user
 */

#ifndef SYSTICK_TIMER_H_
#define SYSTICK_TIMER_H_

#include "misc.h"

extern void 	SysTick_Configuration();
extern void 	SysTick_Handler();
extern uint32_t SysTick_GetSeconds();
extern uint32_t	SysTick_GetTenthsOfSec();
extern uint32_t	SysTick_GetHundredthsOfSec();

#endif /* SYSTICK_TIMER_H_ */
