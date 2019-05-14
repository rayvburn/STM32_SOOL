/*
 * USART_DMA.c
 *
 *  Created on: 26.04.2019
 *      Author: user
 */

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#include "sool/Peripherals/USART/USART_DMA.h"
#include <string.h> 								// strcpy()

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/* When RX buffer detected being too small then increment
 * its size by this value (in bytes) */
#define USART_DMA_RX_BUFFER_INCREMENT 	2

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/* Methods */
// RX
static void 	USART_DMA_ActivateReading(volatile SOOL_USART_DMA *usart);
static void 	USART_DMA_DeactivateReading(volatile SOOL_USART_DMA *usart);
static uint8_t 	USART_DMA_IsDataReceived(volatile SOOL_USART_DMA *usart);
static const volatile Array_String* USART_DMA_GetRxData(volatile SOOL_USART_DMA *usart);
static void		USART_DMA_ClearRxBuffer(volatile SOOL_USART_DMA *usart);

// TX
static uint8_t 	USART_DMA_IsTxLineBusy(volatile SOOL_USART_DMA *usart);
static uint8_t 	USART_DMA_Send(volatile SOOL_USART_DMA *usart, char *to_send_buf);
static void		USART_DMA_ClearTxBuffer(volatile SOOL_USART_DMA *usart);

// IRQ
static uint8_t 	USART_DMA_RxInterruptHandler(volatile SOOL_USART_DMA *usart);
static uint8_t 	USART_DMA_TxInterruptHandler(volatile SOOL_USART_DMA *usart);
static uint8_t 	USART_DMA_IdleInterruptHandler(volatile SOOL_USART_DMA *usart);

// General
static uint8_t	USART_DMA_RestoreBuffersInitialSize(volatile SOOL_USART_DMA *usart);
static void		USART_DMA_Destroy(volatile SOOL_USART_DMA *usart);

// private class function
static void 	USART_DMA_SetupAndStartDmaReading(volatile SOOL_USART_DMA *usart, const size_t buf_start_pos, const size_t num_bytes_to_read);
static void 	USART_DMA_RestartReading(volatile SOOL_USART_DMA *usart, const size_t buf_start_pos, const size_t num_bytes_to_read);
static uint16_t USART_DMA_GetMaxNonEmptyItemIndex(volatile Array_String *arr);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/* Helpers */
typedef struct {
	DMA_Channel_TypeDef* 	  		channel;
	uint8_t					  		irqn;
	struct _SOOL_DMA_InterruptFlags int_flags;
} USART_DMA_SettingsHelper;

// private non-class function
static void SOOL_Periph_USART_DMA_Copy(volatile SOOL_USART_DMA *usart, const USART_DMA_SettingsHelper *rx_settings, const USART_DMA_SettingsHelper *tx_settings);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

volatile SOOL_USART_DMA SOOL_Periph_USART_DMA_Init(USART_TypeDef* USARTx, uint32_t baud, size_t buf_size) {

	/* GPIO setup */
	GPIO_TypeDef* port;
	uint16_t rx_pin, tx_pin;

	/* NVIC IRQn */
	uint8_t irqn;

	/* Helper structs to store the DMA configuration in a clearer way */
	USART_DMA_SettingsHelper dma_rx;
	USART_DMA_SettingsHelper dma_tx;

	/* Create a new USART_DMA object and save initial buffers' size */
	volatile  SOOL_USART_DMA usart_obj;
	usart_obj._setup.BUF_INIT_SIZE = buf_size;

	/* Initialize the peripheral's RX and TX buffers
	 * with an arbitrary length */
	usart_obj._rx.buffer = SOOL_Memory_Array_String_Init(buf_size);
	usart_obj._tx.buffer = SOOL_Memory_Array_String_Init(buf_size);

	/* TODO: Remap handling, USART could go to PD/PC when remapped
	 * Reference Manual, p. 183 - AFIO_MAPR */
	/* port clock */
	if ( USARTx == USART1 || USARTx == USART2 ) {
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
		port = GPIOA;
	} else if ( USARTx == USART3 ) {
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
		port = GPIOB;
	}

	/* clock - alternative function */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,  ENABLE);

	(USARTx == USART1) ? (RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE)) : (0);
	(USARTx == USART2) ? (RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE)) : (0);
	(USARTx == USART3) ? (RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE)) : (0);

	/* Enable clock for the DMA peripheral (ref manual, p. 283) -
	 * RCC_AHBPeriph_DMA1 is valid for USART1, USART2, USART3 */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

	/* TODO: Remap handling */
	if ( USARTx == USART1 ) {

		// GPIO and IRQn
		tx_pin = GPIO_Pin_9;
		rx_pin = GPIO_Pin_10;
		irqn = USART1_IRQn;

		// TX
		dma_tx.channel = DMA1_Channel4;
		dma_tx.irqn = DMA1_Channel4_IRQn;
		dma_tx.int_flags.COMPLETE_FLAG = DMA1_FLAG_TC4;
		dma_tx.int_flags.ERROR_FLAG = DMA1_FLAG_TE4;
		dma_tx.int_flags.HALF_FLAG = DMA1_FLAG_HT4;
		dma_tx.int_flags.GLOBAL_FLAG = DMA1_FLAG_GL4;

		// RX
		dma_rx.channel = DMA1_Channel5;
		dma_rx.irqn = DMA1_Channel5_IRQn;
		dma_rx.int_flags.COMPLETE_FLAG = DMA1_FLAG_TC5;
		dma_rx.int_flags.ERROR_FLAG = DMA1_FLAG_TE5;
		dma_rx.int_flags.HALF_FLAG = DMA1_FLAG_HT5;
		dma_rx.int_flags.GLOBAL_FLAG = DMA1_FLAG_GL5;

	} else if ( USARTx == USART2 ) {

		// GPIO and IRQn
		tx_pin = GPIO_Pin_2;
		rx_pin = GPIO_Pin_3;
		irqn = USART2_IRQn;

		// TX
		dma_tx.channel = DMA1_Channel7;
		dma_tx.irqn = DMA1_Channel7_IRQn;
		dma_tx.int_flags.COMPLETE_FLAG = DMA1_FLAG_TC7;
		dma_tx.int_flags.ERROR_FLAG = DMA1_FLAG_TE7;
		dma_tx.int_flags.HALF_FLAG = DMA1_FLAG_HT7;
		dma_tx.int_flags.GLOBAL_FLAG = DMA1_FLAG_GL7;

		// RX
		dma_rx.channel = DMA1_Channel6;
		dma_rx.irqn = DMA1_Channel6_IRQn;
		dma_rx.int_flags.COMPLETE_FLAG = DMA1_FLAG_TC6;
		dma_rx.int_flags.ERROR_FLAG = DMA1_FLAG_TE6;
		dma_rx.int_flags.HALF_FLAG = DMA1_FLAG_HT6;
		dma_rx.int_flags.GLOBAL_FLAG = DMA1_FLAG_GL6;

	} else if ( USARTx == USART3 ) {

		// GPIO
		tx_pin = GPIO_Pin_10;
		rx_pin = GPIO_Pin_11;

		/* Check remapping
		if ( AFIO->MAPR & AFIO_MAPR_USART3_REMAP_FULLREMAP ) {
		} else if ( AFIO->MAPR & AFIO_MAPR_USART3_REMAP_PARTIALREMAP ) {
		} else if ( (AFIO->MAPR | AFIO_MAPR_USART3_REMAP_NOREMAP) == 0 ) {
		}
		*/

		// IRQn
		irqn = USART3_IRQn;

		// TX
		dma_tx.channel = DMA1_Channel2;
		dma_tx.irqn = DMA1_Channel2_IRQn;
		dma_tx.int_flags.COMPLETE_FLAG = DMA1_FLAG_TC2;
		dma_tx.int_flags.ERROR_FLAG = DMA1_FLAG_TE2;
		dma_tx.int_flags.HALF_FLAG = DMA1_FLAG_HT2;
		dma_tx.int_flags.GLOBAL_FLAG = DMA1_FLAG_GL2;

		// RX
		dma_rx.channel = DMA1_Channel3;
		dma_rx.irqn = DMA1_Channel3_IRQn;
		dma_rx.int_flags.COMPLETE_FLAG = DMA1_FLAG_TC3;
		dma_rx.int_flags.ERROR_FLAG = DMA1_FLAG_TE3;
		dma_rx.int_flags.HALF_FLAG = DMA1_FLAG_HT3;
		dma_rx.int_flags.GLOBAL_FLAG = DMA1_FLAG_GL3;

	}

	/* Init GPIO pins */
	GPIO_InitTypeDef gpio;
	GPIO_StructInit(&gpio);
	gpio.GPIO_Pin = rx_pin;		// RX
	gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(port, &gpio);

	gpio.GPIO_Pin = tx_pin;		// TX
	gpio.GPIO_Mode = GPIO_Mode_AF_PP;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(port, &gpio);

	/* Init USART */
	USART_InitTypeDef usart;
	USART_StructInit(&usart);
	usart.USART_BaudRate = baud;
	USART_Init(USARTx, &usart);

	/* Useful in pure RX/TX interrupts mode */
//	USART_ITConfig(usart_periph_id, USART_IT_RXNE, int_rx_en);
//	USART_ITConfig(usart_periph_id, USART_IT_TXE,  int_tx_en);

	/* Enable interrupt controller for USART (NVIC) */
	NVIC_InitTypeDef nvic;
	nvic.NVIC_IRQChannel = irqn;
	nvic.NVIC_IRQChannelPreemptionPriority = 0;
	nvic.NVIC_IRQChannelSubPriority = 1;
	nvic.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvic);

	/* Enable peripheral */
	USART_Cmd(USARTx, ENABLE);

	/* IDLE line protection - enabled by ActivateReading() */
//	USART_ITConfig(USARTx, USART_IT_IDLE, ENABLE);

	/* DMA configuration structure */
	DMA_InitTypeDef dma;

	/* DMA RX Channel */
	DMA_DeInit(dma_rx.channel);                              	// clear DMA configuration
	DMA_ClearFlag(dma_rx.int_flags.ERROR_FLAG | dma_rx.int_flags.GLOBAL_FLAG |	// clear interrupt flags
				  dma_rx.int_flags.HALF_FLAG  | dma_rx.int_flags.COMPLETE_FLAG);

	dma.DMA_PeripheralBaseAddr = (uint32_t)&USARTx->DR;  		// peripheral's data register address

	// to be specified before first transfer
	dma.DMA_MemoryBaseAddr = (uint32_t)&usart_obj._rx.buffer._data; // data block to be send initial address
	dma.DMA_DIR = DMA_DIR_PeripheralSRC;                    	// transfer direction
	dma.DMA_BufferSize = 1;                              		// temporary value of a buffer's length (number of items to send)
	dma.DMA_PeripheralInc = DMA_PeripheralInc_Disable;      	// auto-increment of address (peripheral's side)
	dma.DMA_MemoryInc = DMA_MemoryInc_Enable;               	// auto-increment of address (buffer's side)
	dma.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;	// data size (peripheral)
	dma.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;         	// data_size (buffer)
	dma.DMA_Mode = DMA_Mode_Normal;                             // mode
	dma.DMA_Priority = DMA_Priority_VeryHigh;                   // priority
	dma.DMA_M2M = DMA_M2M_Disable;                              // memory to memory setting
	DMA_Cmd(dma_rx.channel, DISABLE);							// https://stackoverflow.com/questions/23576241/stm32-dma-transfer-error
	DMA_Init(dma_rx.channel, &dma);								// save the configuration

	/* DMA TX Channel */
	DMA_DeInit(dma_tx.channel);                              	// clear DMA configuration
	DMA_ClearFlag(dma_tx.int_flags.ERROR_FLAG | dma_tx.int_flags.GLOBAL_FLAG | 	// clear interrupt flags
				  dma_tx.int_flags.HALF_FLAG  | dma_tx.int_flags.COMPLETE_FLAG);

	dma.DMA_PeripheralBaseAddr = (uint32_t)&USARTx->DR;  		// peripheral's data register address

	// to be specified before a first transfer
	dma.DMA_MemoryBaseAddr = (uint32_t)&usart_obj._tx.buffer._data; // data block to be send as the initial address
	dma.DMA_DIR = DMA_DIR_PeripheralDST;                    	// transfer direction
	dma.DMA_BufferSize = 1;                              		// temporary value of a buffer's length (number of items to send)
	dma.DMA_PeripheralInc = DMA_PeripheralInc_Disable;      	// auto-increment of address (peripheral's side)
	dma.DMA_MemoryInc = DMA_MemoryInc_Enable;               	// auto-increment of address (buffer's side)
	dma.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;	// data size (peripheral)
	dma.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;         	// data_size (buffer)
	dma.DMA_Mode = DMA_Mode_Normal;                             // mode
	dma.DMA_Priority = DMA_Priority_VeryHigh;                   // priority
	dma.DMA_M2M = DMA_M2M_Disable;                              // memory to memory setting
	DMA_Cmd(dma_tx.channel, DISABLE);							// https://stackoverflow.com/questions/23576241/stm32-dma-transfer-error
	DMA_Init(dma_tx.channel, &dma);								// save configuration

	/* Enable global interrupts for DMA */
	nvic.NVIC_IRQChannel = dma_rx.irqn;
	nvic.NVIC_IRQChannelCmd = ENABLE;
	nvic.NVIC_IRQChannelPreemptionPriority = 0;
	nvic.NVIC_IRQChannelSubPriority = 0;
	NVIC_Init(&nvic);

	nvic.NVIC_IRQChannel = dma_tx.irqn;
	nvic.NVIC_IRQChannelCmd = ENABLE;
	nvic.NVIC_IRQChannelPreemptionPriority = 0;
	nvic.NVIC_IRQChannelSubPriority = 0;
	NVIC_Init(&nvic);

	/* Configure the USART DMA interface */
	USART_DMACmd(USARTx, USART_DMAReq_Rx, ENABLE);
	USART_DMACmd(USARTx, USART_DMAReq_Tx, ENABLE);

	/* Turn on transfer complete (TC) and transfer error (TE) interrupts */
	DMA_ITConfig(dma_rx.channel, DMA_IT_TC | DMA_IT_TE, ENABLE);
	DMA_ITConfig(dma_tx.channel, DMA_IT_TC | DMA_IT_TE, ENABLE);

	/* Start transfer */
//	DMA_Cmd(dma_rx.channel, ENABLE);
//	DMA_Cmd(dma_tx.channel, ENABLE);

	/* Save USART peripheral ID in the `class` */
	usart_obj._setup.USARTx = USARTx;

	/* Fill USART object structure's fields */
	SOOL_Periph_USART_DMA_Copy(&usart_obj, &dma_rx, &dma_tx);

	return (usart_obj);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void SOOL_Periph_USART_DMA_Copy(volatile SOOL_USART_DMA *usart, const USART_DMA_SettingsHelper *rx_settings,
		const USART_DMA_SettingsHelper *tx_settings) {

	/* Fill the Setup structure's fields */

	/* RX */
	usart->_setup.dma_rx.DMA_Channelx = rx_settings->channel;
	usart->_setup.dma_rx.int_flags    = rx_settings->int_flags;

	// buffer already assigned in this moment
	usart->_rx.new_data_flag = 0;

	/* TX */
	usart->_setup.dma_tx.DMA_Channelx = tx_settings->channel;
	usart->_setup.dma_tx.int_flags    = tx_settings->int_flags;

	// buffer already assigned in this moment

	/* Fill USART class' methods */
	usart->ActivateReading = USART_DMA_ActivateReading;
	usart->DeactivateReading = USART_DMA_DeactivateReading;
	usart->IsDataReceived = USART_DMA_IsDataReceived;
	usart->GetRxData = USART_DMA_GetRxData;
	usart->ClearRxBuffer = USART_DMA_ClearRxBuffer;
	usart->_DmaRxIrqHandler = USART_DMA_RxInterruptHandler;

	usart->IsTxLineBusy = USART_DMA_IsTxLineBusy;
	usart->Send = USART_DMA_Send;
	usart->ClearTxBuffer = USART_DMA_ClearTxBuffer;
	usart->_DmaTxIrqHandler = USART_DMA_TxInterruptHandler;

	usart->_IdleLineIrqHandler = USART_DMA_IdleInterruptHandler;
	usart->RestoreBuffersInitialSize = USART_DMA_RestoreBuffersInitialSize;
	usart->Destroy = USART_DMA_Destroy;

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void USART_DMA_SetupAndStartDmaReading(volatile SOOL_USART_DMA *usart, const size_t buf_start_pos,
		const size_t num_bytes_to_read) {

	/* Disable DMA Channel*/
	usart->_setup.dma_rx.DMA_Channelx->CCR &= (uint16_t)(~DMA_CCR1_EN); // DMA_Cmd(usart->_setup.dma_rx.DMA_Channelx, DISABLE);

	/* Update peripheral's Data Register address */
	usart->_setup.dma_rx.DMA_Channelx->CPAR = (uint32_t)&usart->_setup.USARTx->DR;

	/* Update memory base register address */
	usart->_setup.dma_rx.DMA_Channelx->CMAR = (uint32_t)&usart->_rx.buffer._data[buf_start_pos];

	/* Change buffer size - bigger data will come in parts */
	usart->_setup.dma_rx.DMA_Channelx->CNDTR = (uint32_t)num_bytes_to_read;

	/* Reset the New Data flag */
	usart->_rx.new_data_flag = 0;

	/* Enable Idle Line interrupt */
	usart->_setup.USARTx->CR1 |= USART_CR1_IDLEIE;

	/* Start DMA Channel's reading */
	usart->_setup.dma_rx.DMA_Channelx->CCR |= DMA_CCR1_EN; // DMA_Cmd(usart->_setup.dma_rx.DMA_Channelx, ENABLE);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void USART_DMA_ActivateReading(volatile SOOL_USART_DMA *usart) {

	/* NOTE: below created as a fix for cluttering buffer with old data issue;
	 * after investigation - switching receiver mode ON and OFF solved that */
//	/* Read USART Data Register to prevent old data (unread)
//	 * to jump into currently collected string;
//	 * this clears RXNE flag (Reference Manual, p.823, Bit 5) */
//	uint8_t byte = usart->_setup.USARTx->DR;
//	usart->_setup.USARTx->SR &= (~USART_SR_RXNE);
//
//	/* Clear DMA TC interrupt bit */
//	DMA_ClearITPendingBit(usart->_setup.dma_rx.int_flags.GLOBAL_FLAG);

	/* Enable USART receiver mode */
	usart->_setup.USARTx->CR1 |= USART_CR1_RE;

	/* Configure and start DMA RX Channel */
	USART_DMA_SetupAndStartDmaReading(usart, 0, (size_t)usart->_rx.buffer._info.capacity);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void USART_DMA_DeactivateReading(volatile SOOL_USART_DMA *usart) {

	/* Disable DMA Channel*/
	usart->_setup.dma_rx.DMA_Channelx->CCR &= (uint16_t)(~DMA_CCR1_EN); // DMA_Cmd(usart->_setup.dma_rx.DMA_Channelx, DISABLE);

	/* Disable Idle Line interrupt */
	usart->_setup.USARTx->CR1 &= (uint16_t)(~USART_CR1_IDLEIE);

	/* Disable USART receiver mode */
	usart->_setup.USARTx->CR1 &= (uint16_t)(~USART_CR1_RE);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void USART_DMA_RestartReading(volatile SOOL_USART_DMA *usart, const size_t buf_start_pos, const size_t num_bytes_to_read) {

	/* This is invoked after TC but not all data was transmitted due to a full RX buffer */
	USART_DMA_SetupAndStartDmaReading(usart, buf_start_pos, num_bytes_to_read);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t USART_DMA_IsDataReceived(volatile SOOL_USART_DMA *usart) {

//	/* Information about new data could be read only once */
//	uint8_t temp = usart->_rx.new_data_flag;
//	usart->_rx.new_data_flag = 0;
//	return (temp);

	/* Information about new data could be read as long as data
	 * were not read by 'GetRxData()' */
	return (usart->_rx.new_data_flag);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static const volatile Array_String* USART_DMA_GetRxData(volatile SOOL_USART_DMA *usart) {

	/* update Array_String info */
	usart->_rx.buffer._info.total = USART_DMA_GetMaxNonEmptyItemIndex(&usart->_rx.buffer) + 1;
	usart->_rx.buffer._info.add_index = usart->_rx.buffer._info.total;
	usart->_rx.new_data_flag = 0;
	return (&usart->_rx.buffer);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint16_t USART_DMA_GetMaxNonEmptyItemIndex(volatile Array_String *arr) {

	/* Array is filled with 0-s by default, find non-zero value with the biggest index;
	 * it could not be done on the fly because of use of DMA */
	for ( size_t i = (arr->_info.capacity - 1); i >= 0; i-- ) {
		if ( arr->_data[i] != 0 ) {
			return (i);
		}
	}
	return (0);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void	USART_DMA_ClearRxBuffer(volatile SOOL_USART_DMA *usart) {
	usart->_rx.buffer.Clear(&usart->_rx.buffer);
	usart->_rx.new_data_flag = 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t USART_DMA_RxInterruptHandler(volatile SOOL_USART_DMA *usart) {

	/* NOTE on interrupts: GLOBAL_FLAG clears all related interrupts i.e. TC, TE, HT;
	 * it may be a little faster to clear GLOBAL_FLAG at once than clearing 2 or more
	 * of them (with TC flag there is a HT flag active too) */

	if ( DMA_GetITStatus(usart->_setup.dma_rx.int_flags.ERROR_FLAG) == SET ) {

		/* Transfer Error Interrupt */
		DMA_ClearITPendingBit(usart->_setup.dma_rx.int_flags.ERROR_FLAG);
//		DMA_ClearITPendingBit(usart->_setup.dma_rx.int_flags.GLOBAL_FLAG);
		// TODO: some error message

		return (1);

	} else if ( DMA_GetITStatus(usart->_setup.dma_rx.int_flags.COMPLETE_FLAG) == SET ) {

		/* Transfer Complete Interrupt */
//		DMA_ClearITPendingBit(usart->_setup.dma_rx.int_flags.COMPLETE_FLAG);
		DMA_ClearITPendingBit(usart->_setup.dma_rx.int_flags.GLOBAL_FLAG);

		/* Check whether RX line is idle - if yes then disable DMA RX Channel,
		 * otherwise resize the buffer as it is full */
		usart->_setup.dma_rx.DMA_Channelx->CCR &= (uint16_t)(~DMA_CCR1_EN); // DMA_Cmd(usart->_setup.dma_rx.DMA_Channelx, DISABLE);

		/* Disable USART Idle Line Detection (no need to do that after successful flag clearance) */
//		usart->_setup.USARTx->CR1 &= (uint16_t)(~USART_CR1_IDLEIE); // equal to USART_ITConfig(usart->_setup.USARTx, USART_IT_IDLE, DISABLE);

		/* Since TC interrupt was triggered then:
		 * 	- 	number of data to receive was known in advance (no way to tell if it is that case or the next),
		 * 	-	there are still some data to receive (buffer must be resized)
		 */

		/* The buffer must be resized in both cases - make it a little bigger */
		if ( !usart->_rx.buffer.Resize(&usart->_rx.buffer, (size_t)(usart->_rx.buffer._info.capacity + USART_DMA_RX_BUFFER_INCREMENT)) ) {
			// FIXME: unable to resize - print some error
		}

		/* Try to continue reading allowing the message to be continuously
		 * written into buffer (this is an ideal case)
		 * NOTE: sometimes resizing takes too long and data is lost (or maybe it was just a debugging method issue) */
		USART_DMA_RestartReading(usart, (size_t)(usart->_rx.buffer._info.capacity - USART_DMA_RX_BUFFER_INCREMENT), USART_DMA_RX_BUFFER_INCREMENT);

		return (1);

	} else if ( DMA_GetITStatus(usart->_setup.dma_rx.int_flags.HALF_FLAG) == SET ) {

		/* Half Transfer Interrupt */
//		DMA_ClearITPendingBit(usart->_setup.dma_rx.int_flags.HALF_FLAG);
		DMA_ClearITPendingBit(usart->_setup.dma_rx.int_flags.GLOBAL_FLAG);
		return (1);

	} else if ( DMA_GetITStatus(usart->_setup.dma_rx.int_flags.GLOBAL_FLAG) == SET ) {

		/* Global DMA Channel Interrupt */
		DMA_ClearITPendingBit(usart->_setup.dma_rx.int_flags.GLOBAL_FLAG);
		return (1);

	} else {

		// indicate that no interrupt flag was recognized and cleared
		return (0);
	}

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t USART_DMA_IdleInterruptHandler(volatile SOOL_USART_DMA *usart) {

	if ( USART_GetITStatus(usart->_setup.USARTx, USART_IT_IDLE) == SET ) {

//		USART_ClearITPendingBit(usart->_setup.USARTx, USART_IT_IDLE); // CAUTION: does not clear IT flag!
		/* Clear IDLE Line IT flag, Ref manual, p. 824, Bit 4 - "by a software sequence (an read
		 * to the USART_SR register followed by a read to the USART_DR register";
		 * line is IDLE so no data SHOULD be lost by reading a DR */
		usart->_setup.USARTx->SR;
		usart->_setup.USARTx->DR;

		/* NOTE: useful when interrupts generated all the time - this part
		 * is a leftover from an unsuccessful IT flag clearance */
//		usart->_setup.USARTx->CR1 &= (uint16_t)(~USART_CR1_IDLEIE);

		if ( usart->_setup.dma_rx.DMA_Channelx->CCR & DMA_CCR1_EN ) {
			/* reading is active and Idle Line detected */
			/* EN bit check is used to avoid the first interrupt of an Idle Line issue
			 * (triggers just after enabling the IT_IDLE) */
			usart->_rx.new_data_flag = 1;
		}
		return (1);

	}

	return (0);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t USART_DMA_IsTxLineBusy(volatile SOOL_USART_DMA *usart) {

	/* Reach information whether DMA transfer was started (usart->_tx.started_flag is DEPRECATED);
	 * a DMA EN bit raw check is sufficient - it is known in advance that USARTx uses DMA1 */
	return (usart->_setup.dma_tx.DMA_Channelx->CCR | DMA_CCR1_EN);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void	USART_DMA_ClearTxBuffer(volatile SOOL_USART_DMA *usart) {
	usart->_tx.buffer.Clear(&usart->_tx.buffer);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t USART_DMA_TxInterruptHandler(volatile SOOL_USART_DMA *usart) {

	/* NOTE on interrupts: GLOBAL_FLAG clears all related interrupts i.e. TC, TE, HT;
	 * it may be a little faster to clear GLOBAL_FLAG at once than clearing 2 or more
	 * of them (with TC flag there is a HT flag active too) */

	if ( DMA_GetITStatus(usart->_setup.dma_tx.int_flags.ERROR_FLAG) == SET ) {

		/* Transfer Error Interrupt - happens for example when:
		 * 	- 	the peripheral register's address is wrong
		 *	- 	data couldn't be pushed due to the full buffer (RX-TX of MCU transfer)
		 */
//		DMA_ClearITPendingBit(usart->_setup.dma_tx.int_flags.ERROR_FLAG);
		DMA_ClearITPendingBit(usart->_setup.dma_tx.int_flags.GLOBAL_FLAG);
		// TODO: some error message

		return (1);

	} else if ( DMA_GetITStatus(usart->_setup.dma_tx.int_flags.COMPLETE_FLAG) == SET ) {

		/* Transfer Complete Interrupt */
//		DMA_ClearITPendingBit(usart->_setup.dma_tx.int_flags.COMPLETE_FLAG);
		DMA_ClearITPendingBit(usart->_setup.dma_tx.int_flags.GLOBAL_FLAG);

		/* Disable DMA Channel */
		usart->_setup.dma_tx.DMA_Channelx->CCR &= (uint16_t)(~DMA_CCR1_EN);// DMA_Cmd(usart->_setup.dma_tx.DMA_Channelx, DISABLE);

		return (1);

	} else if ( DMA_GetITStatus(usart->_setup.dma_tx.int_flags.HALF_FLAG) == SET ) {

		/* Half Transfer Interrupt */
//		DMA_ClearITPendingBit(usart->_setup.dma_tx.int_flags.HALF_FLAG);
		DMA_ClearITPendingBit(usart->_setup.dma_tx.int_flags.GLOBAL_FLAG);
		return (1);

	} else if ( DMA_GetITStatus(usart->_setup.dma_tx.int_flags.GLOBAL_FLAG) == SET ) {

		/* Global DMA Channel Interrupt */
		DMA_ClearITPendingBit(usart->_setup.dma_tx.int_flags.GLOBAL_FLAG);
		return (1);

	} else {

		// indicate that no interrupt flag was recognized and cleared
		return (0);
	}

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t USART_DMA_Send(volatile SOOL_USART_DMA *usart, char *to_send_buf) {

	/* Check message length */
	uint32_t length = (uint32_t)strlen(to_send_buf);

	/* Decide whether reallocation is a must */
	if ( length > usart->_tx.buffer._info.capacity ) {

		/* `Resize` method discards USART's volatile qualifier - not crucial here */
		if ( !usart->_tx.buffer.Resize(&usart->_tx.buffer, (size_t)length) ) {
			// FIXME: error message
			//perror("COULD NOT REALLOCATE AN ARRAY");
		}

	}

	/* Try to copy contents of the string to be sent */
	strcpy(usart->_tx.buffer._data, to_send_buf); // beware of `&`, this is not a typical array

	/* Disable DMA Channel*/
	usart->_setup.dma_tx.DMA_Channelx->CCR &= (uint16_t)(~DMA_CCR1_EN);// DMA_Cmd(usart->_setup.dma_tx.DMA_Channelx, DISABLE);

	/* Update peripheral's Data Register address */
	usart->_setup.dma_tx.DMA_Channelx->CPAR = (uint32_t)&usart->_setup.USARTx->DR;

	/* Update memory base register address */
	usart->_setup.dma_tx.DMA_Channelx->CMAR = (uint32_t)usart->_tx.buffer._data; // beware of `&`, this is not a typical array

	/* Change the buffer's size */
	/* Ref: `can only be written when the channel is disabled` */
	usart->_setup.dma_tx.DMA_Channelx->CNDTR = length;

	/* Start DMA Channel's transfer */
	usart->_setup.dma_tx.DMA_Channelx->CCR |= DMA_CCR1_EN;// DMA_Cmd(usart->_setup.dma_tx.DMA_Channelx, ENABLE);

	/* Return 1 on successful sent */
	return (1);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t USART_DMA_RestoreBuffersInitialSize(volatile SOOL_USART_DMA *usart) {

	/* `Resize` method discards USART's volatile qualifier - not crucial here */
	if ( usart->_rx.buffer._info.capacity > usart->_setup.BUF_INIT_SIZE) {
		/* Check whether buffer's capacity increased */
		if ( !usart->_rx.buffer.Resize(&usart->_rx.buffer, (size_t)usart->_setup.BUF_INIT_SIZE) ) {
			return (0);
		}
	}

	if ( usart->_tx.buffer._info.capacity > usart->_setup.BUF_INIT_SIZE) {
		/* Check whether buffer's capacity increased */
		if ( !usart->_tx.buffer.Resize(&usart->_tx.buffer, (size_t)usart->_setup.BUF_INIT_SIZE) ) {
			return (0);
		}
	}

	return (1);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void	USART_DMA_Destroy(volatile SOOL_USART_DMA *usart) {

	/* Disable DMA RX Channel */
	usart->_setup.dma_rx.DMA_Channelx->CCR &= (uint16_t)(~DMA_CCR1_EN);

	/* Disable DMA TX Channel */
	usart->_setup.dma_tx.DMA_Channelx->CCR &= (uint16_t)(~DMA_CCR1_EN);

	/* Disable USART_DMA */
	usart->_setup.USARTx->CR1 &= (uint16_t)(~USART_CR1_UE);

	/* Deinit DMA Channel */
	DMA_DeInit(usart->_setup.dma_tx.DMA_Channelx);

	/* Deinit USART */
	USART_DeInit(usart->_setup.USARTx);

	/* Free memory taken by buffers */
//	usart->_rx.buffer.Free(&usart->_rx.buffer); // Infinite loop
//	usart->_tx.buffer.Free(&usart->_tx.buffer); // Infinite loop
	usart->_rx.buffer.Resize(&usart->_rx.buffer, 0);
	usart->_tx.buffer.Resize(&usart->_tx.buffer, 0);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
