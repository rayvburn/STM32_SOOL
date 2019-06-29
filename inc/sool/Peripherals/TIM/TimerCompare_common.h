/*
 * TimerCompare_common.h
 *
 *  Created on: 29.06.2019
 *      Author: user
 */

#ifndef INC_SOOL_PERIPHERALS_TIM_TIMERCOMPARE_COMMON_H_
#define INC_SOOL_PERIPHERALS_TIM_TIMERCOMPARE_COMMON_H_

#include "stm32f10x_tim.h"

extern uint8_t SOOL_Periph_TIM_IsCaptureCompareChannelEnabled(TIM_TypeDef* TIMx, uint16_t channel);
extern uint16_t SOOL_Periph_TIM_GetCCR(TIM_TypeDef* TIMx, uint16_t channel);
extern void SOOL_Periph_TIM_SetCCR(TIM_TypeDef* TIMx, uint16_t channel, uint16_t value);

#endif /* INC_SOOL_PERIPHERALS_TIM_TIMERCOMPARE_COMMON_H_ */
