/*
 * TimerBasic.c
 *
 *  Created on: 20.05.2019
 *      Author: user
 */

#include "sool/Peripherals/TIM/TimerBasic.h"
#include "sool/Peripherals/NVIC/NVIC.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void TimerBasic_Start(volatile SOOL_TimerBasic *timer_ptr);
static void TimerBasic_Stop(volatile SOOL_TimerBasic *timer_ptr);
static void TimerBasic_EnableNVIC(volatile SOOL_TimerBasic *timer_ptr);
static void TimerBasic_DisableNVIC(volatile SOOL_TimerBasic *timer_ptr);
static uint8_t TimerBasic_InterruptHandler(volatile SOOL_TimerBasic *timer_ptr);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/**
 * A generic timer which supports only update interrupts; initializes time base,
 * does NOT enable the TIMx peripheral
 * @param TIMx - for STM32F103C8T6 it may be TIM1, TIM2, TIM3 or TIM4
 * @param prescaler - clock divider (decremented value is loaded into TimeBaseInit structure)
 * @param period - period (decremented value is loaded into TimeBaseInit structure)
 * @param enable_int_update - specifies whether to enable TIM_IT_Update interrupt for TIMx
 * @return SOOL_TimerBasic instance
 */
volatile SOOL_TimerBasic SOOL_Periph_TIM_TimerBasic_Init(TIM_TypeDef* TIMx, uint16_t prescaler,
		uint16_t period, FunctionalState enable_int_update) {

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
	TIM_ITConfig(TIMx, TIM_IT_Update, enable_int_update);
	TIM_Cmd(TIMx, DISABLE);

	/* Configure NVIC if needed */
	NVIC_InitTypeDef nvic;
	nvic.NVIC_IRQChannel = SOOL_Periph_TIM_GetIRQnType(TIMx, SOOL_PERIPH_TIM_IRQ_UP); // for safety moved outside `if`

	if ( enable_int_update == ENABLE ) {

		nvic.NVIC_IRQChannelPreemptionPriority = 0;
		nvic.NVIC_IRQChannelSubPriority = 0;

		/* What's tricky there - when ENABLE is set NVIC will immediately trigger interrupt
		 * after NVIC_Init() call. The interrupt must have been handled by non (yet) existing
		 * object which produces `infinite loop`.
		 * SOLUTION: in IRQHandler namespace firstly initialize object's pointer to 0
		 * and then in ISR check whether the object has already been initialized and copied
		 * to the IRQn namespace.
		 * It will produce few interrupts which will not be handled but later on everything
		 * will run fine - this applies only to debugging, in real-time work MCU will get stuck.
		 *
		 * Example:
		 * if ( (timer_basic_ptr != 0) && (timer_basic_ptr->_InterruptHandler(timer_basic_ptr)) ) {
		 * }
		 */
		nvic.NVIC_IRQChannelCmd = DISABLE;
		NVIC_Init(&nvic);

		/* IT_Update surely needs to be cleared when `NVIC_IRQChannelCmd == ENABLE` */
		//TIMx->SR = (uint16_t)~TIM_IT_Update;

	}

	/* Save class' fields */
	timer._setup.TIMx = TIMx;
	timer._setup.NVIC_IRQ_channel = nvic.NVIC_IRQChannel;

	/* Set class' methods */
	timer.Start = TimerBasic_Start;
	timer.Stop = TimerBasic_Stop;
	timer.EnableNVIC = TimerBasic_EnableNVIC;
	timer.DisableNVIC = TimerBasic_DisableNVIC;
	timer._InterruptHandler = TimerBasic_InterruptHandler;

	return (timer);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void TimerBasic_Start(volatile SOOL_TimerBasic *timer_ptr) {
	/* Enable the TIM Counter */
	timer_ptr->_setup.TIMx->CR1 |= (uint16_t)TIM_CR1_CEN;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void TimerBasic_Stop(volatile SOOL_TimerBasic *timer_ptr) {
	/* Disable the TIM Counter */
	timer_ptr->_setup.TIMx->CR1 &= (uint16_t)(~((uint16_t)TIM_CR1_CEN));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void TimerBasic_EnableNVIC(volatile SOOL_TimerBasic *timer_ptr) {
	SOOL_Periph_NVIC_Enable(timer_ptr->_setup.NVIC_IRQ_channel);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void TimerBasic_DisableNVIC(volatile SOOL_TimerBasic *timer_ptr) {
	SOOL_Periph_NVIC_Disable(timer_ptr->_setup.NVIC_IRQ_channel);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t TimerBasic_InterruptHandler(volatile SOOL_TimerBasic *timer_ptr) {

	/* Check if update interrupt flag of the timer is set */
	if (TIM_GetITStatus(timer_ptr->_setup.TIMx, TIM_IT_Update) == RESET) {
		// not this timer overflowed (different IRQn)
		return (0);
	}

	/* Clear IT pending bit */
	timer_ptr->_setup.TIMx->SR = (uint16_t)~TIM_IT_Update; // TIM_ClearITPendingBit(timer->_TIMx, TIM_IT_Update);
	return (1);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
