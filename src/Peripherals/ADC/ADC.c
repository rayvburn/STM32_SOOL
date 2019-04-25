/*
 * adc.c
 *
 *  Created on: 26.07.2018
 *      Author: user
 */

#include "ADC.h"

#include "stm32f10x_adc.h"
#include "stm32f10x_rcc.h"
#include "misc.h" // NVIC

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#ifndef USE_DMA_FOR_ADC
volatile static uint8_t  adc_current_channel = 0;
volatile static uint8_t  adc_measurement_started = 0;
volatile static uint8_t  adc_just_read = 0;
volatile static uint16_t adc_last_reading = 0;
#endif

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void ADC_Config(FunctionalState state) {

	RCC_ADCCLKConfig(RCC_PCLK2_Div2);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

	// is NVIC needed when DMA takes care of readings?
	NVIC_InitTypeDef nvic;
	nvic.NVIC_IRQChannel = ADC1_2_IRQn; // !
	nvic.NVIC_IRQChannelPreemptionPriority = 0;
	nvic.NVIC_IRQChannelSubPriority = 0;
	nvic.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvic);

	ADC_InitTypeDef adc;
	ADC_StructInit(&adc);
	adc.ADC_ScanConvMode = DISABLE;  			// single channel used
	adc.ADC_ContinuousConvMode = ENABLE;
	adc.ADC_NbrOfChannel = ADC_CHANNELS_TOTAL;	// nb of inputs used
	adc.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_Init(ADC1, &adc);


#ifndef USE_DMA_FOR_ADC
	ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE); // causes lag because of continuous assignment of value to adc_last_reading variable
#endif


	// - - - - - depends on ADC_CHANNELS_TOTAL - - - - -
	ADC_RegularChannelConfig(ADC1, POTENTIOMETER_ADC_CHANNEL, 1, ADC_SampleTime_1Cycles5);
	//ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 2, ADC_SampleTime_1Cycles5);
	//ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 3, ADC_SampleTime_1Cycles5);
	//ADC_RegularChannelConfig(ADC1, ADC_Channel_3, 4, ADC_SampleTime_1Cycles5);


#ifdef USE_DMA_FOR_ADC
	ADC_DMACmd(ADC1, ENABLE);	// to turn on the adc-dma interface?
#endif

	ADC_Cmd(ADC1, ENABLE);		// needs to be enabled for calibration

	// calibration
	ADC_ResetCalibration(ADC1);
	while (ADC_GetResetCalibrationStatus(ADC1));
	ADC_StartCalibration(ADC1);
	while (ADC_GetCalibrationStatus(ADC1));

	ADC_Cmd(ADC1, state);     	// to turn on the peripheral?
	ADC_SoftwareStartConvCmd(ADC1, DISABLE); 	// to start reading?

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void ADC_SwitchCmd(FunctionalState state) {

#ifdef USE_DMA_FOR_ADC
	// ( state == ENABLE ) ? (ADC_DMACmd(ADC1, ENABLE)) : (0);	// seems to switch off when ADC DISABLED
#endif

	// ADC_DMACmd(ADC1, state);
	ADC_Cmd(ADC1, state);
	ADC_SoftwareStartConvCmd(ADC1, state);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#ifndef USE_DMA_FOR_ADC

void ADC_SelectChannel(uint8_t channel) {

	adc_just_read = 0;
	adc_measurement_started = 1;

	if ( channel < 4 ) { // 4 input signals PA0 - PA3

		adc_current_channel = channel;

		switch (channel) {
			case(0):
				channel = ADC_Channel_0;
				break;
			case(1):
				channel = ADC_Channel_1;
				break;
			case(2):
				channel = ADC_Channel_2;
				break;
			case(3):
				channel = ADC_Channel_3;
				break;
		}

		ADC_RegularChannelConfig(ADC1, channel, 1, ADC_SampleTime_1Cycles5);
		ADC_SoftwareStartConvCmd(ADC1, ENABLE); // start measurement
	}

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void ADC1_IRQHandler() {

	if (ADC_GetITStatus(ADC1, ADC_IT_EOC) == SET)	{

		ADC_ClearITPendingBit(ADC1, ADC_IT_EOC);
		adc_last_reading = ADC_GetConversionValue(ADC1);
		adc_just_read = 1;
		adc_measurement_started = 0;

	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

uint8_t ADC_GetCurrChannel() {

	return adc_current_channel;

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

uint8_t ADC_ValueReady() {

	if ( adc_just_read ) {
		return 1;
	} else {
		return 0;
	}

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

uint16_t ADC_GetReading() {

	if ( adc_just_read ) {
		adc_just_read = 0;
		return adc_last_reading;
	} else {
		return 9999; // value out of 12-bit ADC range
	}

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

uint8_t ADC_IsReadyToSelectChannel() {
	uint8_t out = 0;
	(adc_measurement_started) ? (out = 0) : (out = 1);
	return out;
}

#endif
