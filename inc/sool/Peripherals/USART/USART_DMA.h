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
//#include "stm32f10x_dma.h"
#include "misc.h"

// C libs
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// SOOL
#include <sool/Peripherals/DMA/DMA_common.h>
#include <sool/Peripherals/DMA/DMA.h>
#include <sool/Memory/String/String.h>

// - - - - - - - - - - - - - - - -

/* IMPORTANT NOTE: beware of breakpoints during debugging - DMA is very sensitive to some interrupts
 * during its job - getting into a breakpoint blocks and never releases an initialized transfer */

// - - - - - - - - - - - - - - - -

struct _SOOL_USART_DMA_State {
};

// - - - - - - - - - - - - - - - -

/* USART coupled with DMA configuration */
struct _SOOL_USART_DMA_Config {
	USART_TypeDef*					USARTx;
	uint8_t							IRQn;
	uint32_t						BUF_INIT_SIZE;
} ;

// - - - - - - - - - - - - - - - -

struct _SOOL_USART_Rx {
	SOOL_String 			buffer;
	uint8_t 				new_data_flag;
} USART_Rx;

// - - - - - - - - - - - - - - - -

struct _SOOL_USART_Tx {
	SOOL_String 		buffer;
	uint8_t 			prepping_request;
} USART_Tx;

// - - - - - - - - - - - - - - - -

/* Forward declaration */
struct _SOOL_USART_DMA_Struct;
typedef struct _SOOL_USART_DMA_Struct SOOL_USART_DMA;

// - - - - - - - - - - - - - - - -

/* USART_DMA `class` */
struct _SOOL_USART_DMA_Struct {

	// --------------------------------------------------
	SOOL_DMA 						base_dma_rx;
	SOOL_DMA 						base_dma_tx;

	// --------------------------------------------------
	struct _SOOL_USART_DMA_Config 	_setup;
	struct _SOOL_USART_DMA_State	_state; // temporarily empty
	struct _SOOL_USART_Rx			_rx;
	struct _SOOL_USART_Tx			_tx;

	/* NVIC section */
	void 	(*EnableNVIC)(volatile SOOL_USART_DMA*);
	void 	(*DisableNVIC)(volatile SOOL_USART_DMA*);

	/* RX section */
	void	(*ActivateReading)(volatile SOOL_USART_DMA*); 			// restarts reading starting from first buffer element
	void	(*DeactivateReading)(volatile SOOL_USART_DMA*); 			// disables DMA and USART idle interrupts
	uint8_t (*IsDataReceived)(volatile SOOL_USART_DMA*); 				// returns info whether data was received - based on USART Idle line detection
	const volatile SOOL_String* (*GetRxData)(volatile SOOL_USART_DMA*); // returns a pointer to a buffer - IMPORTANT: use this method instead of raw ArrayString operations because some calculations are performed here (it is not possible to count number of bytes read from DMA on the fly)
	size_t 	(*GetRxDataLength)(volatile SOOL_USART_DMA*);			// how many valid characters are in the buffer (do not confuse with the buffer length); uses C's `strlen` function
	void	(*ClearRxBuffer)(volatile SOOL_USART_DMA*); 				// clears whole buffer (NOTE: does not set incoming data pointer to the buffer's start)

	/// \brief Manages buffer content so the next part (stage) of the `dataframe` will be
	/// properly located in the internal RX buffer.
	/// \param USART DMA instance
	/// \return 1 if operation was successful
	/// \details Clears RX buffer, restores its initial state,
	/// reconfigures DMA RX Channel so the data will be written
	/// to the proper memory addresses (considers the current
	/// RX buffer start address).
	/// This acts as a confirmation that the data received so far
	/// were already processed and the app is ready to receive a new dataframe.
	uint8_t (*ConfirmReception)(volatile SOOL_USART_DMA*);

	/**
	 * @brief Function invoked inside interrupt handler only when `Transfer Complete` flag is set
	 * @param
	 * @return
	 */
	uint8_t (*_DmaRxIrqHandler)(volatile SOOL_USART_DMA*); 			// interrupt callback function which needs to be put into global DMA IRQHandler

	/* TX section */
//	uint8_t (*IsTxQueueEmpty)(volatile SOOL_USART_DMA*);				// returns info whether TX queue is empty
	uint8_t (*IsTxLineBusy)(volatile SOOL_USART_DMA*); 				// returns info whether TX DMA is currently working

	uint8_t (*Send)(volatile SOOL_USART_DMA*, const char*); 		// blocking call - if DMA_TX is not busy - copies given data into buffer and fires up the transfer
	void	(*ClearTxBuffer)(volatile SOOL_USART_DMA*); 				// clears whole buffer
	/**
	 * @brief Function invoked inside interrupt handler only when `Transfer Complete` flag is set
	 * @param
	 * @return
	 */
	uint8_t (*_DmaTxIrqHandler)(volatile SOOL_USART_DMA*); 			// interrupt callback function which needs to be put into global DMA IRQHandler

	/* General */
	uint8_t (*_IdleLineIrqHandler)(volatile SOOL_USART_DMA*); 			// interrupt callback function which needs to be put into global USART IRQHandler

	/// \brief Restores initial size of buffer(s)
	/// \param USART DMA instance
	/// \param 0 if RX buffer must be modified, 1 - if TX, 2 - if both of them
	/// \return 0 if operation was not successful
	/// \details Brings back the initial size of buffers by reallocating
	/// memory (only if buffer's length is actually bigger than initial size)
	uint8_t	(*RestoreBuffersInitialSize)(volatile SOOL_USART_DMA*, uint8_t);

	void 	(*Destroy)(volatile SOOL_USART_DMA*);						// frees memory taken by buffers, stops USART and DMA (USART_DMA instance needs re-initialization then)

};

// - - - - - - - - - - - - - - - -

// NVICs of USART, DMA_RX, DMA_TX must be enabled separately
extern volatile SOOL_USART_DMA SOOL_Periph_USART_DMA_Init(USART_TypeDef* USARTx, uint32_t baud, size_t buf_size);

/**
 * @brief USART_DMA startup routine
 * @param usart_ptr
 * @note Remember to copy the object into proper IRQHandlers
 */
extern void SOOL_Periph_USART_DMA_Startup(volatile SOOL_USART_DMA* usart_ptr);

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
