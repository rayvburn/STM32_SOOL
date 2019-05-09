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

/* USART coupled with DMA - DMA RX channel configuration */
typedef struct {
	DMA_Channel_TypeDef* 	dma_channel;
	uint32_t				dma_tc_flag;
	uint8_t 				dma_channel_id;	// IT flags bitwise shifting (see @defgroup DMA_interrupts_definition )
//	DMA_InitTypeDef			dma_config;		// raw register value will be changed (buf size)
} UsartDmaRxConfiguration;

// - - - - - - - - - - - - - - - -

/* USART coupled with DMA - DMA TX channel configuration */
typedef struct {
	DMA_Channel_TypeDef* 	dma_channel;
	uint32_t				dma_tc_flag;
	uint8_t 				dma_channel_id;	// IT flags bitwise shifting (see @defgroup DMA_interrupts_definition )
//	DMA_InitTypeDef			dma_config;		// raw register value will be changed (buf size)
} UsartDmaTxConfiguration;

// - - - - - - - - - - - - - - - -

/* USART coupled with DMA configuration */
typedef struct {

	USART_TypeDef*			usart_periph;
//	NVIC_InitTypeDef 		nvic;			// to be able to disable interrupts
	UsartDmaRxConfiguration	dma_rx;
	UsartDmaTxConfiguration dma_tx;

} UsartDmaConfiguration;

// - - - - - - - - - - - - - - - -

typedef struct {
	ArrayString 		rx_buffer;
	uint8_t 			new_data_flag;
	uint8_t				data_fully_received_flag;
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
	uint8_t (*Send)(volatile UsartPeriph*, char*);
	uint8_t (*IsDataReceived)(volatile UsartPeriph*);
	uint8_t (*DmaTxIrqHandler)(volatile UsartPeriph*);

};

// - - - - - - - - - - - - - - - -

volatile UsartPeriph SOOL_USART_DMA_Init(USART_TypeDef* usart_periph_id, uint32_t baud);

// - - - - - - - - - - - - - - - -

/* STMF103xB Reference manual notes */

/* 	-----------------------------------------------------------------------------
 * 	DMA section, "DMA main features", p. 274
 * 		3 event flags (DMA Half Transfer, DMA Transfer complete and DMA Transfer Error)
 * 		logically ORed together in a single interrupt request for each channel
 *	-----------------------------------------------------------------------------
 * 	DMA section, "DMA transactions", p. 277
 * 	each DMA transfer consists of three operations:
 *
 *		• The loading of data from the peripheral data register or a location in memory addressed
 *			through an internal current peripheral/memory address register. The start address used
 *			for the first transfer is the base peripheral/memory address programmed in the
 *			DMA_CPARx or DMA_CMARx register
 *
 *			StdPeriph:
 *			DMA_CPARx <-> DMA_InitStruct->DMA_PeripheralBaseAddr
 *			DMA_CMARx <-> DMA_InitStruct->DMA_MemoryBaseAddr
 *
 *		• The storage of the data loaded to the peripheral data register or a location in memory
 *			addressed through an internal current peripheral/memory address register. The start
 *			address used for the first transfer is the base peripheral/memory address programmed
 *			in the DMA_CPARx or DMA_CMARx register
 *
 *		• The post-decrementing of the DMA_CNDTRx register, which contains the number of
 *			transactions that have still to be performed.
 *
 *			StdPeriph:
 *			DMA_CNDTRx <-> DMA_InitStruct->DMA_BufferSize;
 *	-----------------------------------------------------------------------------
 *	DMA section, "Channel configuration procedure", p. 278
 *	-----------------------------------------------------------------------------
 *	DMA section, "Interrupts", p. 280
 *
 *		DMA interrupt requests:
 *			• Half-transfer 		HTIF (event flag); 	HTIE (enable control bit)
 *			• Transfer complete 	TCIF (event flag);	TCIE (enable control bit)
 *			• Transfer error 		TEIF (event flag);	TEIE (enable control bit)
 *	-----------------------------------------------------------------------------
 *	DMA section, "Table 78. Summary of DMA1 requests for each channel", p. 282
 *	-----------------------------------------------------------------------------
 *	USART section, "Data register (USART_DR)"
 *			• Contains the Received or Transmitted data character, depending on whether it is read from
 *				or written to.
 *	-----------------------------------------------------------------------------
 *
 *	-----------------------------------------------------------------------------
 */

#endif /* INCLUDE_PERIPHERALS_USART_USART_DMA_H_ */
