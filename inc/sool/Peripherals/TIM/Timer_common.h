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

/**
 * To specify which IRQn type should be chosen from a certain timer's possibilities
 */
typedef enum {
	SOOL_PERIPH_TIM_IRQ_SHARED = 0,//!< SOOL_PERIPH_TIM_IRQ_SHARED - no effect, designed for timers which have a shared IRQHandler for all flags
	SOOL_PERIPH_TIM_IRQ_UP,        //!< SOOL_PERIPH_TIM_IRQ_UP - counter up
	SOOL_PERIPH_TIM_IRQ_CC,        //!< SOOL_PERIPH_TIM_IRQ_CC - capture/compare
	SOOL_PERIPH_TIM_IRQ_BRK,       //!< SOOL_PERIPH_TIM_IRQ_BRK
	SOOL_PERIPH_TIM_IRQ_TRG_COM    //!< SOOL_PERIPH_TIM_IRQ_TRG_COM
} SOOL_Periph_TIM_IRQnType;


extern void SOOL_Periph_TIM_EnableAPBClock(TIM_TypeDef* TIMx);
extern uint8_t SOOL_Periph_TIM_GetIRQnType(TIM_TypeDef* TIMx, SOOL_Periph_TIM_IRQnType irqn_type);


#endif /* INC_SOOL_PERIPHERALS_TIM_TIMER_COMMON_H_ */
