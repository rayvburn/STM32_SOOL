/*
 * USART_DMA.c
 *
 *  Created on: 26.04.2019
 *      Author: user
 */

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#include "sool/Peripherals/USART/USART_DMA.h"
#include "sool/Peripherals/NVIC/NVIC.h"
#include <string.h> 								// strcpy()

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/* When RX buffer detected being too small then increment
 * its size by this value (in bytes) */
#define USART_DMA_RX_BUFFER_INCREMENT 	10

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/* Methods */
// NVIC
static void 	USART_DMA_EnableNVIC(volatile SOOL_USART_DMA *usart);
static void 	USART_DMA_DisableNVIC(volatile SOOL_USART_DMA *usart);

// RX
static void 	USART_DMA_ActivateReading(volatile SOOL_USART_DMA *usart);
static void 	USART_DMA_DeactivateReading(volatile SOOL_USART_DMA *usart);
static uint8_t 	USART_DMA_IsDataReceived(volatile SOOL_USART_DMA *usart);
static const volatile SOOL_String* USART_DMA_GetRxData(volatile SOOL_USART_DMA *usart);
static size_t 	USART_DMA_GetRxDataLength(volatile SOOL_USART_DMA *usart);
static void		USART_DMA_ClearRxBuffer(volatile SOOL_USART_DMA *usart);
static uint8_t 	USART_DMA_ConfirmReception(volatile SOOL_USART_DMA* usart);

// TX
static uint8_t 	USART_DMA_IsTxLineBusy(volatile SOOL_USART_DMA *usart);
static uint8_t 	USART_DMA_Send(volatile SOOL_USART_DMA *usart, const char *to_send_buf);
static void		USART_DMA_ClearTxBuffer(volatile SOOL_USART_DMA *usart);

// IRQ
static uint8_t 	USART_DMA_RxInterruptHandler(volatile SOOL_USART_DMA *usart);
static uint8_t 	USART_DMA_TxInterruptHandler(volatile SOOL_USART_DMA *usart);
static uint8_t 	USART_DMA_IdleInterruptHandler(volatile SOOL_USART_DMA *usart);

// General
static uint8_t	USART_DMA_RestoreBuffersInitialSize(volatile SOOL_USART_DMA *usart, uint8_t mode);
static void		USART_DMA_Destroy(volatile SOOL_USART_DMA *usart);

// private class functions
static void 	USART_DMA_SetupAndStartDmaReading(volatile SOOL_USART_DMA *usart, const size_t buf_start_pos, const size_t num_bytes_to_read);
static uint16_t USART_DMA_GetMaxNonEmptyItemIndex(volatile SOOL_String *arr);
static uint8_t	USART_DMA_IsRxBufferFilledUp(volatile SOOL_USART_DMA *usart);
static size_t	USART_DMA_FindBufferEnd(volatile SOOL_USART_DMA *usart);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// private non-class function
//static void SOOL_Periph_USART_DMA_Copy(volatile SOOL_USART_DMA *usart, const struct _SOOL_DMA_SetupStruct *rx_settings, const struct _SOOL_DMA_SetupStruct *tx_settings);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/**
 * This function does handle only TX/RX pins of the USART and DMA peripherals interface - if any
 * remapping, which changes CK/CTS/RTS pins ports and numbers, is done then must be handled
 * separately
 * @param USARTx
 * @param baud
 * @param buf_size
 * @return
 */
volatile SOOL_USART_DMA SOOL_Periph_USART_DMA_Init(USART_TypeDef* USARTx, uint32_t baud, size_t buf_size) {

	/* GPIO setup */
	GPIO_TypeDef* port;
	uint16_t rx_pin, tx_pin;

	/* NVIC IRQn */
	uint8_t irqn;

//	/* Helper structs to store the DMA configuration in a clearer way */
//	struct _SOOL_DMA_SetupStruct dma_rx;
//	struct _SOOL_DMA_SetupStruct dma_tx;

	/* Create variables to configure DMA */
	DMA_TypeDef* 			rx_DMAy;
	DMA_Channel_TypeDef* 	rx_DMAy_Channelx;

	DMA_TypeDef* 			tx_DMAy;
	DMA_Channel_TypeDef* 	tx_DMAy_Channelx;

	/* Create a new USART_DMA object */
	volatile  SOOL_USART_DMA usart_obj;

	/* Initialize the peripheral's RX and TX buffers
	 * with an arbitrary length */
	if (buf_size == 0) {
		buf_size = 1;
	}
	usart_obj._rx.buffer = SOOL_Memory_String_Init(buf_size);
	usart_obj._tx.buffer = SOOL_Memory_String_Init(buf_size);
	usart_obj._tx.prepping_request = 0;

	/* Start port clock considering remapping (Reference Manual, p. 183 - AFIO_MAPR) */
	if ( USARTx == USART1 ) {

		/* Check remapping register */
		if ( AFIO->MAPR & AFIO_MAPR_USART1_REMAP ) {
			/* Remapped */
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
			port = GPIOB;
		} else {
			/* No remap */
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
			port = GPIOA;
		}

	} else if ( USARTx == USART2 ) {

		/* Check remapping register */
		if ( AFIO->MAPR & AFIO_MAPR_USART2_REMAP ) {
			/* Remapped */
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
			port = GPIOD;
		} else {
			/* No remap */
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
			port = GPIOA;
		}

	} else if ( USARTx == USART3 ) {

		if ( AFIO->MAPR & AFIO_MAPR_USART3_REMAP_FULLREMAP ) {
			/* Fully remapped */
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
			port = GPIOD;
		} else if ( AFIO->MAPR & AFIO_MAPR_USART3_REMAP_PARTIALREMAP ) {
			/* Partially remapped */
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE); /* NOTE: this applies only to RX and TX pins, advanced control must be handled separately! -> 01: Partial remap (TX/PC10, RX/PC11, CK/PC12, CTS/PB13, RTS/PB14) */
			port = GPIOC;
		} else {
			/* No remap */
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
			port = GPIOB;
		}

	}

	/* clock - alternative function */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,  ENABLE);

	(USARTx == USART1) ? (RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE)) : (0);
	(USARTx == USART2) ? (RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE)) : (0);
	(USARTx == USART3) ? (RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE)) : (0);

	/* Enable clock for the DMA peripheral (ref manual, p. 283) -
	 * RCC_AHBPeriph_DMA1 is valid for USART1, USART2, USART3 */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

	if ( USARTx == USART1 ) {

		/* Check remapping register */
		if ( AFIO->MAPR & AFIO_MAPR_USART1_REMAP ) {
			/* Remapped */
			// GPIO
			tx_pin = GPIO_Pin_6;
			rx_pin = GPIO_Pin_7;
		} else {
			/* No remap */
			// GPIO
			tx_pin = GPIO_Pin_9;
			rx_pin = GPIO_Pin_10;
		}

		// IRQn
		irqn = USART1_IRQn;

		// TX
		tx_DMAy = DMA1;
		tx_DMAy_Channelx = DMA1_Channel4;

		// RX
		rx_DMAy = DMA1;
		rx_DMAy_Channelx = DMA1_Channel5;

	} else if ( USARTx == USART2 ) {

		/* Check remapping register */
		if ( AFIO->MAPR & AFIO_MAPR_USART2_REMAP ) {
			/* Remapped */
			// GPIO
			tx_pin = GPIO_Pin_5;
			rx_pin = GPIO_Pin_6;
		} else {
			/* No remap */
			// GPIO
			tx_pin = GPIO_Pin_2;
			rx_pin = GPIO_Pin_3;
		}

		// IRQn
		irqn = USART2_IRQn;

		// TX
		tx_DMAy = DMA1;
		tx_DMAy_Channelx = DMA1_Channel7;

		// RX
		rx_DMAy = DMA1;
		rx_DMAy_Channelx = DMA1_Channel6;

	} else if ( USARTx == USART3 ) {

		if ( AFIO->MAPR & AFIO_MAPR_USART3_REMAP_FULLREMAP ) {
			/* Fully remapped */
			// GPIO
			tx_pin = GPIO_Pin_8;
			rx_pin = GPIO_Pin_9;
		} else if ( AFIO->MAPR & AFIO_MAPR_USART3_REMAP_PARTIALREMAP ) {
			/* Partially remapped */
			// GPIO
			tx_pin = GPIO_Pin_10;
			rx_pin = GPIO_Pin_11;
		} else {
			/* No remap */
			// GPIO
			tx_pin = GPIO_Pin_10;
			rx_pin = GPIO_Pin_11;
		}

		// IRQn
		irqn = USART3_IRQn;

		// TX
		tx_DMAy = DMA1;
		tx_DMAy_Channelx = DMA1_Channel2;

		// RX
		rx_DMAy = DMA1;
		rx_DMAy_Channelx = DMA1_Channel3;

	}

	/* Initialize DMA Channel for TX */
	/* Create DMA instances */
	SOOL_DMA dma_tx = SOOL_Periph_DMA_Init(tx_DMAy, tx_DMAy_Channelx, DMA_DIR_PeripheralDST,
							   DMA_PeripheralInc_Disable, DMA_MemoryInc_Enable, DMA_PeripheralDataSize_Byte,
							   DMA_MemoryDataSize_Byte, DMA_Mode_Normal, DMA_Priority_VeryHigh,
							   DMA_M2M_Disable, ENABLE, DISABLE, ENABLE);
	dma_tx.SetPeriphBaseAddr(&dma_tx, (uint32_t)&USARTx->DR);
	dma_tx.SetMemoryBaseAddr(&dma_tx, (uint32_t)&usart_obj._tx.buffer._data);
	dma_tx.SetBufferSize(&dma_tx, 1);

	/* Initialize DMA Channel for RX */
	SOOL_DMA dma_rx = SOOL_Periph_DMA_Init(rx_DMAy, rx_DMAy_Channelx, DMA_DIR_PeripheralSRC,
							   DMA_PeripheralInc_Disable, DMA_MemoryInc_Enable, DMA_PeripheralDataSize_Byte,
							   DMA_MemoryDataSize_Byte, DMA_Mode_Normal, DMA_Priority_VeryHigh,
							   DMA_M2M_Disable, ENABLE, DISABLE, ENABLE);
	dma_rx.SetPeriphBaseAddr(&dma_rx, (uint32_t)&USARTx->DR);
	dma_rx.SetMemoryBaseAddr(&dma_rx, (uint32_t)&usart_obj._rx.buffer._data);
	dma_rx.SetBufferSize(&dma_rx, 1);

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
	nvic.NVIC_IRQChannelCmd = DISABLE;
	NVIC_Init(&nvic);

	/* Enable peripheral */
	USART_Cmd(USARTx, ENABLE);

	/* IDLE line protection - enabled by ActivateReading() */
//	USART_ITConfig(USARTx, USART_IT_IDLE, ENABLE);

	/* Configure the USART DMA interface */
	USART_DMACmd(USARTx, USART_DMAReq_Rx, ENABLE);
	USART_DMACmd(USARTx, USART_DMAReq_Tx, ENABLE);

	/* Start transfer */
//	DMA_Cmd(dma_rx.channel, ENABLE);
//	DMA_Cmd(dma_tx.channel, ENABLE);

	/* -------- Fill USART object structure's fields -------- */
	/* Save USART peripheral ID in the `class`, USART NVIC channel and initial buffer size */
	usart_obj._setup.USARTx = USARTx;
	usart_obj._setup.IRQn = irqn;
	usart_obj._setup.BUF_INIT_SIZE = buf_size;

	/* Save DMA RX and DMA TX structures */
	usart_obj.base_dma_rx = dma_rx;
	usart_obj.base_dma_tx = dma_tx;

	// RX buffer already assigned in this moment
	usart_obj._rx.new_data_flag = 0;

	/* State */

	// TX buffer already assigned in this moment

	/* Fill USART class' methods */
	usart_obj.EnableNVIC = USART_DMA_EnableNVIC;
	usart_obj.DisableNVIC = USART_DMA_DisableNVIC;

	usart_obj.ActivateReading = USART_DMA_ActivateReading;
	usart_obj.DeactivateReading = USART_DMA_DeactivateReading;
	usart_obj.IsDataReceived = USART_DMA_IsDataReceived;
	usart_obj.GetRxData = USART_DMA_GetRxData;
	usart_obj.GetRxDataLength = USART_DMA_GetRxDataLength;
	usart_obj.ClearRxBuffer = USART_DMA_ClearRxBuffer;
	usart_obj._DmaRxIrqHandler = USART_DMA_RxInterruptHandler;

	usart_obj.IsTxLineBusy = USART_DMA_IsTxLineBusy;
	usart_obj.Send = USART_DMA_Send;
	usart_obj.ClearTxBuffer = USART_DMA_ClearTxBuffer;
	usart_obj._DmaTxIrqHandler = USART_DMA_TxInterruptHandler;

	usart_obj._IdleLineIrqHandler = USART_DMA_IdleInterruptHandler;
	usart_obj.RestoreBuffersInitialSize = USART_DMA_RestoreBuffersInitialSize;
	usart_obj.ConfirmReception = USART_DMA_ConfirmReception;
	usart_obj.Destroy = USART_DMA_Destroy;

//	SOOL_Periph_USART_DMA_Copy(&usart_obj, &dma_rx, &dma_tx);

	return (usart_obj);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void SOOL_Periph_USART_DMA_Startup(volatile SOOL_USART_DMA* usart_ptr) {
	usart_ptr->EnableNVIC(usart_ptr);
	usart_ptr->base_dma_tx.EnableNVIC(&usart_ptr->base_dma_tx);
	usart_ptr->base_dma_rx.EnableNVIC(&usart_ptr->base_dma_rx);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

//static void SOOL_Periph_USART_DMA_Copy(volatile SOOL_USART_DMA *usart, const struct _SOOL_DMA_SetupStruct *rx_settings,
//		const struct _SOOL_DMA_SetupStruct *tx_settings) {
//
//	/* Fill the Setup structure's fields */
//}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/* This is invoked after TC (RX side) but it may be that NOT ALL data was transmitted due to a full RX buffer */
static void USART_DMA_SetupAndStartDmaReading(volatile SOOL_USART_DMA *usart, const size_t buf_start_pos,
		const size_t num_bytes_to_read) {

	/* Disable DMA Channel*/
//	usart->_setup.dma_rx.DMAy_Channelx->CCR &= (uint16_t)(~DMA_CCR1_EN); // DMA_Cmd(usart->_setup.dma_rx.DMAy_Channelx, DISABLE);
	usart->base_dma_rx.Stop(&usart->base_dma_rx);

	/* Update peripheral's Data Register address */
//	usart->_setup.dma_rx.DMAy_Channelx->CPAR = (uint32_t)&usart->_setup.USARTx->DR;
	usart->base_dma_rx.SetPeriphBaseAddr(&usart->base_dma_rx, (uint32_t)&usart->_setup.USARTx->DR);

	/* Update memory base register address */
//	usart->_setup.dma_rx.DMAy_Channelx->CMAR = (uint32_t)&usart->_rx.buffer._data[buf_start_pos];
	usart->base_dma_rx.SetMemoryBaseAddr(&usart->base_dma_rx, (uint32_t)&usart->_rx.buffer._data[buf_start_pos]);

	/* Change buffer size - bigger data will come in parts */
//	usart->_setup.dma_rx.DMAy_Channelx->CNDTR = (uint32_t)num_bytes_to_read;
	usart->base_dma_rx.SetBufferSize(&usart->base_dma_rx, (uint32_t)num_bytes_to_read);

	/* Reset the New Data flag */
	usart->_rx.new_data_flag = 0;

	/* Enable Idle Line interrupt */
	usart->_setup.USARTx->CR1 |= USART_CR1_IDLEIE;

	/* Start DMA Channel's reading */
//	usart->_setup.dma_rx.DMAy_Channelx->CCR |= DMA_CCR1_EN; // DMA_Cmd(usart->_setup.dma_rx.DMAy_Channelx, ENABLE);
	usart->base_dma_rx.Start(&usart->base_dma_rx);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void USART_DMA_EnableNVIC(volatile SOOL_USART_DMA *usart) {
	SOOL_Periph_NVIC_Enable(usart->_setup.IRQn);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void USART_DMA_DisableNVIC(volatile SOOL_USART_DMA *usart) {
	SOOL_Periph_NVIC_Disable(usart->_setup.IRQn);
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
//	usart->_setup.dma_rx.DMAy_Channelx->CCR &= (uint16_t)(~DMA_CCR1_EN); // DMA_Cmd(usart->_setup.dma_rx.DMAy_Channelx, DISABLE);
	usart->base_dma_rx.Stop(&usart->base_dma_rx);

	/* Disable Idle Line interrupt */
	usart->_setup.USARTx->CR1 &= (uint16_t)(~USART_CR1_IDLEIE);

	/* Disable USART receiver mode */
	usart->_setup.USARTx->CR1 &= (uint16_t)(~USART_CR1_RE);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t USART_DMA_IsDataReceived(volatile SOOL_USART_DMA *usart) {

	/* Information about new data could be read as long as data
	 * were not read by 'GetRxData()' */
	return (usart->_rx.new_data_flag);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static const volatile SOOL_String* USART_DMA_GetRxData(volatile SOOL_USART_DMA *usart) {
	usart->_rx.new_data_flag = 0;
	return (&usart->_rx.buffer);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static size_t USART_DMA_GetRxDataLength(volatile SOOL_USART_DMA *usart) {

	uint8_t temp = usart->_rx.new_data_flag;
	size_t len = strlen(usart->GetRxData(usart)->_data);

	// restore `new_data` flag which is cleared by `GetRxData` call
	usart->_rx.new_data_flag = temp;
	return (len);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint16_t USART_DMA_GetMaxNonEmptyItemIndex(volatile SOOL_String *arr) {

	/* Array is filled with 0-s by default, find non-zero value with the biggest index;
	 * it could not be done on the fly because of use of DMA;
	 * Do not do this when capacity is \le 1, as the returned value
	 * will be approx. 65k (makes non sense) */
	if ( arr->_info.capacity > 1 ) {
		for ( size_t i = (arr->_info.capacity - 1); i >= 0; i-- ) {
			if ( arr->_data[i] != 0 ) {
				return (i);
			}
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

static uint8_t USART_DMA_ConfirmReception(volatile SOOL_USART_DMA* usart) {

	USART_DMA_ClearRxBuffer(usart);
	if ( !USART_DMA_RestoreBuffersInitialSize(usart, 0) ) {
		return (0);
	}

	/* Configure and start DMA RX Channel */
	USART_DMA_SetupAndStartDmaReading(usart, 0, (size_t)usart->_rx.buffer._info.capacity);

	return (1);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/**
 * @brief Function invoked inside interrupt handler only when `Transfer Complete` flag is set
 * @param usart
 * @return
 */
static uint8_t USART_DMA_RxInterruptHandler(volatile SOOL_USART_DMA *usart) {

	/* Check whether RX line is idle - if yes then disable DMA RX Channel,
	 * otherwise resize the buffer as it is full */
//	usart->_setup.dma_rx.DMAy_Channelx->CCR &= (uint16_t)(~DMA_CCR1_EN); // DMA_Cmd(usart->_setup.dma_rx.DMAy_Channelx, DISABLE);
	usart->base_dma_rx.Stop(&usart->base_dma_rx);

	/* Disable USART Idle Line Detection (no need to do that after successful flag clearance) */
//	usart->_setup.USARTx->CR1 &= (uint16_t)(~USART_CR1_IDLEIE); // equal to USART_ITConfig(usart->_setup.USARTx, USART_IT_IDLE, DISABLE);

	/* Since TC interrupt was triggered then:
	 * 	- 	number of data to receive was known in advance (no way to tell if it is that case or the next),
	 * 	-	there are still some data to receive (buffer must be resized)
	 */

	/* First, check whether RX buffer was fully filled up. Then, if filled, resize it */
	// NOTE: capacity cannot be set to 0
	size_t rx_buffer_end = USART_DMA_FindBufferEnd(usart);

	if (rx_buffer_end == (size_t)(usart->_rx.buffer._info.capacity - 1)) {
		/* The buffer must be resized in both cases - make it a little bigger.
		 * Only after "IDLE Line" interrupt is called, we are sure that all data were received!
		 * At this moment we expect new data to come, therefore resize */
		if (!usart->_rx.buffer.Resize(
			&usart->_rx.buffer,
			(size_t)(usart->_rx.buffer._info.capacity + USART_DMA_RX_BUFFER_INCREMENT))
		) {
			// FIXME: unable to resize - print some error
		}
	}

	/* Try to continue reading allowing the message to be continuously
	 * written into buffer (this is an ideal case)
	 * NOTE: sometimes resizing takes too long and data is lost (or maybe it was just a debugging method issue) */
	USART_DMA_SetupAndStartDmaReading(
		usart,
		rx_buffer_end + 1,
		(size_t)(usart->_rx.buffer._info.capacity) - rx_buffer_end - 1
	);

	return (1);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t USART_DMA_IdleInterruptHandler(volatile SOOL_USART_DMA *usart) {

	if ( USART_GetITStatus(usart->_setup.USARTx, USART_IT_IDLE) == RESET ) {
		return (0);
	}

//	USART_ClearITPendingBit(usart->_setup.USARTx, USART_IT_IDLE); // CAUTION: does not clear IT flag!
	/* Clear IDLE Line IT flag, Ref manual, p. 824, Bit 4 - "by a software sequence (an read
	 * to the USART_SR register followed by a read to the USART_DR register";
	 * line is IDLE so no data SHOULD be lost by reading a DR */
	usart->_setup.USARTx->SR;
	usart->_setup.USARTx->DR;

	/* NOTE: useful when interrupts generated all the time - this part
	 * is a leftover from an unsuccessful IT flag clearance */
//	usart->_setup.USARTx->CR1 &= (uint16_t)(~USART_CR1_IDLEIE);

//	if ( usart->_setup.dma_rx.DMAy_Channelx->CCR & DMA_CCR1_EN ) {
	if ( usart->base_dma_rx.IsRunning(&usart->base_dma_rx) ) {

		/* update Array_String info */
		usart->_rx.buffer._info.total = (uint16_t)(USART_DMA_GetMaxNonEmptyItemIndex(&usart->_rx.buffer) + 1);
		usart->_rx.buffer._info.add_index = usart->_rx.buffer._info.total;

		/* reading is active and Idle Line detected */
		/* EN bit check is used to avoid the first interrupt of an Idle Line issue
		 * (triggers just after enabling the IT_IDLE) */
		usart->_rx.new_data_flag = 1;

	}
	return (1);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t USART_DMA_IsTxLineBusy(volatile SOOL_USART_DMA *usart) {

	/* Reach information whether DMA transfer was started (usart->_tx.started_flag is DEPRECATED);
	 * a DMA EN bit raw check is sufficient - it is known in advance that USARTx uses DMA1 */
//	return (usart->_setup.dma_tx.DMAy_Channelx->CCR & DMA_CCR1_EN);
	return (usart->_tx.prepping_request || usart->base_dma_tx.IsRunning(&usart->base_dma_tx));

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void	USART_DMA_ClearTxBuffer(volatile SOOL_USART_DMA *usart) {
	usart->_tx.buffer.Clear(&usart->_tx.buffer);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/**
 * @brief Function invoked inside interrupt handler only when `Transfer Complete` flag is set
 * @param usart
 * @return
 */
static uint8_t USART_DMA_TxInterruptHandler(volatile SOOL_USART_DMA *usart) {
	/* Disable DMA Channel */
//	usart->_setup.dma_tx.DMAy_Channelx->CCR &= (uint16_t)(~DMA_CCR1_EN);// DMA_Cmd(usart->_setup.dma_tx.DMAy_Channelx, DISABLE);
	usart->base_dma_tx.Stop(&usart->base_dma_tx);

	USART_DMA_ClearTxBuffer(usart);

	return (1);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// this is used directly to send data from queue (see TxInterruptHandler);
// SendFull is indirectly called from main loop via USART_DMA instance's `Send()`
static uint8_t USART_DMA_Send(volatile SOOL_USART_DMA *usart, const char *to_send_buf) {
	/* Check whether Send call was generated by main loop request or comes from TxInterrupt (queue). */
	usart->_tx.prepping_request = 1;

	/* Check message length */
	uint32_t length = (uint32_t)strlen(to_send_buf);

	/* Make sure that DMA is not running, and wait if needed */
	while (usart->base_dma_tx.IsRunning(&usart->base_dma_tx));

	/* Decide whether reallocation is a must */
	// shrink the buffer if smaller is needed, extend if bigger
	if ( length != usart->_tx.buffer._info.capacity ) {

		/* `Resize` method discards USART's volatile qualifier - not crucial here */
		if ( !usart->_tx.buffer.Resize(&usart->_tx.buffer, (size_t)length) ) {
			// FIXME: error message
			//perror("COULD NOT REALLOCATE AN ARRAY");
			// TODO: debug this
		}

	}

	/* Try to copy contents of the string to be sent */
	strcpy(usart->_tx.buffer._data, to_send_buf); // beware of `&`, this is not a typical array

	/* Disable DMA Channel*/
//	usart->_setup.dma_tx.DMAy_Channelx->CCR &= (uint16_t)(~DMA_CCR1_EN);// DMA_Cmd(usart->_setup.dma_tx.DMAy_Channelx, DISABLE);
	usart->base_dma_tx.Stop(&usart->base_dma_tx);

	/* Update peripheral's Data Register address */
//	usart->_setup.dma_tx.DMAy_Channelx->CPAR = (uint32_t)&usart->_setup.USARTx->DR;
	usart->base_dma_tx.SetPeriphBaseAddr(&usart->base_dma_tx, (uint32_t)&usart->_setup.USARTx->DR);

	/* Update memory base register address */
//	usart->_setup.dma_tx.DMAy_Channelx->CMAR = (uint32_t)usart->_tx.buffer._data; // beware of `&`, this is not a typical array
	usart->base_dma_tx.SetMemoryBaseAddr(&usart->base_dma_tx, (uint32_t)usart->_tx.buffer._data);

	/* Change the buffer's size */
	/* Ref: `can only be written when the channel is disabled` */
//	usart->_setup.dma_tx.DMAy_Channelx->CNDTR = length;
	usart->base_dma_tx.SetBufferSize(&usart->base_dma_tx, (uint32_t)length);

	/* Start DMA Channel's transfer */
//	usart->_setup.dma_tx.DMAy_Channelx->CCR |= DMA_CCR1_EN;// DMA_Cmd(usart->_setup.dma_tx.DMAy_Channelx, ENABLE);
	usart->base_dma_tx.Start(&usart->base_dma_tx);

	usart->_tx.prepping_request = 0;

	/* Return 1 on successful sent */
	return (1);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t USART_DMA_RestoreBuffersInitialSize(volatile SOOL_USART_DMA *usart, uint8_t mode) {

	if ( mode == 0 || mode == 2 ) {
		/* `Resize` method discards USART's volatile qualifier - not crucial here */
		if ( usart->_rx.buffer._info.capacity > usart->_setup.BUF_INIT_SIZE) {
			/* Check whether buffer's capacity increased */
			if ( !usart->_rx.buffer.Resize(&usart->_rx.buffer, (size_t)usart->_setup.BUF_INIT_SIZE) ) {
				return (0);
			}
		}
	}

	if ( mode == 1 || mode == 2 ) {
		if ( usart->_tx.buffer._info.capacity > usart->_setup.BUF_INIT_SIZE) {
			/* Check whether buffer's capacity increased */
			if ( !usart->_tx.buffer.Resize(&usart->_tx.buffer, (size_t)usart->_setup.BUF_INIT_SIZE) ) {
				return (0);
			}
		}
	}

	return (1);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void	USART_DMA_Destroy(volatile SOOL_USART_DMA *usart) {

	/* Disable DMA RX Channel */
//	usart->_setup.dma_rx.DMAy_Channelx->CCR &= (uint16_t)(~DMA_CCR1_EN);
	usart->base_dma_rx.Stop(&usart->base_dma_rx);

	/* Disable DMA TX Channel */
//	usart->_setup.dma_tx.DMAy_Channelx->CCR &= (uint16_t)(~DMA_CCR1_EN);
	usart->base_dma_tx.Stop(&usart->base_dma_tx);

	/* Disable USART_DMA */
	usart->_setup.USARTx->CR1 &= (uint16_t)(~USART_CR1_UE);

	/* Deinit DMA Channels */
//	DMA_DeInit(usart->_setup.dma_tx.DMAy_Channelx);
	DMA_DeInit(usart->base_dma_rx._setup.DMAy_Channelx);
	DMA_DeInit(usart->base_dma_tx._setup.DMAy_Channelx);

	/* Deinit USART */
	USART_DeInit(usart->_setup.USARTx);

	/* Free memory taken by buffers */
//	usart->_rx.buffer.Free(&usart->_rx.buffer); // Infinite loop
//	usart->_tx.buffer.Free(&usart->_tx.buffer); // Infinite loop
	usart->_rx.buffer.Resize(&usart->_rx.buffer, 0);
	usart->_tx.buffer.Resize(&usart->_tx.buffer, 0);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t USART_DMA_IsRxBufferFilledUp(volatile SOOL_USART_DMA *usart) {
	if (usart->_rx.buffer._info.capacity == 0) {
		return 1;
	}

	size_t last_buffer_index = (size_t)(usart->_rx.buffer._info.capacity - 1);
	if (USART_DMA_FindBufferEnd(usart) == last_buffer_index) {
		return 1;
	}

	return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/* Looks for a last non-null character index in the given RX buffer */
static size_t USART_DMA_FindBufferEnd(volatile SOOL_USART_DMA *usart) {
	size_t capacity = (size_t)usart->_rx.buffer._info.capacity;

	if (capacity == 0) {
		return 0;
	}

	/* Heuristics applied here: we are looking for a few null chars in a row */
	size_t num_chars_null = 0;
	// number of found null characters, counted from the end of buffer
	size_t nulls_in_row = 0;
	size_t index_last_non_null_char = 0;

	for (size_t i = 0; i < capacity; i++) {
		if (usart->_rx.buffer._data[i] == 0) {
			num_chars_null++;
			nulls_in_row++;
		} else {
			nulls_in_row = 0;
			index_last_non_null_char = i;
		}
	}

	if (nulls_in_row > 0 && nulls_in_row == num_chars_null) {
		// we are sure that buffer is not full, since all null characters are placed at the end of the buffer
		return index_last_non_null_char;
	}
	return capacity;
}
