/*
 * Timer_common.h
 *
 *  Created on: 20.05.2019
 *      Author: user
 */

#ifndef INC_SOOL_PERIPHERALS_TIM_TIMER_COMMON_H_
#define INC_SOOL_PERIPHERALS_TIM_TIMER_COMMON_H_

#include "stm32f10x.h"
#include "stm32f10x_tim.h"

extern void SOOL_Periph_TIM_EnableAPBClock(TIM_TypeDef* TIMx);
extern uint8_t SOOL_Periph_TIM_GetIRQnType(TIM_TypeDef* TIMx);

#endif /* INC_SOOL_PERIPHERALS_TIM_TIMER_COMMON_H_ */
