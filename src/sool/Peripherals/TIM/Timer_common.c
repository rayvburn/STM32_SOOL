/*
 * Timer_common.c
 *
 *  Created on: 20.05.2019
 *      Author: user
 */

#include "sool/Peripherals/TIM/Timer_common.h"
#include "stm32f10x_rcc.h"

/* start at:
 * "stm32f10x.h" ->
	#ifdef STM32F10X_MD
  		ADC1_2_IRQn   (..)
 */

void SOOL_Periph_TIM_EnableAPBClock(TIM_TypeDef* TIMx) {

	/* Left only those which are supported by STM32F103x8 */
	if (        TIMx == TIM1  ) {	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1,  ENABLE);
//	} else if ( TIMx == TIM8  ) {	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8,  ENABLE);
//	} else if ( TIMx == TIM9  ) { 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM9,  ENABLE);
//	} else if ( TIMx == TIM10 ) { 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM10, ENABLE);
//	} else if ( TIMx == TIM11 ) { 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM11, ENABLE);
//	} else if ( TIMx == TIM15 ) {	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM15, ENABLE);
//	} else if ( TIMx == TIM16 ) {	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM16, ENABLE);
//	} else if ( TIMx == TIM17 ) {	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM17, ENABLE);
	} else if ( TIMx == TIM2  ) {	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,  ENABLE);
	} else if ( TIMx == TIM3  ) {	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,  ENABLE);
	} else if ( TIMx == TIM4  ) {	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,  ENABLE);
//	} else if ( TIMx == TIM5  ) {	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5,  ENABLE);
//	} else if ( TIMx == TIM6  ) {	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6,  ENABLE);
//	} else if ( TIMx == TIM7  ) {	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7,  ENABLE);
//	} else if ( TIMx == TIM12 ) {	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM12, ENABLE);
//	} else if ( TIMx == TIM13 ) {	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM13, ENABLE);
//	} else if ( TIMx == TIM14 ) {	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM14, ENABLE);
	}

}

uint8_t SOOL_Periph_TIM_GetIRQnType(TIM_TypeDef* TIMx, SOOL_Periph_TIM_IRQnType irqn_type) {

	/* Left only those which are supported by STM32F103x8 */

	if ( TIMx == TIM1  ) {

		// further investigation is needed for TIM1
		switch (irqn_type) {
		case(SOOL_PERIPH_TIM_IRQ_UP):
			return (TIM1_UP_IRQn);
			break;
		case(SOOL_PERIPH_TIM_IRQ_CC):
			return (TIM1_CC_IRQn);
			break;
		case(SOOL_PERIPH_TIM_IRQ_BRK):
			return (TIM1_BRK_IRQn);
			break;
		case(SOOL_PERIPH_TIM_IRQ_TRG_COM):
			return (TIM1_TRG_COM_IRQn);
			break;
		default:
			return (TIM1_UP_IRQn);
			break;
		}

//	} else if ( TIMx == TIM8  ) {	return (TIM8_UP_IRQn);
//	} else if ( TIMx == TIM9  ) { 	return (TIM1_BRK_TIM9_IRQn);
//	} else if ( TIMx == TIM10 ) { 	return (TIM1_UP_TIM10_IRQn);
//	} else if ( TIMx == TIM11 ) { 	return (TIM1_TRG_COM_TIM11_IRQn);
//	} else if ( TIMx == TIM15 ) {	return (TIM1_BRK_TIM15_IRQn);
//	} else if ( TIMx == TIM16 ) {	return ();
//	} else if ( TIMx == TIM17 ) {	return ();
	} else if ( TIMx == TIM2  ) {	return (TIM2_IRQn);
	} else if ( TIMx == TIM3  ) {	return (TIM3_IRQn);
	} else if ( TIMx == TIM4  ) {	return (TIM4_IRQn);
//	} else if ( TIMx == TIM5  ) {	return ();
//	} else if ( TIMx == TIM6  ) {	return ();
//	} else if ( TIMx == TIM7  ) {	return ();
//	} else if ( TIMx == TIM12 ) {	return ();
//	} else if ( TIMx == TIM13 ) {	return ();
//	} else if ( TIMx == TIM14 ) {	return ();
	} else { 						return(0); 	/* Window WatchDog Interrupt */
	}

}
