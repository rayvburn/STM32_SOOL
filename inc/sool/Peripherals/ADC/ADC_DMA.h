/*
 * ADC_DMA.h
 *
 *  Created on: 15.05.2019
 *      Author: user
 */

#ifndef INC_SOOL_PERIPHERALS_ADC_ADC_DMA_H_
#define INC_SOOL_PERIPHERALS_ADC_ADC_DMA_H_

struct _SOOL_ADC_DMAStruct;

/*
 * uint8_t ARRAY_SIZE;
 *
 */
typedef struct _SOOL_ADC_DMAStruct SOOL_ADC_DMA;

volatile SOOL_ADC_DMA SOOL_Periph_ADC_DMA_Init();

#endif /* INC_SOOL_PERIPHERALS_ADC_ADC_DMA_H_ */
