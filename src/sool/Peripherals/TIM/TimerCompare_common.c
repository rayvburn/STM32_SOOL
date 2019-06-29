/*
 * TimerCompare_common.c
 *
 *  Created on: 29.06.2019
 *      Author: user
 */

#include "sool/Peripherals/TIM/TimerCompare_common.h"

uint8_t SOOL_Periph_TIM_IsCaptureCompareChannelEnabled(TIM_TypeDef* TIMx, uint16_t channel) {

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

uint16_t SOOL_Periph_TIM_GetCCR(TIM_TypeDef* TIMx, uint16_t channel) {

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

void SOOL_Periph_TIM_SetCCR(TIM_TypeDef* TIMx, uint16_t channel, uint16_t value) {

	switch (channel) {

	case(TIM_Channel_1):
		TIMx->CCR1 = value;
		break;
	case(TIM_Channel_2):
		TIMx->CCR2 = value;
		break;
	case(TIM_Channel_3):
		TIMx->CCR3 = value;
		break;
	case(TIM_Channel_4):
		TIMx->CCR4 = value;
		break;
	default:
		break;
	}

}

