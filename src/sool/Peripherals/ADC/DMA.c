/*
 * DMA.c
 *
 *  Created on: 20.09.2018
 *      Author: user
 */

#include "sool/Peripherals/ADC/DMA.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_dma.h"
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static uint16_t dma_adc_values[POTENTIOMETER_FILTERING_SAMPLES];
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void DMA_ADC_Config(FunctionalState state) {

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	DMA_InitTypeDef dma;
	DMA_StructInit(&dma);

	dma.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;
	dma.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	dma.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	dma.DMA_MemoryBaseAddr = (uint32_t)dma_adc_values;
	dma.DMA_MemoryInc = DMA_MemoryInc_Enable;
	dma.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	dma.DMA_DIR = DMA_DIR_PeripheralSRC;
	dma.DMA_BufferSize = POTENTIOMETER_FILTERING_SAMPLES;
	dma.DMA_Mode = DMA_Mode_Circular;
	DMA_Init(DMA1_Channel1, &dma);
	DMA_Cmd(DMA1_Channel1, state);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void DMA_ADC_SwitchCmd(FunctionalState state) {
	DMA_Cmd(DMA1_Channel1, state);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

inline uint32_t DMA_ADC_GetADCArrayAddress() {
	return (uint32_t)dma_adc_values;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
