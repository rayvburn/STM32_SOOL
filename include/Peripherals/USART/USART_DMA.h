/*
 * USART_DMA.h
 *
 *  Created on: 26.04.2019
 *      Author: user
 */

#ifndef INCLUDE_PERIPHERALS_USART_USART_DMA_H_
#define INCLUDE_PERIPHERALS_USART_USART_DMA_H_

// ST libs
#include "stm32f10x.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_dma.h"
#include "misc.h"

// C libs
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// SOOL
#include <include/Memory/Array.h>

// - - - - - - - - - - - - - - - -
/* References:
 * https://elektronika327.blogspot.com/2017/08/28-stm32f4-usart-rx-oraz-tx-z-dma.html
 * http://stm32f4-discovery.net/2017/07/stm32-tutorial-efficiently-receive-uart-data-using-dma/
 * https://github.com/avislab/STM32F103/blob/master/Example_BMP280/main.c
 * https://stackoverflow.com/questions/43298708/stm32-implementing-uart-in-dma-mode
 * https://github.com/mubes/blackmagic/blob/bluepill/src/platforms/stm32/traceswoasync.c
 */

// - - - - - - - - - - - - - - - -
typedef struct {

	USART_TypeDef*			usart_periph;
//	NVIC_InitTypeDef 		nvic;			// to be able to disable interrupts
	DMA_Channel_TypeDef* 	dma_rx_channel;
	uint32_t				dma_rx_tc_flag;
//	DMA_InitTypeDef			dma_rx_config;
	DMA_Channel_TypeDef* 	dma_tx_channel;
	uint32_t				dma_tx_tc_flag;
//	DMA_InitTypeDef			dma_tx_config;

} UsartDmaConfiguration;

// - - - - - - - - - - - - - - - -

typedef struct {
	ArrayString 		rx_buffer;
	uint8_t 			new_data_flag;
} UsartRx;

// - - - - - - - - - - - - - - - -

typedef struct {
	ArrayString 		tx_buffer;
	uint8_t				transfer_finished;
} UsartTx;

// - - - - - - - - - - - - - - - -

struct UsartPeriphStruct;
typedef struct UsartPeriphStruct UsartPeriph;

struct UsartPeriphStruct {

	UsartDmaConfiguration 	setup;
	UsartRx					rx;
	UsartTx					tx;

	// methods
	uint8_t (*Send)(UsartPeriph*, char*);
	uint8_t (*IsDataReceived)(UsartPeriph*);
	uint8_t (*DmaTcIrqHandler)(UsartPeriph*);

};

// - - - - - - - - - - - - - - - -

volatile UsartPeriph SOOL_USART_DMA_Init(USART_TypeDef* usart_periph_id, uint32_t baud);

// - - - - - - - - - - - - - - - -

#endif /* INCLUDE_PERIPHERALS_USART_USART_DMA_H_ */
