/*
 * Potentiometer.c
 *
 *  Created on: 20.10.2018
 *      Author: user
 */

#include "Potentiometer.h"
#include "DMA.h"
#include "Filters.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void Potentiometer_Config() {

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	GPIO_InitTypeDef gpio;
	GPIO_StructInit(&gpio);

	gpio.GPIO_Pin = GPIO_Pin_0;
	gpio.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOA, &gpio);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

uint16_t Potentiometer_GetReading() {

	ADC_SwitchCmd(DISABLE);     // kind of a mutex / power off for a while (array with values could be changed during filering)
	uint16_t *arr_ptr = DMA_ADC_GetADCArrayAddress();
	uint16_t temp = arr_ptr[POTENTIOMETER_FILTERING_SAMPLES-1]; // get the newest value
	ADC_SwitchCmd(ENABLE);
	return temp;

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

uint16_t Potentiometer_GetFilteredReading() {

	ADC_SwitchCmd(DISABLE);
	uint16_t temp = (uint16_t)Filter_GetAvg(DMA_ADC_GetADCArrayAddress(), POTENTIOMETER_FILTERING_SAMPLES);
	ADC_SwitchCmd(ENABLE);
	return temp;

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
