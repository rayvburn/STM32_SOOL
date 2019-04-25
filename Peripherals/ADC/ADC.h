/*
 * adc.h
 *
 *  Created on: 26.07.2018
 *      Author: user
 */

#ifndef ADC_H_
#define ADC_H_

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#include "stm32f10x.h"
#include "Potentiometer.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#define USE_DMA_FOR_ADC
#define ADC_CHANNELS_TOTAL 1

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

extern void 	ADC_Config(FunctionalState state);
extern void 	ADC_SwitchCmd(FunctionalState state);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#ifndef USE_DMA_FOR_ADC
extern void 	ADC_SelectChannel(uint8_t channel);
extern void 	ADC1_IRQHandler();

extern uint8_t 	ADC_GetCurrChannel();
extern uint8_t 	ADC_ValueReady();
extern uint16_t	ADC_GetReading();
extern uint8_t 	ADC_IsReadyToSelectChannel();
#endif

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#endif /* ADC_H_ */

