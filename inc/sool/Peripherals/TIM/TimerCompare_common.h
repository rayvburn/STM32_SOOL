/*
 * TimerCompare_common.h
 *
 *  Created on: 29.06.2019
 *      Author: user
 */

#ifndef INC_SOOL_PERIPHERALS_TIM_TIMERCOMPARE_COMMON_H_
#define INC_SOOL_PERIPHERALS_TIM_TIMERCOMPARE_COMMON_H_

#include "stm32f10x_tim.h"

// DEPRECATED
//typedef enum {
//	SOOL_PERIPH_TIMCOMPARE_CCMR_CLEAR_ENABLE = 0,
//	SOOL_PERIPH_TIMCOMPARE_CCMR_MODE,
//	SOOL_PERIPH_TIMCOMPARE_CCMR_PRELOAD_ENABLE,
//	SOOL_PERIPH_TIMCOMPARE_CCMR_FAST_ENABLE,
//	SOOL_PERIPH_TIMCOMPARE_CCMR_SELECTION,
//} SOOL_Periph_TIMCompare_BitCCMR;

extern uint16_t SOOL_Periph_TIMCompare_IsCaptureCompareChannelEnabled(TIM_TypeDef* TIMx, uint16_t channel);
extern uint16_t SOOL_Periph_TIMCompare_GetCCR(TIM_TypeDef* TIMx, uint16_t channel);
extern void SOOL_Periph_TIMCompare_SetCCR(TIM_TypeDef* TIMx, uint16_t channel, uint16_t value);

// DEPRECATED
//extern void SOOL_Periph_TIMCompare_SetCCMR(TIM_TypeDef* TIMx, uint16_t channel, uint16_t mask);

extern void SOOL_Periph_TIMCompare_EnableChannel(TIM_TypeDef* TIMx, uint16_t channel);
extern void SOOL_Periph_TIMCompare_DisableChannel(TIM_TypeDef* TIMx, uint16_t channel);

#endif /* INC_SOOL_PERIPHERALS_TIM_TIMERCOMPARE_COMMON_H_ */
