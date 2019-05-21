/*
 * TimerBasic.c
 *
 *  Created on: 20.05.2019
 *      Author: user
 */

#include "sool/Peripherals/TIM/TimerBasic.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void TimerBasic_Start(volatile SOOL_TimerBasic *timer);
static void TimerBasic_Stop(volatile SOOL_TimerBasic *timer);
static uint8_t TimerBasic_InterruptHandler(volatile SOOL_TimerBasic *timer);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/**
 * @param TIMx - for STM32F103C8T6 it may be TIM1, TIM2, TIM3 or TIM4
 * @param prescaler - clock divider (decremented value is loaded into TimeBaseInit structure)
 * @param period - period (decremented value is loaded into TimeBaseInit structure)
 * @return
 */
volatile SOOL_TimerBasic SOOL_Periph_TIM_TimerBasic_Init(TIM_TypeDef* TIMx, uint16_t prescaler, uint16_t period) {

	/* Object to be returned from the initializer */
	volatile SOOL_TimerBasic timer;

	/* Start proper clock */
	SOOL_Periph_TIM_EnableAPBClock(TIMx);

	/* Configure time base */
	TIM_TimeBaseInitTypeDef tim;
	TIM_TimeBaseStructInit(&tim);
	tim.TIM_CounterMode = TIM_CounterMode_Up;
	tim.TIM_Prescaler = prescaler - 1;
	tim.TIM_Period = period - 1;
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

	/* Save class' fields */
	timer.Start = TimerBasic_Start;
	timer.Stop = TimerBasic_Stop;
	timer._TIMx = TIMx;
	timer._InterruptHandler = TimerBasic_InterruptHandler;

	return (timer);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void TimerBasic_Start(volatile SOOL_TimerBasic *timer) {
	/* Enable the TIM Counter */
	timer->_TIMx->CR1 |= TIM_CR1_CEN;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void TimerBasic_Stop(volatile SOOL_TimerBasic *timer) {
	/* Disable the TIM Counter */
	timer->_TIMx->CR1 &= (uint16_t)(~((uint16_t)TIM_CR1_CEN));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t TimerBasic_InterruptHandler(volatile SOOL_TimerBasic *timer) {

	/* Check if update interrupt flag of the timer is set */
	if (TIM_GetITStatus(timer->_TIMx, TIM_IT_Update) == RESET) {
		// not this timer overflowed (different IRQn)
		return (0);
	}

	/* Clear IT pending bit */
	timer->_TIMx->SR = (uint16_t)~TIM_IT_Update; // TIM_ClearITPendingBit(timer->_TIMx, TIM_IT_Update);
	return (1);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
