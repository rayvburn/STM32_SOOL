/*
 * ADC_DMA.h
 *
 *  Created on: 15.05.2019
 *      Author: user
 */

#ifndef INC_SOOL_PERIPHERALS_ADC_ADC_DMA_H_
#define INC_SOOL_PERIPHERALS_ADC_ADC_DMA_H_

#include "ADC_Channel.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_dma.h"
#include "sool/Memory/Vector/VectorUint16.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

struct _SOOL_ADC_DMASetupStruct {
	ADC_TypeDef* 			ADCx;
	uint8_t 				NVIC_IRQChannel;
	uint8_t 				adc_channel_num;
	DMA_Channel_TypeDef*	dma_channel;
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/* Forward declaration */
struct _SOOL_ADC_DMAStruct;
typedef struct _SOOL_ADC_DMAStruct SOOL_ADC_DMA;

struct _SOOL_ADC_DMAStruct {

	struct _SOOL_ADC_DMASetupStruct _setup;
	SOOL_Vector_Uint16				_v;

	/**
	 * @brief Adds channel, extends vector of values where readings are stored
	 * @note Call this when ADC_DMA is disabled
	 * @param
	 * @param SOOL_ADC_Channel* - pointer to SOOL_ADC_Channel instance (it has `rank` field modified)
	 */
	void (*AddChannel)(volatile SOOL_ADC_DMA*, SOOL_ADC_Channel*);

	/**
	 * @brief Returns the newest value associated with a given channel
	 * @param
	 * @param
	 * @return
	 */
	uint16_t (*GetReading)(volatile SOOL_ADC_DMA*, SOOL_ADC_Channel);

	void (*EnableNVIC)(volatile SOOL_ADC_DMA*);
	void (*DisableNVIC)(volatile SOOL_ADC_DMA*);

	void (*Start)(volatile SOOL_ADC_DMA*);
	void (*Stop)(volatile SOOL_ADC_DMA*);

};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/**
 * @brief Initializer of ADC and DMA peripherals, creates interface between them
 * @note Works only with ADC1 and ADC3 (if available), ADC2 is not equipped in DMA interface.
 * @note Works in continuous mode by default, no interrupts are generated, all happens in `background`,
 * value ready to read can be achieved via GetReading member function
 * @note Always the newest values are stored in vector and can be read anytime
 * @param ADCx
 * @param RCC_PCLK2_DivX
 * @return
 */
extern volatile SOOL_ADC_DMA SOOL_Periph_ADC_DMA_Init(ADC_TypeDef* ADCx, uint32_t RCC_PCLK2_DivX);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#endif /* INC_SOOL_PERIPHERALS_ADC_ADC_DMA_H_ */
