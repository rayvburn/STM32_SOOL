/*
 * DMA.h
 *
 *  Created on: 20.09.2018
 *      Author: user
 */

#ifndef DMA_H_
#define DMA_H_

#include "stm32f10x.h"
#include "ADC.h"
#include "Potentiometer.h"

extern void 		DMA_ADC_Config(FunctionalState state);
extern void 		DMA_ADC_SwitchCmd(FunctionalState state);
extern uint32_t 	DMA_ADC_GetADCArrayAddress();

#endif /* DMA_H_ */
