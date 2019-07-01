/*
 * TimerCompare_common.c
 *
 *  Created on: 29.06.2019
 *      Author: user
 */

#include "sool/Peripherals/TIM/TimerCompare_common.h"

uint8_t SOOL_Periph_TIMCompare_IsCaptureCompareChannelEnabled(TIM_TypeDef* TIMx, uint16_t channel) {

	switch (channel) {

	case(TIM_Channel_1):
		return (TIMx->CCER & (uint16_t)TIM_CCER_CC1E);
		break;
	case(TIM_Channel_2):
		return (TIMx->CCER & (uint16_t)TIM_CCER_CC2E);
		break;
	case(TIM_Channel_3):
		return (TIMx->CCER & (uint16_t)TIM_CCER_CC3E);
		break;
	case(TIM_Channel_4):
		return (TIMx->CCER & (uint16_t)TIM_CCER_CC4E);
		break;
	default:
		return (0);
	}

}

uint16_t SOOL_Periph_TIMCompare_GetCCR(TIM_TypeDef* TIMx, uint16_t channel) {

	switch (channel) {

	case(TIM_Channel_1):
		return (TIMx->CCR1);
		break;
	case(TIM_Channel_2):
		return (TIMx->CCR2);
		break;
	case(TIM_Channel_3):
		return (TIMx->CCR3);
		break;
	case(TIM_Channel_4):
		return (TIMx->CCR4);
		break;
	default:
		return (0);
	}

}

void SOOL_Periph_TIMCompare_SetCCR(TIM_TypeDef* TIMx, uint16_t channel, uint16_t value) {

	switch (channel) {

	case(TIM_Channel_1):
		TIMx->CCR1 = (uint16_t)value;
		break;
	case(TIM_Channel_2):
		TIMx->CCR2 = (uint16_t)value;
		break;
	case(TIM_Channel_3):
		TIMx->CCR3 = (uint16_t)value;
		break;
	case(TIM_Channel_4):
		TIMx->CCR4 = (uint16_t)value;
		break;
	default:
		break;
	}

}

// DEPRECATED
//void SOOL_Periph_TIMCompare_SetCCMR(TIM_TypeDef* TIMx, uint16_t channel, uint16_t mask) {
//
//	// There are 2 CCMRx registers available
//	switch (channel) {
//
//	case(TIM_Channel_1):
//		TIMx->CCMR1 = (uint16_t)mask;
//		break;
//
//	case(TIM_Channel_2):
//		TIMx->CCMR1 = (uint16_t)mask;
//
//		break;
//	case(TIM_Channel_3):
//		TIMx->CCMR2 = (uint16_t)mask;
//		break;
//
//	case(TIM_Channel_4):
//		TIMx->CCMR2 = (uint16_t)mask;
//		break;
//
//	default:
//		break;
//
//	}
//
//}

void SOOL_Periph_TIMCompare_EnableChannel(TIM_TypeDef* TIMx, uint16_t channel) {

	/* Enable the Channel x: Set the CCxE Bit */
	switch (channel) {

	case(TIM_Channel_1):
		TIMx->CCER |= (uint16_t)TIM_CCER_CC1E;
		break;

	case(TIM_Channel_2):
		TIMx->CCER |= (uint16_t)TIM_CCER_CC2E;
		break;

	case(TIM_Channel_3):
		TIMx->CCER |= (uint16_t)TIM_CCER_CC3E;
		break;

	case(TIM_Channel_4):
		TIMx->CCER |= (uint16_t)TIM_CCER_CC4E;
		break;

	}

}
void SOOL_Periph_TIMCompare_DisableChannel(TIM_TypeDef* TIMx, uint16_t channel) {

	/* Disable the Channel x: Reset the CCxE Bit */
	switch (channel) {

	case(TIM_Channel_1):
		TIMx->CCER &= (uint16_t)(~((uint16_t)TIM_CCER_CC1E));
		break;

	case(TIM_Channel_2):
		TIMx->CCER &= (uint16_t)(~((uint16_t)TIM_CCER_CC2E));
		break;

	case(TIM_Channel_3):
		TIMx->CCER &= (uint16_t)(~((uint16_t)TIM_CCER_CC3E));
		break;

	case(TIM_Channel_4):
		TIMx->CCER &= (uint16_t)(~((uint16_t)TIM_CCER_CC4E));
		break;

	}

}
