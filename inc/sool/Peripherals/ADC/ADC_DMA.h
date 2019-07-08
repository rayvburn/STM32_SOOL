/*
 * ADC_DMA.h
 *
 *  Created on: 15.05.2019
 *      Author: user
 */

#ifndef INC_SOOL_PERIPHERALS_ADC_ADC_DMA_H_
#define INC_SOOL_PERIPHERALS_ADC_ADC_DMA_H_

#include "ADC_Channel.h"
#include "sool/Memory/Vector/VectorUint16.h"

struct _SOOL_ADC_DMAStruct;
struct _SOOL_ADC_DMASetupStruct;

struct _SOOL_ADC_DMASetupStruct {
	ADC_TypeDef* ADCx;
	uint8_t NVIC_IRQChannel;
	uint8_t adc_channel_num;
	DMA_Channel_TypeDef *dma_channel;
};

/*
 * uint8_t ARRAY_SIZE;
 *
 */
typedef struct _SOOL_ADC_DMAStruct SOOL_ADC_DMA;

struct _SOOL_ADC_DMAStruct {

	struct _SOOL_ADC_DMASetupStruct _setup;
	SOOL_Vector_Uint16				_v;

	/**
	 * @note Call this when ADC_DMA is disabled
	 * @param
	 * @param
	 */
	void (*AddChannel)(volatile SOOL_ADC_DMA*, SOOL_ADC_Channel);
	uint16_t (*GetReading)(volatile SOOL_ADC_DMA*, SOOL_ADC_Channel);

	void (*EnableNVIC)(volatile SOOL_ADC_DMA*);
	void (*DisableNVIC)(volatile SOOL_ADC_DMA*);

	void (*Start)(volatile SOOL_ADC_DMA*);
	void (*Stop)(volatile SOOL_ADC_DMA*);

};

// Works only with ADC1 and ADC3 (if available), ADC2 is not equipped in DMA interface
// Works in continuous mode by default, no interrupts are generated, all happens in `background`
// Always the newest values are stored in vector and can be read anytime
volatile SOOL_ADC_DMA SOOL_Periph_ADC_DMA_Init(ADC_TypeDef* ADCx, uint32_t RCC_PCLK2_DivX);

#endif /* INC_SOOL_PERIPHERALS_ADC_ADC_DMA_H_ */
