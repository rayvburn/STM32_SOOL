/*
 * ADC_DMA.c
 *
 *  Created on: 15.05.2019
 *      Author: user
 */

#include "sool/Peripherals/ADC/ADC_DMA.h"
#include "stm32f10x_rcc.h" // #include "stm32f10x.h"
#include "sool/Peripherals/NVIC/NVIC.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void SOOL_ADC_DMA_AddChannel(volatile SOOL_ADC_DMA *adc_dma_ptr, SOOL_ADC_Channel *channel_ptr);
static uint16_t SOOL_ADC_DMA_GetReading(volatile SOOL_ADC_DMA *adc_dma_ptr, SOOL_ADC_Channel channel);

static void SOOL_ADC_DMA_EnableNVIC(volatile SOOL_ADC_DMA *adc_dma_ptr);
static void SOOL_ADC_DMA_DisableNVIC(volatile SOOL_ADC_DMA *adc_dma_ptr);

static void SOOL_ADC_DMA_Start(volatile SOOL_ADC_DMA *adc_dma_ptr);
static void SOOL_ADC_DMA_Stop(volatile SOOL_ADC_DMA *adc_dma_ptr);

// helper
static void SOOL_ADC_DMA_SetNumberOfChannels(volatile SOOL_ADC_DMA *adc_dma_ptr);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

volatile SOOL_ADC_DMA SOOL_Periph_ADC_DMA_Init(ADC_TypeDef* ADCx, uint32_t RCC_PCLK2_DivX) {

	/* Create a new instance */
	volatile SOOL_ADC_DMA adc_dma;

	/* Check peripheral */
	if ( ADCx == ADC2 ) {
		return (adc_dma);
	}

	/* Initialize vector of values */
	adc_dma._v = SOOL_Memory_Vector_Uint16_Init();

	/* Configure ADC clock divider - hard coded parameter, */
	RCC_ADCCLKConfig(RCC_PCLK2_DivX);

	/* Enable ADC clock */
	if ( ADCx == ADC1 ) { RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE); } else
	if ( ADCx == ADC2 ) { RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC2, ENABLE); } else
	if ( ADCx == ADC3 ) { RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC3, ENABLE); }

	/* Configure peripheral */
	ADC_InitTypeDef adc;
	ADC_StructInit(&adc);
	adc.ADC_ScanConvMode = DISABLE;  			// single channel used
	adc.ADC_ContinuousConvMode = ENABLE;
	adc.ADC_NbrOfChannel = 0;					// nb of inputs used, will be updated in each AddChannel call
	adc.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_Init(ADCx, &adc);

//	DEPRECATED
//	/* Configure ADC interrupts */
//	ADC_ITConfig(ADCx, ADC_IT_EOC, !continuous_mode);

	/* Configure NVIC */
	NVIC_InitTypeDef nvic;
	nvic.NVIC_IRQChannel = ADC1_2_IRQn;			// TODO: hard-coded version for STM32F103
	nvic.NVIC_IRQChannelPreemptionPriority = 0;
	nvic.NVIC_IRQChannelSubPriority = 0;
	nvic.NVIC_IRQChannelCmd = DISABLE;
	NVIC_Init(&nvic);

	/* Enable DMA clock, find DMA Channel */
	DMA_TypeDef *dma_id = 0;
	DMA_Channel_TypeDef *dma_channel = 0;

	if ( ADCx == ADC1 ) {
		RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
		dma_id = DMA1;
		dma_channel = DMA1_Channel1;
	} else if ( ADCx == ADC3 ) {
		RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);
		dma_id = DMA2;
		dma_channel = DMA2_Channel5;
	}

	/* Configure DMA peripheral */
//	DMA_InitTypeDef dma;
//	DMA_StructInit(&dma);
//
//	dma.DMA_PeripheralBaseAddr = (uint32_t)&ADCx->DR;
//	dma.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
//	dma.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
//	dma.DMA_MemoryBaseAddr = (uint32_t)0;							// at this moment vector of values is not ready
//	dma.DMA_MemoryInc = DMA_MemoryInc_Enable;
//	dma.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
//	dma.DMA_DIR = DMA_DIR_PeripheralSRC;
//	dma.DMA_BufferSize = 0;
//	dma.DMA_Mode = DMA_Mode_Circular;
//	DMA_Init(dma_channel, &dma);
//	DMA_Cmd(dma_channel, DISABLE);
	SOOL_DMA dma = SOOL_Periph_DMA_Init(dma_id, dma_channel, DMA_DIR_PeripheralSRC, DMA_PeripheralInc_Disable,
				   DMA_MemoryInc_Enable, DMA_PeripheralDataSize_HalfWord, DMA_MemoryDataSize_HalfWord,
				   DMA_Mode_Circular, DMA_Priority_Low, DMA_M2M_Disable, DISABLE, DISABLE, DISABLE);

	dma.SetPeriphBaseAddr(&dma, (uint32_t)&ADCx->DR);

	/* Enable ADC-DMA interface */
	ADC_DMACmd(ADCx, ENABLE);

	/* Perform calibration (requires ADCx to be enabled) */
	ADC_Cmd(ADCx, ENABLE);
	ADC_ResetCalibration(ADCx);
	while (ADC_GetResetCalibrationStatus(ADCx));
	ADC_StartCalibration(ADCx);
	while (ADC_GetCalibrationStatus(ADCx));
	ADC_Cmd(ADCx, DISABLE);

	/* Save setup variables */
	adc_dma._setup.ADCx = ADCx;
	adc_dma._setup.NVIC_IRQChannel = ADC1_2_IRQn;	// TODO: hard-coded version for STM32F103
	adc_dma._setup.adc_channel_num = 0;
//	adc_dma._setup.dma_channel = dma_channel;

	/* Save base class */
	adc_dma.base_dma = dma;

	/* Set internal functions pointers */
	adc_dma.AddChannel = SOOL_ADC_DMA_AddChannel;
	adc_dma.DisableNVIC = SOOL_ADC_DMA_DisableNVIC;
	adc_dma.EnableNVIC = SOOL_ADC_DMA_EnableNVIC;
	adc_dma.GetReading = SOOL_ADC_DMA_GetReading;
	adc_dma.Start = SOOL_ADC_DMA_Start;
	adc_dma.Stop = SOOL_ADC_DMA_Stop;

	return (adc_dma);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void SOOL_ADC_DMA_AddChannel(volatile SOOL_ADC_DMA *adc_dma_ptr, SOOL_ADC_Channel *channel_ptr) {

	/* Set rank (value between 1 - 16) */
	channel_ptr->rank = adc_dma_ptr->_setup.adc_channel_num + 1;

	/* Save number of channels */
	adc_dma_ptr->_setup.adc_channel_num++;

	/* Resize buffer, put 0 inside */
	adc_dma_ptr->_v.Add(&adc_dma_ptr->_v, 0);

	/* Initialize channel */
	ADC_RegularChannelConfig(adc_dma_ptr->_setup.ADCx, channel_ptr->adc_channel, channel_ptr->rank, channel_ptr->sample_time);

	/* Update number of channels in the ADC_SQR1 register */
	SOOL_ADC_DMA_SetNumberOfChannels(adc_dma_ptr);

	/* Update number of channels in the DMA CNDTR register */
//	adc_dma_ptr->_setup.dma_channel->CNDTR = adc_dma_ptr->_setup.adc_channel_num;
	adc_dma_ptr->base_dma.SetBufferSize(&adc_dma_ptr->base_dma, (uint32_t)adc_dma_ptr->_setup.adc_channel_num);

	/* Update memory address in the DMA peripheral */
//	adc_dma_ptr->_setup.dma_channel->CMAR = adc_dma_ptr->_v._data;
	adc_dma_ptr->base_dma.SetMemoryBaseAddr(&adc_dma_ptr->base_dma, (uint32_t)adc_dma_ptr->_v._data);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint16_t SOOL_ADC_DMA_GetReading(volatile SOOL_ADC_DMA *adc_dma_ptr, SOOL_ADC_Channel channel) {
	return (adc_dma_ptr->_v.Get(&adc_dma_ptr->_v, channel.rank - 1));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void SOOL_ADC_DMA_EnableNVIC(volatile SOOL_ADC_DMA *adc_dma_ptr) {
	SOOL_Periph_NVIC_Enable(adc_dma_ptr->_setup.NVIC_IRQChannel);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void SOOL_ADC_DMA_DisableNVIC(volatile SOOL_ADC_DMA *adc_dma_ptr) {
	SOOL_Periph_NVIC_Disable(adc_dma_ptr->_setup.NVIC_IRQChannel);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void SOOL_ADC_DMA_Start(volatile SOOL_ADC_DMA *adc_dma_ptr) {

//	adc_dma_ptr->_setup.dma_channel->CCR |= DMA_CCR1_EN;		// DMA_Cmd(adc_dma_ptr->_setup.dma_channel, ENABLE);
	adc_dma_ptr->base_dma.Start(&adc_dma_ptr->base_dma);

	// #define CR2_ADON_Set                ((uint32_t)0x00000001)
	adc_dma_ptr->_setup.ADCx->CR2 |= ((uint32_t)0x00000001);	// ADC_Cmd(adc_dma_ptr->_setup.ADCx, ENABLE);

	// #define CR2_EXTTRIG_SWSTART_Set     ((uint32_t)0x00500000)
	adc_dma_ptr->_setup.ADCx->CR2 |= ((uint32_t)0x00500000); 	// ADC_SoftwareStartConvCmd(adc_dma_ptr->_setup.ADCx, ENABLE);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void SOOL_ADC_DMA_Stop(volatile SOOL_ADC_DMA *adc_dma_ptr) {

	// FAIL
//	adc_dma_ptr->_setup.dma_channel->CCR |= DMA_CCR1_EN;		// DMA_Cmd(dma_channel, DISABLE);
	// OK
	adc_dma_ptr->base_dma.Stop(&adc_dma_ptr->base_dma);

	// #define CR2_ADON_Reset              ((uint32_t)0xFFFFFFFE)
	adc_dma_ptr->_setup.ADCx->CR2 &= ((uint32_t)0xFFFFFFFE);	// ADC_Cmd(adc_dma_ptr->_setup.ADCx, DISABLE);

	// #define CR2_EXTTRIG_SWSTART_Reset   ((uint32_t)0xFFAFFFFF)
	adc_dma_ptr->_setup.ADCx->CR2 &= ((uint32_t)0xFFAFFFFF); 	// ADC_SoftwareStartConvCmd(adc_dma_ptr->_setup.ADCx, DISABLE);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// helper
static void SOOL_ADC_DMA_SetNumberOfChannels(volatile SOOL_ADC_DMA *adc_dma_ptr) {

	// RM0008, p. 247, ADC_SQR1
	uint32_t reg_val = adc_dma_ptr->_setup.ADCx->SQR1 & 0xFF0FFFFF; // reset bits from [20] to [23]

	/* Clear bits [20] to [23] */
	adc_dma_ptr->_setup.ADCx->SQR1 &= reg_val;

	/* Set bits [20] to [23] depending on channel number */
	adc_dma_ptr->_setup.ADCx->SQR1 = adc_dma_ptr->_setup.adc_channel_num << 20;

}
