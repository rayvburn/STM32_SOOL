/*
 * USART_DMA.c
 *
 *  Created on: 26.04.2019
 *      Author: user
 */

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#include <include/Peripherals/USART/USART_DMA.h>
#include <string.h> // strcpy()
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// static uint8_t USART_DMA_SendData(UsartPeriph *usart);
static uint8_t USART_DMA_IsRxBufferNotEmpty(UsartPeriph *usart);
static uint8_t USART_DMA_TransmissionCompleted_InterruptHandler(UsartPeriph *usart);
static uint8_t USART_DMA_Send(UsartPeriph *usart, char *to_send_buf);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

volatile UsartPeriph SOOL_USART_DMA_Init(USART_TypeDef* usart_periph_id, uint32_t baud) {

	/* gpio port */
	GPIO_TypeDef* port;

	/* dma variables */
	/*		tx 		*/
	DMA_Channel_TypeDef* dma_channel_tx;
	uint8_t dma_tx_irqn;
	uint32_t dma_tx_tc_flag;

	/*		rx 		*/
	DMA_Channel_TypeDef* dma_channel_rx;
	uint8_t dma_rx_irqn;
	uint32_t dma_rx_tc_flag;

	/* USART_DMA object */
	volatile  UsartPeriph usart_obj;
	/* initialize peripheral's buffers */
	usart_obj.tx.tx_buffer = SOOL_ArrayString_Init(25);
	usart_obj.rx.rx_buffer = SOOL_ArrayString_Init(25);


	// clock for port
	// TODO: remap handling, USART could go to PD/PC when remapped
	if ( usart_periph_id == USART1 || usart_periph_id == USART2 ) {
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
		port = GPIOA;
	} else if ( usart_periph_id == USART3 ) {
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
		port = GPIOB;
	}

	// clock - alternative function
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,  ENABLE);

	(usart_periph_id == USART1) ? (RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE)) : (0);
	(usart_periph_id == USART2) ? (RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE)) : (0);
	(usart_periph_id == USART3) ? (RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE)) : (0);

	// enable clock for DMA (ref manual, p. 283) - valid for USART1, USART2, USART3
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

	uint16_t rx_pin, tx_pin;
	uint8_t irqn;

	// TODO: remap handling
	if ( usart_periph_id == USART1 ) {

		tx_pin = GPIO_Pin_9;
		rx_pin = GPIO_Pin_10;
		irqn = USART1_IRQn;
		//tx
		dma_channel_tx = DMA1_Channel4;
		dma_tx_irqn = DMA1_Channel4_IRQn;
		dma_tx_tc_flag = DMA1_FLAG_TC4;
		//rx
		dma_channel_rx = DMA1_Channel5;
		dma_rx_irqn = DMA1_Channel5_IRQn;
		dma_rx_tc_flag = DMA1_FLAG_TC5;

	} else if ( usart_periph_id == USART2 ) {

		tx_pin = GPIO_Pin_2;
		rx_pin = GPIO_Pin_3;
		irqn = USART2_IRQn;
		//tx
		dma_channel_tx = DMA1_Channel7;
		dma_tx_irqn = DMA1_Channel7_IRQn;
		dma_tx_tc_flag = DMA1_FLAG_TC7;
		// rx
		dma_channel_rx = DMA1_Channel6;
		dma_rx_irqn = DMA1_Channel6_IRQn;
		dma_rx_tc_flag = DMA1_FLAG_TC6;

	} else if ( usart_periph_id == USART3 ) {

		tx_pin = GPIO_Pin_10;
		rx_pin = GPIO_Pin_11;
		irqn = USART3_IRQn;
		//tx
		dma_channel_tx = DMA1_Channel2;
		dma_tx_irqn = DMA1_Channel2_IRQn;
		dma_tx_tc_flag = DMA1_FLAG_TC2;
		//rx
		dma_channel_rx = DMA1_Channel3;
		dma_rx_irqn = DMA1_Channel3_IRQn;
		dma_rx_tc_flag = DMA1_FLAG_TC3;

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
	USART_Init(usart_periph_id, &usart);

//	USART_ITConfig(usart_periph_id, USART_IT_RXNE, int_rx_en);
//	USART_ITConfig(usart_periph_id, USART_IT_TXE,  int_tx_en);

	/* Enable interrupts from usart */
	NVIC_InitTypeDef nvic;
	nvic.NVIC_IRQChannel = irqn;
	nvic.NVIC_IRQChannelPreemptionPriority = 0;
	nvic.NVIC_IRQChannelSubPriority = 1;
	nvic.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvic);

	USART_Cmd(usart_periph_id, ENABLE);

	/* IDLE line protection */
//	USART_ITConfig(usart_periph_id, USART_IT_IDLE, ENABLE);

	/* DMA configuration */
	DMA_InitTypeDef dma;

	/* Rx */
	DMA_DeInit(dma_channel_rx);                              	// clear DMA configuration
	dma.DMA_PeripheralBaseAddr = usart_periph_id->DR;  			// peripheral register address

	// to be specified before first transfer
	dma.DMA_MemoryBaseAddr = (uint32_t)usart_obj.rx.rx_buffer.items; // data block to be send initial address
	dma.DMA_DIR = DMA_DIR_PeripheralDST;                    	// transfer direction
	dma.DMA_BufferSize = 1;                              		// buffer's length (number of items to send)
	dma.DMA_PeripheralInc = DMA_PeripheralInc_Disable;      	// auto-increment of address (peripheral's side)
	dma.DMA_MemoryInc = DMA_MemoryInc_Enable;               	// auto-increment of address (buffer's side)
	dma.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;	// data size (peripheral)
	dma.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;         	// data_size (buffer)
	dma.DMA_Mode = DMA_Mode_Normal;                             // mode
	dma.DMA_Priority = DMA_Priority_VeryHigh;                   // priority
	dma.DMA_M2M = DMA_M2M_Disable;                              // memory to memory setting
	DMA_Init(dma_channel_rx, &dma);								// save configuration

	/* Tx */
	DMA_DeInit(dma_channel_tx);                              	// clear DMA configuration
	dma.DMA_PeripheralBaseAddr = usart_periph_id->DR;  			// peripheral register address

	// to be specified before a first transfer
	dma.DMA_MemoryBaseAddr = (uint32_t)usart_obj.tx.tx_buffer.items; // data block to be send initial address
	dma.DMA_DIR = DMA_DIR_PeripheralSRC;                    	// transfer direction
	dma.DMA_BufferSize = 1;                              		// buffer's length (number of items to send)
	dma.DMA_PeripheralInc = DMA_PeripheralInc_Disable;      	// auto-increment of address (peripheral's side)
	dma.DMA_MemoryInc = DMA_MemoryInc_Enable;               	// auto-increment of address (buffer's side)
	dma.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;	// data size (peripheral)
	dma.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;         	// data_size (buffer)
	dma.DMA_Mode = DMA_Mode_Normal;                             // mode
	dma.DMA_Priority = DMA_Priority_VeryHigh;                   // priority
	dma.DMA_M2M = DMA_M2M_Disable;                              // memory to memory setting
	DMA_Init(dma_channel_tx, &dma);								// save configuration

	/* Enable global interrupts for DMA */
	nvic.NVIC_IRQChannel = dma_rx_irqn;
	nvic.NVIC_IRQChannelCmd = ENABLE;
	nvic.NVIC_IRQChannelPreemptionPriority = 0;
	nvic.NVIC_IRQChannelSubPriority = 0;
	NVIC_Init(&nvic);

	nvic.NVIC_IRQChannel = dma_tx_irqn;
	nvic.NVIC_IRQChannelCmd = ENABLE;
	nvic.NVIC_IRQChannelPreemptionPriority = 0;
	nvic.NVIC_IRQChannelSubPriority = 0;
	NVIC_Init(&nvic);

	/* configure USART DMA interface */
	USART_DMACmd(usart_periph_id, USART_DMAReq_Rx, ENABLE);
	USART_DMACmd(usart_periph_id, USART_DMAReq_Tx, ENABLE);

	/* Turn on transfer complete interupt */
	DMA_ITConfig(dma_channel_rx, DMA_IT_TC, ENABLE);
	DMA_ITConfig(dma_channel_tx, DMA_IT_TC, ENABLE);
//	DMA_Cmd(dma_channel_rx, ENABLE);
//	DMA_Cmd(dma_channel_tx, ENABLE);

	/* fill rx/tx structures fields */
	usart_obj.tx.transfer_finished = 0;
	usart_obj.rx.new_data_flag = 0;

	/* fill setup structure fields */
	usart_obj.setup.dma_rx_channel = dma_channel_rx;
	usart_obj.setup.dma_rx_tc_flag = dma_rx_tc_flag;
	usart_obj.setup.dma_tx_channel = dma_channel_tx;
	usart_obj.setup.dma_tx_tc_flag = dma_tx_tc_flag;
	usart_obj.setup.usart_periph = usart_periph_id;

	/* fill object's methods */
	usart_obj.DmaTcIrqHandler = USART_DMA_TransmissionCompleted_InterruptHandler;
	usart_obj.IsDataReceived = USART_DMA_IsRxBufferNotEmpty;
	usart_obj.Send = USART_DMA_Send;

	return (usart_obj);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

uint8_t USART_DMA_SendData(UsartPeriph *usart) {

	DMA_InitTypeDef DMA_InitStructure;
	DMA_Cmd(usart->setup.dma_tx_channel, ENABLE); 				// enable DMA transfer

	while( !DMA_GetFlagStatus(usart->setup.dma_tx_tc_flag) ); 	// wait until whole buffer will be sent (transfer complete flag)
	DMA_ClearFlag(usart->setup.dma_tx_tc_flag); 				// clear TC flag
	DMA_Cmd(usart->setup.dma_tx_channel, DISABLE); 				// disable DMA transfer

	DMA_InitStructure.DMA_PeripheralBaseAddr = usart->setup.usart_periph->DR;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)usart->tx.tx_buffer.items;

	DMA_Init(usart->setup.dma_tx_channel, &DMA_InitStructure);
	return (1);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

uint8_t USART_DMA_IsRxBufferNotEmpty(UsartPeriph *usart) {

	uint8_t temp = usart->rx.new_data_flag;
	usart->rx.new_data_flag = 0;
	return (temp);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t USART_DMA_TransmissionCompleted_InterruptHandler(UsartPeriph *usart) {

	if ( DMA_GetFlagStatus( usart->setup.dma_tx_tc_flag ) == RESET ) {
		return (0);
	}

	DMA_ClearITPendingBit(usart->setup.dma_tx_tc_flag);
	DMA_Cmd(usart->setup.dma_tx_channel, DISABLE);
	usart->tx.transfer_finished = 1;

	return (1);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t USART_DMA_Send(UsartPeriph *usart, char *to_send_buf) {

	strcpy(usart->tx.tx_buffer.items, to_send_buf);

	/* Restart DMA Channel*/
	DMA_Cmd(usart->setup.dma_tx_channel, DISABLE);

	/* Change buffer size */
	usart->setup.dma_tx_channel->CNDTR = strlen(usart->tx.tx_buffer.items);

	/* Start DMA Channel's transfer */
	DMA_Cmd(usart->setup.dma_tx_channel, ENABLE);

	return (1);
}

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



