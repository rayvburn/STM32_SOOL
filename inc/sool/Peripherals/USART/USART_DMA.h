/*
 * USART_DMA.h
 *
 *  Created on: 26.04.2019
 *      Author: user
 */

#ifndef INCLUDE_PERIPHERALS_USART_USART_DMA_H_
#define INCLUDE_PERIPHERALS_USART_USART_DMA_H_

// ST libs
#include <sool/Memory/Array/ArrayChar.h>
#include "sool/Peripherals/DMA/Common.h"

#include "stm32f10x.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_dma.h"
#include "misc.h"

// C libs
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// SOOL

// - - - - - - - - - - - - - - - -

/* IMPORTANT NOTE: beware of breakpoints during debugging - DMA is very sensitive to some interrupts
 * during its job - getting into a breakpoint blocks and never releases an initialized transfer */

// - - - - - - - - - - - - - - - -

/* USART coupled with DMA configuration */
struct _SOOL_USART_DMA_Config {
	USART_TypeDef*		USARTx;
	SOOL_DMA_ChannelConfig 	dma_rx;
	SOOL_DMA_ChannelConfig 	dma_tx;
	uint32_t			BUF_INIT_SIZE;
} ;

// - - - - - - - - - - - - - - - -

struct _SOOL_USART_Rx {
	SOOL_Array_Char 		buffer;
	uint8_t 			new_data_flag;
} USART_Rx;

// - - - - - - - - - - - - - - - -

struct _SOOL_USART_Tx {
	SOOL_Array_Char 		buffer;
} USART_Tx;

// - - - - - - - - - - - - - - - -

/* Forward declaration */
struct _SOOL_USART_DMA_Struct;
typedef struct _SOOL_USART_DMA_Struct SOOL_USART_DMA;

// - - - - - - - - - - - - - - - -

/* USART_DMA `class` */
struct _SOOL_USART_DMA_Struct {

	struct _SOOL_USART_DMA_Config 	_setup;
	struct _SOOL_USART_Rx			_rx;
	struct _SOOL_USART_Tx			_tx;

	// RX section
	void	(*ActivateReading)(volatile SOOL_USART_DMA*); 			// restarts reading starting from first buffer element
	void	(*DeactivateReading)(volatile SOOL_USART_DMA*); 			// disables DMA and USART idle interrupts
	uint8_t (*IsDataReceived)(volatile SOOL_USART_DMA*); 				// returns info whether data was received - based on USART Idle line detection
	const volatile SOOL_Array_Char* (*GetRxData)(volatile SOOL_USART_DMA*); // returns a pointer to a buffer - IMPORTANT: use this method instead of raw ArrayString operations because some calculations are performed here (it is not possible to count number of bytes read from DMA on the fly)
	void	(*ClearRxBuffer)(volatile SOOL_USART_DMA*); 				// clears whole buffer (NOTE: does not set incoming data pointer to the buffer's start)
	uint8_t (*_DmaRxIrqHandler)(volatile SOOL_USART_DMA*); 			// interrupt callback function which needs to be put into global DMA IRQHandler

	// TX section
	uint8_t (*IsTxLineBusy)(volatile SOOL_USART_DMA*); 				// returns info whether TX DMA is currently working
	uint8_t (*Send)(volatile SOOL_USART_DMA*, char*); 				// copies given data into buffer and fires up the transfer
	void	(*ClearTxBuffer)(volatile SOOL_USART_DMA*); 				// clears whole buffer
	uint8_t (*_DmaTxIrqHandler)(volatile SOOL_USART_DMA*); 			// interrupt callback function which needs to be put into global DMA IRQHandler

	// General
	uint8_t (*_IdleLineIrqHandler)(volatile SOOL_USART_DMA*); 			// interrupt callback function which needs to be put into global USART IRQHandler
	uint8_t	(*RestoreBuffersInitialSize)(volatile SOOL_USART_DMA*); 	// brings back the initial size of buffers by reallocating memory (only if buffer's length is actually bigger than initial size)
	void 	(*Destroy)(volatile SOOL_USART_DMA*);						// frees memory taken by buffers, stops USART and DMA (USART_DMA instance needs re-initialization then)

};

// - - - - - - - - - - - - - - - -

volatile SOOL_USART_DMA SOOL_Periph_USART_DMA_Init(USART_TypeDef* USARTx, uint32_t baud, size_t buf_size);

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
 *		� The loading of data from the peripheral data register or a location in memory addressed
 *			through an internal current peripheral/memory address register. The start address used
 *			for the first transfer is the base peripheral/memory address programmed in the
 *			DMA_CPARx or DMA_CMARx register
 *
 *			StdPeriph:
 *			DMA_CPARx <-> DMA_InitStruct->DMA_PeripheralBaseAddr
 *			DMA_CMARx <-> DMA_InitStruct->DMA_MemoryBaseAddr
 *
 *		� The storage of the data loaded to the peripheral data register or a location in memory
 *			addressed through an internal current peripheral/memory address register. The start
 *			address used for the first transfer is the base peripheral/memory address programmed
 *			in the DMA_CPARx or DMA_CMARx register
 *
 *		� The post-decrementing of the DMA_CNDTRx register, which contains the number of
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
 *			� Half-transfer 		HTIF (event flag); 	HTIE (enable control bit)
 *			� Transfer complete 	TCIF (event flag);	TCIE (enable control bit)
 *			� Transfer error 		TEIF (event flag);	TEIE (enable control bit)
 *	-----------------------------------------------------------------------------
 *	DMA section, "Table 78. Summary of DMA1 requests for each channel", p. 282
 *	-----------------------------------------------------------------------------
 *	USART section, "Data register (USART_DR)"
 *			� Contains the Received or Transmitted data character, depending on whether it is read from
 *				or written to.
 *	-----------------------------------------------------------------------------
 *
 *	-----------------------------------------------------------------------------
 */

#endif /* INCLUDE_PERIPHERALS_USART_USART_DMA_H_ */