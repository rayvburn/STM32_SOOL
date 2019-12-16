/*
 * ADC_common.c
 *
 *  Created on: 16.12.2019
 *      Author: user
 */

#include <sool/Peripherals/ADC/ADC_common.h>
#include "stm32f10x_adc.h"

extern void SOOL_ADC_Common_ConvertChannelToPortPin(uint8_t ADC_Channel, GPIO_TypeDef** GPIO_Port, uint16_t* GPIO_Pin) {

	switch (ADC_Channel) {
		case(ADC_Channel_0):
			*GPIO_Port = GPIOA;
			*GPIO_Pin = GPIO_Pin_0;
			break;
		case(ADC_Channel_1):
			*GPIO_Port = GPIOA;
			*GPIO_Pin = GPIO_Pin_1;
			break;
		case(ADC_Channel_2):
			*GPIO_Port = GPIOA;
			*GPIO_Pin = GPIO_Pin_2;
			break;
		case(ADC_Channel_3):
			*GPIO_Port = GPIOA;
			*GPIO_Pin = GPIO_Pin_3;
			break;
		case(ADC_Channel_4):
			*GPIO_Port = GPIOA;
			*GPIO_Pin = GPIO_Pin_4;
			break;
		case(ADC_Channel_5):
			*GPIO_Port = GPIOA;
			*GPIO_Pin = GPIO_Pin_5;
			break;
		case(ADC_Channel_6):
			*GPIO_Port = GPIOA;
			*GPIO_Pin = GPIO_Pin_6;
			break;
		case(ADC_Channel_7):
			*GPIO_Port = GPIOA;
			*GPIO_Pin = GPIO_Pin_7;
			break;
		case(ADC_Channel_8):
			*GPIO_Port = GPIOB;
			*GPIO_Pin = GPIO_Pin_0;
			break;
		case(ADC_Channel_9):
			*GPIO_Port = GPIOB;
			*GPIO_Pin = GPIO_Pin_1;
			break;
		default:
			break;
	}

}

