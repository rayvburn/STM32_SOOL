/*
 * TimerBasic.c
 *
 *  Created on: 20.05.2019
 *      Author: user
 */

#include "sool/Peripherals/TIM/TimerBasic.h"

volatile SOOL_TimerBasic SOOL_Periph_TIM_TimerBasic_Init(TIM_TypeDef* TIMx, uint16_t prescaler, uint16_t period) {

	/* Object to be returned from the initializer */
	volatile SOOL_TimerBasic timer;

	/* Start proper clock */
	SOOL_Periph_TIM_EnableAPBClock(TIMx);

	/* Configure time base */
	TIM_TimeBaseInitTypeDef tim;
	TIM_TimeBaseStructInit(&tim);
	tim.TIM_CounterMode = TIM_CounterMode_Up;
	tim.TIM_Prescaler = prescaler;
	tim.TIM_Period = period;
	tim.TIM_ClockDivision = 0;
	tim.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIMx, &tim);

	/* Configure interrupts */
	TIM_ITConfig(TIMx, TIM_IT_Update, ENABLE);
	TIM_Cmd(TIMx, DISABLE);

	NVIC_InitTypeDef nvic;
	nvic.NVIC_IRQChannel = SOOL_Periph_TIM_GetIRQnType(TIMx);
	nvic.NVIC_IRQChannelPreemptionPriority = 0;
	nvic.NVIC_IRQChannelSubPriority = 0;
	nvic.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvic);

	return (timer);

}


/* IRQHandler
 *
 *
void TIM1_UP_TIM16_IRQHandler() {

	if (TIM_GetITStatus(TIM16, TIM_IT_Update) == SET) {

			TIM_ClearITPendingBit(TIM16, TIM_IT_Update);

			// something
	}
}
 *
 *
 */
