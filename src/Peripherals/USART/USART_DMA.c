/*
 * USART_DMA.c
 *
 *  Created on: 26.04.2019
 *      Author: user
 */

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#include <include/Peripherals/USART/USART_DMA.h>
#include <string.h> 								// strcpy()
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// RX
static void 	USART_DMA_ActivateReading(volatile USART_DMA_Periph *usart);
static uint8_t 	USART_DMA_IsDataReceived(volatile USART_DMA_Periph *usart);
static const Array_String* USART_DMA_GetRxData(volatile USART_DMA_Periph *usart);
static void		USART_DMA_ClearRxBuffer(volatile USART_DMA_Periph *usart);

// TX
static uint8_t 	USART_DMA_IsTxLineBusy(volatile USART_DMA_Periph *usart);
static uint8_t 	USART_DMA_Send(volatile USART_DMA_Periph *usart, char *to_send_buf);
static void		USART_DMA_ClearTxBuffer(volatile USART_DMA_Periph *usart);

static void		USART_DMA_DestroyBuffers(volatile USART_DMA_Periph *usart);

// IRQ
static uint8_t USART_DMA_RxInterruptHandler(volatile USART_DMA_Periph *usart);
static uint8_t USART_DMA_TxInterruptHandler(volatile USART_DMA_Periph *usart);
static uint8_t USART_DMA_IdleInterruptHandler(volatile USART_DMA_Periph *usart);

// private class function
static void USART_DMA_SetupAndStartDmaReading(volatile USART_DMA_Periph *usart, const size_t buf_start_pos);
static void USART_DMA_RestartReading(volatile USART_DMA_Periph *usart, const size_t buf_start_pos);
static uint16_t USART_DMA_GetMaxNonEmptyItemIndex(Array_String *arr);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

typedef struct {
	DMA_Channel_TypeDef* 	channel;
	uint8_t					irqn;
	DMA_InterruptFlags		int_flags;
} USART_DMA_SettingsHelper;

// private non-class function
static void SOOL_USART_DMA_Copy(volatile USART_DMA_Periph *usart, const USART_DMA_SettingsHelper *rx_settings, const USART_DMA_SettingsHelper *tx_settings);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

volatile USART_DMA_Periph SOOL_USART_DMA_Init(USART_TypeDef* USARTx, uint32_t baud, size_t buf_size) {

	/* GPIO setup */
	GPIO_TypeDef* port;
	uint16_t rx_pin, tx_pin;

	/* NVIC IRQn */
	uint8_t irqn;

	/* Helper structs to store the DMA configuration in a clearer way */
	USART_DMA_SettingsHelper dma_rx;
	USART_DMA_SettingsHelper dma_tx;

	/* Create a new USART_DMA object and save initial buffers' size */
	volatile  USART_DMA_Periph usart_obj;
	usart_obj.setup.BUF_INIT_SIZE = buf_size;

	/* Initialize the peripheral's RX and TX buffers
	 * with an arbitrary length */
	usart_obj.rx.buffer = SOOL_Array_String_Init(buf_size);
	usart_obj.tx.buffer = SOOL_Array_String_Init(buf_size);

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

		// GPIO and IRQn
		tx_pin = GPIO_Pin_10;
		rx_pin = GPIO_Pin_11;
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
	dma.DMA_MemoryBaseAddr = (uint32_t)&usart_obj.rx.buffer.data; // data block to be send initial address
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
	dma.DMA_MemoryBaseAddr = (uint32_t)&usart_obj.tx.buffer.data; // data block to be send as the initial address
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
	usart_obj.setup.usart_id = USARTx;

	/* Fill USART object structure's fields */
	SOOL_USART_DMA_Copy(&usart_obj, &dma_rx, &dma_tx);

	return (usart_obj);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void SOOL_USART_DMA_Copy(volatile USART_DMA_Periph *usart, const USART_DMA_SettingsHelper *rx_settings,
		const USART_DMA_SettingsHelper *tx_settings) {

	/* Fill the Setup structure's fields */

	/* RX */
	usart->setup.dma_rx.dma_channel = rx_settings->channel;
	usart->setup.dma_rx.int_flags   = rx_settings->int_flags;

	// buffer already assigned in this moment
	usart->rx.new_data_flag = 0;
	usart->rx.idle_line_flag = 0;

	/* TX */
	usart->setup.dma_tx.dma_channel = tx_settings->channel;
	usart->setup.dma_tx.int_flags   = tx_settings->int_flags;

	// buffer already assigned in this moment
	usart->tx.finished_flag = 0;
	usart->tx.started_flag = 0;

	/* Fill USART class' methods */
	usart->ActivateReading = USART_DMA_ActivateReading;
	usart->IsDataReceived = USART_DMA_IsDataReceived;
	usart->GetRxData = USART_DMA_GetRxData;
	usart->ClearRxBuffer = USART_DMA_ClearRxBuffer;
	usart->DmaRxIrqHandler = USART_DMA_RxInterruptHandler;

	usart->IsTxLineBusy = USART_DMA_IsTxLineBusy;
	usart->Send = USART_DMA_Send;
	usart->ClearTxBuffer = USART_DMA_ClearTxBuffer;
	usart->DmaTxIrqHandler = USART_DMA_TxInterruptHandler;

	usart->IdleLineIrqHandler = USART_DMA_IdleInterruptHandler;
	usart->DestroyBuffers = USART_DMA_DestroyBuffers;

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void USART_DMA_SetupAndStartDmaReading(volatile USART_DMA_Periph *usart, const size_t buf_start_pos) {

	/* Disable DMA Channel*/
	DMA_Cmd(usart->setup.dma_rx.dma_channel, DISABLE);

	/* Update peripheral's Data Register address */
	usart->setup.dma_rx.dma_channel->CPAR = (uint32_t)&usart->setup.usart_id->DR;

	/* Update memory base register address */
//	usart->setup.dma_rx.dma_channel->CMAR = (uint32_t)usart->rx.buffer.data;
	usart->setup.dma_rx.dma_channel->CMAR = (uint32_t)&usart->rx.buffer.data[buf_start_pos];

	/* Change buffer size - bigger data will come in parts */
	usart->setup.dma_rx.dma_channel->CNDTR = usart->setup.BUF_INIT_SIZE;

	/* Reset the Idle Line flag */
	usart->rx.idle_line_flag = 0;

	/* Enable Idle Line interrupt */
	USART_ITConfig(usart->setup.usart_id, USART_IT_IDLE, ENABLE);

	/* Start DMA Channel's reading */
	DMA_Cmd(usart->setup.dma_rx.dma_channel, ENABLE);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void USART_DMA_ActivateReading(volatile USART_DMA_Periph *usart) {
	USART_DMA_SetupAndStartDmaReading(usart, 0);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void USART_DMA_RestartReading(volatile USART_DMA_Periph *usart, const size_t buf_start_pos) {
//
//
//
//	/* Reset the Idle Line flag */
//	usart->rx.idle_line_flag = 0;
//
//	/* Disable DMA Channel*/
//	DMA_Cmd(usart->setup.dma_rx.dma_channel, DISABLE);
//
//	/* Update peripheral's Data Register address */
//	usart->setup.dma_rx.dma_channel->CPAR = (uint32_t)&usart->setup.usart_id->DR;
//
//	/* Update memory base register address */
//
//
//	/* Change buffer size */
//	usart->setup.dma_rx.dma_channel->CNDTR = 10;
//
//	// read usart's dr
//	//uint16_t test = usart->setup.usart_id->DR;
//
//	/* Start DMA Channel's reading */
//	DMA_Cmd(usart->setup.dma_rx.dma_channel, ENABLE);


	/* This is invoked after TC but not all data was transmitted due to full RX buffer */
	USART_DMA_SetupAndStartDmaReading(usart, buf_start_pos);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t USART_DMA_IsDataReceived(volatile USART_DMA_Periph *usart) {

	uint8_t temp = usart->rx.new_data_flag;
	usart->rx.new_data_flag = 0;
	return (temp);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static const Array_String* USART_DMA_GetRxData(volatile USART_DMA_Periph *usart) {

	/* update Array_String info */
	usart->rx.buffer.info.total = USART_DMA_GetMaxNonEmptyItemIndex(&usart->rx.buffer) + 1;
	usart->rx.buffer.info.add_index = usart->rx.buffer.info.total;
	usart->rx.new_data_flag = 0;
	return (&usart->rx.buffer);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint16_t USART_DMA_GetMaxNonEmptyItemIndex(Array_String *arr) {

	/* Array is filled with 0-s by default, find non-zero value with the biggest index */
	for ( size_t i = (arr->info.capacity - 1); i >= 0; i-- ) {
		if ( arr->data[i] != 0 ) {
			return (i);
		}
	}
	return (0);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void	USART_DMA_ClearRxBuffer(volatile USART_DMA_Periph *usart) {
	usart->rx.buffer.Clear(&usart->rx.buffer);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t USART_DMA_RxInterruptHandler(volatile USART_DMA_Periph *usart) {

	if ( DMA_GetITStatus(usart->setup.dma_rx.int_flags.ERROR_FLAG) == SET ) {

		/* Transfer Error Interrupt */
//		DMA_ClearITPendingBit(usart->setup.dma_rx.int_flags.ERROR_FLAG);
		DMA_ClearITPendingBit(usart->setup.dma_rx.int_flags.GLOBAL_FLAG);
		// TODO: some error message

		return (1);

	} else if ( DMA_GetITStatus(usart->setup.dma_rx.int_flags.COMPLETE_FLAG) == SET ) {

		/* Transfer Complete Interrupt */
//		DMA_ClearITPendingBit(usart->setup.dma_rx.int_flags.COMPLETE_FLAG);
		DMA_ClearITPendingBit(usart->setup.dma_rx.int_flags.GLOBAL_FLAG);

		/* Check whether RX line is idle - if yes then disable DMA RX Channel,
		 * otherwise resize the buffer as it is full */
		DMA_Cmd(usart->setup.dma_rx.dma_channel, DISABLE);

		if ( usart->rx.idle_line_flag ) {

			/* Idle Line and Transfer Complete flag indicate new data */
			usart->rx.new_data_flag = 1;

		} else {

			/* The line is not in idle state:
			 * 	- 	number of data to receive was known in advance (no way to tell if it is that case or the next),
			 * 	-	there are still some data to receive (buffer must be resized)
			 */
			/* The buffer must be resized in both cases - make it 2 times bigger */
			if ( !usart->rx.buffer.Resize(&usart->rx.buffer, 2 * usart->rx.buffer.info.capacity) ) {
				// FIXME: unable to resize - data will be lost, this is not a good situation - print some error
				int abc = 0;
				abc++;
			}

			/* Try to continue reading making the message continuously written into buffer;
			 * usually this resize takes too long and data is lost */
			USART_DMA_RestartReading(usart, usart->rx.buffer.info.capacity / 2);
//			usart->ActivateReading(usart);

		}
		return (1);

	} else if ( DMA_GetITStatus(usart->setup.dma_rx.int_flags.HALF_FLAG) == SET ) {

		/* Half Transfer Interrupt */
//		DMA_ClearITPendingBit(usart->setup.dma_rx.int_flags.HALF_FLAG);
		DMA_ClearITPendingBit(usart->setup.dma_rx.int_flags.GLOBAL_FLAG);
		return (1);

	} else if ( DMA_GetITStatus(usart->setup.dma_rx.int_flags.GLOBAL_FLAG) == SET ) {

		/* Global DMA Channel Interrupt */
		DMA_ClearITPendingBit(usart->setup.dma_rx.int_flags.GLOBAL_FLAG);
		return (1);

	} else {

		// indicate that no interrupt flag was recognized and cleared
		return (0);
	}

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t USART_DMA_IdleInterruptHandler(volatile USART_DMA_Periph *usart) {

	if ( USART_GetITStatus(usart->setup.usart_id, USART_IT_IDLE) == SET ) {

		USART_ClearITPendingBit(usart->setup.usart_id, USART_IT_IDLE);
		USART_ITConfig(usart->setup.usart_id, USART_IT_IDLE, DISABLE); // interrupts generated all the time when no data on bus

		if ( usart->setup.dma_rx.dma_channel->CCR & DMA_CCR1_EN ) {
			/* if reading is active and Idle Line detected
			 * then save this information for later use */
			/* EN bit check is used to avoid the first interrupt of an Idle Line issue
			 * (triggers just after enabling the IT_IDLE) */
			usart->rx.idle_line_flag = 1;
		}
		return (1);

	}

	return (0);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t USART_DMA_IsTxLineBusy(volatile USART_DMA_Periph *usart) {
	return (usart->tx.started_flag);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void	USART_DMA_ClearTxBuffer(volatile USART_DMA_Periph *usart) {
	usart->tx.buffer.Clear(&usart->tx.buffer);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t USART_DMA_TxInterruptHandler(volatile USART_DMA_Periph *usart) {

	if ( DMA_GetITStatus(usart->setup.dma_tx.int_flags.ERROR_FLAG) == SET ) {

		/* Transfer Error Interrupt - happens for example when:
		 * 	- 	the peripheral register's address is wrong
		 *	- 	data couldn't be pushed due to the full buffer (RX-TX of MCU transfer)
		 */
//		DMA_ClearITPendingBit(usart->setup.dma_tx.int_flags.ERROR_FLAG);
		DMA_ClearITPendingBit(usart->setup.dma_tx.int_flags.GLOBAL_FLAG);
		// TODO: some error message

		return (1);

	} else if ( DMA_GetITStatus(usart->setup.dma_tx.int_flags.COMPLETE_FLAG) == SET ) {

		/* Transfer Complete Interrupt */
//		DMA_ClearITPendingBit(usart->setup.dma_tx.int_flags.COMPLETE_FLAG);
		DMA_ClearITPendingBit(usart->setup.dma_tx.int_flags.GLOBAL_FLAG);
		DMA_Cmd(usart->setup.dma_tx.dma_channel, DISABLE);
		usart->tx.finished_flag = 1;
		usart->tx.started_flag  = 0;

		return (1);

	} else if ( DMA_GetITStatus(usart->setup.dma_tx.int_flags.HALF_FLAG) == SET ) {

		/* Half Transfer Interrupt */
//		DMA_ClearITPendingBit(usart->setup.dma_tx.int_flags.HALF_FLAG);
		DMA_ClearITPendingBit(usart->setup.dma_tx.int_flags.GLOBAL_FLAG);
		return (1);

	} else if ( DMA_GetITStatus(usart->setup.dma_tx.int_flags.GLOBAL_FLAG) == SET ) {

		/* Global DMA Channel Interrupt */
		DMA_ClearITPendingBit(usart->setup.dma_tx.int_flags.GLOBAL_FLAG);
		return (1);

	} else {

		// indicate that no interrupt flag was recognized and cleared
		return (0);
	}

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t USART_DMA_Send(volatile USART_DMA_Periph *usart, char *to_send_buf) {

	/* Check message length to decide whether reallocation is a must */
	uint32_t length = (uint32_t)strlen(to_send_buf);

	if ( length > usart->tx.buffer.info.capacity ) {

		// Resize method discards USART's volatile qualifier - not crucial here
		if ( !usart->tx.buffer.Resize(&usart->tx.buffer, (size_t)length) ) {
			//perror("COULD NOT REALLOCATE AN ARRAY");
		}

	}

	/* Try to copy contents of the string to be sent */
	strcpy(usart->tx.buffer.data, to_send_buf);

	/* Disable DMA Channel*/
	DMA_Cmd(usart->setup.dma_tx.dma_channel, DISABLE);

	/* Update peripheral's Data Register address */
	usart->setup.dma_tx.dma_channel->CPAR = (uint32_t)&usart->setup.usart_id->DR;

	/* Update memory base register address */
	usart->setup.dma_tx.dma_channel->CMAR = (uint32_t)usart->tx.buffer.data;

	/* Change buffer size */
	/* Ref: `can only be written when the channel is disabled` */
	usart->setup.dma_tx.dma_channel->CNDTR = length;

	/* Start DMA Channel's transfer */
	DMA_Cmd(usart->setup.dma_tx.dma_channel, ENABLE);

	/* Store information that DMA transfer was started */
	usart->tx.started_flag = 1;

	/* Return 1 on successful sent */
	return (1);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void	USART_DMA_DestroyBuffers(volatile USART_DMA_Periph *usart) {
	usart->rx.buffer.Free(&usart->rx.buffer);
	usart->tx.buffer.Free(&usart->tx.buffer);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/*
uint8_t USART_DMA_SendData(UsartPeriph *usart) {

	DMA_InitTypeDef DMA_InitStructure;
	DMA_Cmd(usart->setup.dma_tx.dma_channel, ENABLE); 				// enable DMA transfer

	while( !DMA_GetFlagStatus(usart->setup.dma_tx.dma_tc_flag) ); 	// wait until whole buffer will be sent (transfer complete flag)
	DMA_ClearFlag(usart->setup.dma_tx.dma_tc_flag); 				// clear TC flag
	DMA_Cmd(usart->setup.dma_tx.dma_channel, DISABLE); 				// disable DMA transfer

	DMA_InitStructure.DMA_PeripheralBaseAddr = usart->setup.usart_periph->DR;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)usart->tx.tx_buffer.items;

	DMA_Init(usart->setup.dma_tx.dma_channel, &DMA_InitStructure);
	return (1);

}
*/

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

//char* TxBuf;
//int DATA_LEN = 256;
//
//
//void USART_Config(void)
//{
//  // USART init
//  USART_InitTypeDef  USART_InitStructure;
//
//  USART_InitStructure.USART_BaudRate = 9600;
//  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
//  USART_InitStructure.USART_StopBits = USART_StopBits_1;
//  USART_InitStructure.USART_Parity = USART_Parity_No;
//  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
//  USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
//
//	USART_Init(USART1, &USART_InitStructure);
//
//	USART_DMACmd(USART1, USART_DMAReq_Rx | USART_DMAReq_Tx, ENABLE);
//
//	USART_Cmd(USART1, ENABLE);
//
//
//}
//
//void DMA_Config() {
//
//	// DMA INIT
//  DMA_InitTypeDef DMA_InitStructure;
//
//  DMA_DeInit(DMA1_Channel4);                                            	// clear DMA config
//  DMA_InitStructure.DMA_PeripheralBaseAddr = USART1_BASE;  					// source register adress
//  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)TxBuf;            		// data block to be send initial address
//  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;                    	// transfer direction
//  DMA_InitStructure.DMA_BufferSize = DATA_LEN;                               	// buffer's length (number of items to send)
//  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;      	// auto-increment of address (peripheral's side)
//  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;               	// auto-increment of address (buffer's side)
//  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;	// data size (peripheral)
//  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;         	// data_size (buffer)
//  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;                             // mode
//  DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;                   // priority
//  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;                              // memory to memory setting
//
//  DMA_Init(DMA1_Channel4, &DMA_InitStructure);								// save config
//
//}
//
//void DMA_Start(void)
//{
//	DMA_InitTypeDef DMA_InitStructure;
//
//	DMA_Cmd(DMA1_Channel4, ENABLE); 																									//Wlaczenie transmisji DMA
//
//	while( !DMA_GetFlagStatus(DMA1_FLAG_TC4) ); 	 // DMA1 Channel4 transfer complete flag.																			//Czekaj az zostanie wyslany caly bufor
//	DMA_ClearFlag(DMA1_FLAG_TC4); 																										//Wyzeruj flage TC4
//	DMA_Cmd(DMA1_Channel4, DISABLE); 																									//Wylacz transmisje DMA
//
//  DMA_InitStructure.DMA_PeripheralBaseAddr = USART1_BASE;
//  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)TxBuf;
//
//  DMA_Init(DMA1_Channel4, &DMA_InitStructure);																			//Zapis konfiguracji
//
//}



