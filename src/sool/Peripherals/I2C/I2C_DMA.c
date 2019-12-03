/*
 * I2C_DMA.c
 *
 *  Created on: 28.11.2019
 *      Author: user
 */

#include "sool/Peripherals/I2C/I2C_DMA.h"
#include "sool/Peripherals/GPIO/PinConfig_AltFunction.h"
#include "stm32f10x_i2c.h"
#include "sool/Peripherals/NVIC/NVIC.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void SOOL_I2C_DMA_EnableNVIC(volatile SOOL_I2C_DMA *i2c_ptr);
static void SOOL_I2C_DMA_DisableNVIC(volatile SOOL_I2C_DMA *i2c_ptr);

static uint8_t SOOL_I2C_DMA_IsBusy(volatile SOOL_I2C_DMA *i2c_ptr);
static uint8_t SOOL_I2C_DMA_IsNewData(volatile SOOL_I2C_DMA *i2c_ptr);
static uint8_t SOOL_I2C_DMA_SendProcedural(volatile SOOL_I2C_DMA *i2c_ptr, uint8_t slave_address, uint32_t buf_tx_addr, uint32_t length, uint32_t buf_rx_addr);

static uint8_t SOOL_I2C_DMA_DmaTxIrqHandler(volatile SOOL_I2C_DMA *i2c_ptr);
static uint8_t SOOL_I2C_DMA_DmaRxIrqHandler(volatile SOOL_I2C_DMA *i2c_ptr);

// sends some data and points an address where received data will be stored;
// RX buffer must have a proper size (must be long enough) before this call
static uint8_t SOOL_I2C_DMA_Send(volatile SOOL_I2C_DMA *i2c_ptr, uint32_t addr_tx, uint32_t length, uint32_t addr_rx);
static uint8_t SOOL_I2C_DMA_RequestData();

static uint8_t SOOL_I2C_DMA_MasterTransmitter(volatile SOOL_I2C_DMA *i2c_ptr, uint8_t slave_address,
										      uint32_t buf_tx_addr, uint32_t length, uint32_t buf_rx_addr);
static uint8_t SOOL_I2C_DMA_MasterReceiver(volatile SOOL_I2C_DMA *i2c_ptr, uint8_t slave_address,
										   uint32_t buf_tx_addr, uint32_t length, uint32_t buf_rx_addr);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

volatile SOOL_I2C_DMA SOOL_Periph_I2C_DMA_Init(
	I2C_TypeDef*	I2Cx,
	uint16_t 		I2C_Ack,
	uint16_t		I2C_AcknowledgedAddress,
	uint32_t 		I2C_ClockSpeed /* <= 400 kHz */,
	uint16_t 		I2C_OwnAddress1
)

{
	/* New instance */
	volatile SOOL_I2C_DMA i2c_dma;

	/* Enable peripheral clock */
	if ( I2Cx == I2C1 ) {
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
	} else if ( I2Cx == I2C2 ) {
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);
	} else {
		return (i2c_dma);
	}

	/* DMA configuration */
	DMA_TypeDef* dma_periph;
	DMA_Channel_TypeDef* dma_channel_rx;
	DMA_Channel_TypeDef* dma_channel_tx;

	/* NVIC */
	uint8_t irqn_ev;
	uint8_t irqn_er;

	/* Initialize SDA and SCL pins, prepare DMA configuration */
	if ( I2Cx == I2C1 ) {

		/* DMA setup */
		dma_periph = DMA1;
		dma_channel_tx = DMA1_Channel6;
		dma_channel_rx = DMA1_Channel7;

		/* NVIC */
		irqn_ev = I2C1_EV_IRQn;
		irqn_er = I2C1_ER_IRQn;

		/* Check remapping */
		if ( AFIO->MAPR & AFIO_MAPR_I2C1_REMAP ) { // Reference manual 0008 -> 9.3.9
			SOOL_Periph_GPIO_PinConfig_Initialize_AltFunction(GPIOB, GPIO_Pin_8, GPIO_Mode_AF_OD); 	// SCL
			SOOL_Periph_GPIO_PinConfig_Initialize_AltFunction(GPIOB, GPIO_Pin_9, GPIO_Mode_AF_OD); 	// SDA
		} else {
			SOOL_Periph_GPIO_PinConfig_Initialize_AltFunction(GPIOB, GPIO_Pin_6, GPIO_Mode_AF_OD); 	// SCL
			SOOL_Periph_GPIO_PinConfig_Initialize_AltFunction(GPIOB, GPIO_Pin_7, GPIO_Mode_AF_OD); 	// SDA
		}

	} else if ( I2Cx == I2C2 ) {

		/* DMA setup */
		dma_periph = DMA1;
		dma_channel_tx = DMA1_Channel4;
		dma_channel_rx = DMA1_Channel5;

		/* NVIC */
		irqn_ev = I2C2_EV_IRQn;
		irqn_er = I2C2_ER_IRQn;

		/* Pin configuration */
		SOOL_Periph_GPIO_PinConfig_Initialize_AltFunction(GPIOB, GPIO_Pin_10, GPIO_Mode_AF_OD); 	// SCL
		SOOL_Periph_GPIO_PinConfig_Initialize_AltFunction(GPIOB, GPIO_Pin_11, GPIO_Mode_AF_OD); 	// SDA

	}

	/* Initialize DMA */

	// TX
	SOOL_DMA dma_tx = SOOL_Periph_DMA_Init(dma_periph, dma_channel_tx, DMA_DIR_PeripheralSRC, DMA_PeripheralInc_Disable,
			DMA_MemoryInc_Enable, DMA_PeripheralDataSize_Byte, DMA_MemoryDataSize_Byte, DMA_Mode_Normal, DMA_Priority_VeryHigh,
			DMA_M2M_Disable,
			DISABLE,	// transfer completed interrupt 		FIXME
			DISABLE, 	// half transfer completed interrupt 	FIXME
			DISABLE);	// error interrupt 						FIXME

	// RX
	SOOL_DMA dma_rx = SOOL_Periph_DMA_Init(dma_periph, dma_channel_rx, DMA_DIR_PeripheralDST, DMA_PeripheralInc_Disable,
			DMA_MemoryInc_Enable, DMA_PeripheralDataSize_Byte, DMA_MemoryDataSize_Byte, DMA_Mode_Normal, DMA_Priority_VeryHigh,
			DMA_M2M_Disable,
			DISABLE,	// transfer completed interrupt			FIXME
			DISABLE, 	// half transfer completed interrupt
			DISABLE);	// error interrupt						FIXME

//	/* Enable the SPI RX & TX DMA requests */
//	I2C_DMACmd(I2Cx, ENABLE);

	/* Enable I2Cx peripheral
	 * clock cannot be set then? TODO */
//	I2C_Cmd(I2Cx, ENABLE);

	/* TODO: Software reset
	 * @ref https://community.st.com/s/question/0D50X00009XkiPQSAZ/stm32f407-i2c-error-addr-bit-not-set-after-the-7bit-address-is-sent
	 */
//	I2C_SoftwareResetCmd(I2Cx, ENABLE);

	/* Leftovers from non-object-oriented version */
	// software reset - prevents the bus being stuck
	// I2C_SoftwareResetLight(I2C_PERIPH_ID);	// !

//	/* Enable the selected I2C Clock stretching */
//	I2C_StretchClockCmd(I2Cx, ENABLE);

//	/* Reset the I2C interface */
//	I2C_DeInit(I2Cx);

	/* Initialize I2Cx peripheral */
	I2C_InitTypeDef i2c;
	I2C_StructInit(&i2c);
	i2c.I2C_Mode = I2C_Mode_I2C;
	i2c.I2C_DutyCycle = I2C_DutyCycle_2;
	i2c.I2C_Ack = I2C_Ack;
	i2c.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress; 	// 7-bit / 10-bit addressing mode
	i2c.I2C_ClockSpeed = I2C_ClockSpeed;
	i2c.I2C_OwnAddress1 = I2C_OwnAddress1;
	I2C_Init(I2Cx, &i2c);

	/* Enable I2Cx peripheral TODO */
	I2C_Cmd(I2Cx, ENABLE);

	/* Initialize I2C NVIC */
	NVIC_InitTypeDef nvic;
	nvic.NVIC_IRQChannel = irqn_ev; 						// FIXME: ev | er (?)
	nvic.NVIC_IRQChannelPreemptionPriority = 0;
	nvic.NVIC_IRQChannelSubPriority = 0;
	nvic.NVIC_IRQChannelCmd = DISABLE;
	NVIC_Init(&nvic);
	nvic.NVIC_IRQChannel = irqn_er;
	NVIC_Init(&nvic);

	/* Save base classes */
	i2c_dma.base_dma_tx = dma_tx;
	i2c_dma.base_dma_rx = dma_rx;

	/* Save `Setup` structure fields */
	i2c_dma._setup.I2Cx = I2Cx;
	i2c_dma._setup.IRQn_EV = irqn_ev;
	i2c_dma._setup.IRQn_ER = irqn_er;

	/* Save `StateRx` structure fields */
	i2c_dma._state_rx.finished = 0;

	/* Save `StateTx` structure fields */
	i2c_dma._state_tx.finished = 0;

	/* Save `State` structure fields */
	// <BLANK>

	/* Save function pointers */
	i2c_dma.EnableNVIC = SOOL_I2C_DMA_EnableNVIC;
	i2c_dma.DisableNVIC = SOOL_I2C_DMA_DisableNVIC;
	i2c_dma.IsBusy = SOOL_I2C_DMA_IsBusy;
	i2c_dma.IsNewData = SOOL_I2C_DMA_IsNewData;
	i2c_dma.SendReceive = SOOL_I2C_DMA_SendProcedural;
	i2c_dma._DmaRxIrqHandler = SOOL_I2C_DMA_DmaRxIrqHandler;
	i2c_dma._DmaTxIrqHandler = SOOL_I2C_DMA_DmaTxIrqHandler;

	// FIXME:
	i2c_dma.MasterTransmitter = SOOL_I2C_DMA_MasterTransmitter;
	i2c_dma.MasterReceiver = SOOL_I2C_DMA_MasterReceiver;

	/* Check if the bus is busy */
	while ( I2C1->SR2 & I2C_FLAG_BUSY );

	return (i2c_dma);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void SOOL_I2C_DMA_EnableNVIC(volatile SOOL_I2C_DMA *i2c_ptr) {
	SOOL_Periph_NVIC_Enable(i2c_ptr->_setup.IRQn_EV);
	SOOL_Periph_NVIC_Enable(i2c_ptr->_setup.IRQn_ER);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void SOOL_I2C_DMA_DisableNVIC(volatile SOOL_I2C_DMA *i2c_ptr) {
	SOOL_Periph_NVIC_Disable(i2c_ptr->_setup.IRQn_EV);
	SOOL_Periph_NVIC_Disable(i2c_ptr->_setup.IRQn_ER);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t SOOL_I2C_DMA_IsBusy(volatile SOOL_I2C_DMA *i2c_ptr) {
	if ( i2c_ptr->base_dma_rx.IsRunning(&i2c_ptr->base_dma_rx) ||
		 i2c_ptr->base_dma_tx.IsRunning(&i2c_ptr->base_dma_tx) ) {
		return (1);
	}
	return (0);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t SOOL_I2C_DMA_IsNewData(volatile SOOL_I2C_DMA *i2c_ptr) {
	return (i2c_ptr->_state_rx.finished);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t SOOL_I2C_DMA_SendProcedural(volatile SOOL_I2C_DMA *i2c_ptr, uint8_t slave_address, uint32_t buf_tx_addr, uint32_t length, uint32_t buf_rx_addr) {

//	//	V1
//	/*----- Transmission Phase -----*/
//	/* Send I2C1 START condition */
//	I2C_GenerateSTART(i2c_ptr->_setup.I2Cx, ENABLE);
//	/* Test on I2C1 EV5 and clear it */
//	while(!I2C_CheckEvent(i2c_ptr->_setup.I2Cx, I2C_EVENT_MASTER_MODE_SELECT));
//	/* Send slave Address for write */
//	I2C_Send7bitAddress(i2c_ptr->_setup.I2Cx, slave_address, I2C_Direction_Transmitter);
//	/* Test on I2C1 EV6 and clear it */
//	while(!I2C_CheckEvent(i2c_ptr->_setup.I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
//
//	I2C_DMACmd(i2c_ptr->_setup.I2Cx, ENABLE);
//
//	/* Enable DMA */
// //	i2c_ptr->base_dma_tx.Start(&i2c_ptr->base_dma_tx);
//	i2c_ptr->base_dma_rx.Start(&i2c_ptr->base_dma_rx);

	// V2

	// FIXME
	i2c_ptr->base_dma_tx.Stop(&i2c_ptr->base_dma_tx);

	I2C_GenerateSTART(i2c_ptr->_setup.I2Cx, ENABLE);
	while (!I2C_GetFlagStatus(i2c_ptr->_setup.I2Cx, I2C_FLAG_SB));							// wait for EV5

	I2C_Send7bitAddress(i2c_ptr->_setup.I2Cx, slave_address, I2C_Direction_Transmitter);
	while (!I2C_GetFlagStatus(i2c_ptr->_setup.I2Cx, I2C_FLAG_ADDR));						// wait for EV6
	uint16_t sr1 = i2c_ptr->_setup.I2Cx->SR1;
	uint16_t sr2 = i2c_ptr->_setup.I2Cx->SR2;
	// ADDR=1, cleared by reading SR1 register followed by reading SR2


	// DMA should take care of this part
	while (!I2C_GetFlagStatus(i2c_ptr->_setup.I2Cx, I2C_FLAG_TXE));							// wait for EV8(_1)
	I2C_SendData(i2c_ptr->_setup.I2Cx, 0x01);
	while (!I2C_GetFlagStatus(i2c_ptr->_setup.I2Cx, I2C_FLAG_TXE));							// wait for EV8
	I2C_SendData(i2c_ptr->_setup.I2Cx, 0x02);
	while (!(I2C_GetFlagStatus(i2c_ptr->_setup.I2Cx, I2C_FLAG_TXE) &&
			 I2C_GetFlagStatus(i2c_ptr->_setup.I2Cx, I2C_FLAG_BTF)));						// wait for EV8_2
	I2C_GenerateSTOP(i2c_ptr->_setup.I2Cx, ENABLE);

	i2c_ptr->base_dma_tx.Start(&i2c_ptr->base_dma_tx);

	return (1);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t SOOL_I2C_DMA_MasterReceiver(volatile SOOL_I2C_DMA *i2c_ptr, uint8_t slave_address,
										   uint32_t buf_tx_addr, uint32_t length, uint32_t buf_rx_addr)

{

	// 1a - Configure the I2C DMA channel for reception
	/* Configure DMA TX */
	i2c_ptr->base_dma_tx.SetBufferSize(&i2c_ptr->base_dma_tx, (uint32_t)length);
	i2c_ptr->base_dma_tx.SetMemoryBaseAddr(&i2c_ptr->base_dma_tx, (uint32_t)buf_tx_addr);
	i2c_ptr->base_dma_tx.SetPeriphBaseAddr(&i2c_ptr->base_dma_tx, (uint32_t)&i2c_ptr->_setup.I2Cx->DR);

	/* Configure DMA RX */
	i2c_ptr->base_dma_rx.SetBufferSize(&i2c_ptr->base_dma_rx, (uint32_t)length);
	i2c_ptr->base_dma_rx.SetMemoryBaseAddr(&i2c_ptr->base_dma_rx, (uint32_t)buf_rx_addr);
	i2c_ptr->base_dma_rx.SetPeriphBaseAddr(&i2c_ptr->base_dma_rx, (uint32_t)&i2c_ptr->_setup.I2Cx->DR);

	/* Set buffers length, for RX and TX channels it is the same */
	if ( length > 1 ) {
		i2c_ptr->base_dma_rx.SetMemoryInc(&i2c_ptr->base_dma_rx, ENABLE);
		i2c_ptr->base_dma_tx.SetMemoryInc(&i2c_ptr->base_dma_tx, ENABLE);
	} else {
		i2c_ptr->base_dma_rx.SetMemoryInc(&i2c_ptr->base_dma_rx, DISABLE);
		i2c_ptr->base_dma_tx.SetMemoryInc(&i2c_ptr->base_dma_tx, DISABLE);
	}

	// 1b - Enable the I2C channel for reception
	i2c_ptr->base_dma_rx.Start(&i2c_ptr->base_dma_rx); /* Enable DMA */

	// 2a - Set DMAEN bit
	I2C_DMACmd(i2c_ptr->_setup.I2Cx, ENABLE);
	// 2b - Set LAST bit (used to generate a NACK automatically on the last received byte.)
	i2c_ptr->_setup.I2Cx->CR2 |= I2C_CR2_LAST;

	// 3a - Send a START condition
	I2C_GenerateSTART(i2c_ptr->_setup.I2Cx, ENABLE);
	// 3b - Wait until SB is set and clear it (cleared by a read of CR1 and writing DR)
	while (!I2C_GetFlagStatus(i2c_ptr->_setup.I2Cx, I2C_FLAG_SB));

	// 4a - Send slave address
	I2C_Send7bitAddress(i2c_ptr->_setup.I2Cx, slave_address, I2C_Direction_Receiver);
	// 4b - Wait until ADDR is set and clear it
	while (!I2C_GetFlagStatus(i2c_ptr->_setup.I2Cx, I2C_FLAG_ADDR));						// wait for EV6
	uint16_t sr1 = i2c_ptr->_setup.I2Cx->SR1;
	uint16_t sr2 = i2c_ptr->_setup.I2Cx->SR2;

	// 5a - Wait until the DMA end of transfer
	while (!DMA_GetFlagStatus(i2c_ptr->base_dma_rx._setup.int_flags.COMPLETE_FLAG));
	// 5b - Disable the DMA channel
	i2c_ptr->base_dma_rx.Stop(&i2c_ptr->base_dma_rx);
	// 5c - Clear the DMA transfer complete flag
	DMA_ClearFlag(i2c_ptr->base_dma_rx._setup.int_flags.COMPLETE_FLAG);

	// 6b - Program the STOP
	I2C_GenerateSTOP(i2c_ptr->_setup.I2Cx, ENABLE);
	// 6c - Wait until STOP bit is cleared by hardware
	while (i2c_ptr->_setup.I2Cx->CR1 & I2C_CR1_STOP);

	return (1);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t SOOL_I2C_DMA_MasterTransmitter(volatile SOOL_I2C_DMA *i2c_ptr, uint8_t slave_address,
										      uint32_t buf_tx_addr, uint32_t length, uint32_t buf_rx_addr)
{

	// according to the ST's AN2824
	// -----------------------------------------------------------------------------------------------

	// 1a - Configure the I2C DMA channel for transmission
	/* Configure DMA TX */
	i2c_ptr->base_dma_tx.SetBufferSize(&i2c_ptr->base_dma_tx, (uint32_t)length);
	i2c_ptr->base_dma_tx.SetMemoryBaseAddr(&i2c_ptr->base_dma_tx, (uint32_t)buf_tx_addr);
	i2c_ptr->base_dma_tx.SetPeriphBaseAddr(&i2c_ptr->base_dma_tx, (uint32_t)&i2c_ptr->_setup.I2Cx->DR);

	/* Configure DMA RX */
	i2c_ptr->base_dma_rx.SetBufferSize(&i2c_ptr->base_dma_rx, (uint32_t)length);
	i2c_ptr->base_dma_rx.SetMemoryBaseAddr(&i2c_ptr->base_dma_rx, (uint32_t)buf_rx_addr);
	i2c_ptr->base_dma_rx.SetPeriphBaseAddr(&i2c_ptr->base_dma_rx, (uint32_t)&i2c_ptr->_setup.I2Cx->DR);

	/* Set buffers length, for RX and TX channels it is the same */
	if ( length > 1 ) {
		i2c_ptr->base_dma_rx.SetMemoryInc(&i2c_ptr->base_dma_rx, ENABLE);
		i2c_ptr->base_dma_tx.SetMemoryInc(&i2c_ptr->base_dma_tx, ENABLE);
	} else {
		i2c_ptr->base_dma_rx.SetMemoryInc(&i2c_ptr->base_dma_rx, DISABLE);
		i2c_ptr->base_dma_tx.SetMemoryInc(&i2c_ptr->base_dma_tx, DISABLE);
	}

	// 1b - Enable the I2C DMA channel for transmission
	/* Enable DMA Channel */
	i2c_ptr->base_dma_tx.Start(&i2c_ptr->base_dma_tx);

	// 2 - Set DMAEN bit
	I2C_DMACmd(i2c_ptr->_setup.I2Cx, ENABLE);

	// 3a - Send a START condition
	I2C_GenerateSTART(i2c_ptr->_setup.I2Cx, ENABLE);
	// 3b - Wait until SB is set and clear it (cleared by a read of CR1 and writing DR)
	while (!I2C_GetFlagStatus(i2c_ptr->_setup.I2Cx, I2C_FLAG_SB));							// wait for EV5

	// 4a - Send slave address
	I2C_Send7bitAddress(i2c_ptr->_setup.I2Cx, slave_address, I2C_Direction_Transmitter);
	// 4b - Wait until ADDR is set and clear it
	while (!I2C_GetFlagStatus(i2c_ptr->_setup.I2Cx, I2C_FLAG_ADDR));						// wait for EV6
	uint16_t sr1 = i2c_ptr->_setup.I2Cx->SR1;
	uint16_t sr2 = i2c_ptr->_setup.I2Cx->SR2;


	// ------ DMA transfer starts right here ------


	// 5a - Wait until the DMA end of transfer
	while (!DMA_GetFlagStatus(i2c_ptr->base_dma_tx._setup.int_flags.COMPLETE_FLAG));
	// 5b - Disable the DMA channel
	i2c_ptr->base_dma_tx.Stop(&i2c_ptr->base_dma_tx);
	// 5c - Clear the DMA transfer complete flag
	DMA_ClearFlag(i2c_ptr->base_dma_tx._setup.int_flags.COMPLETE_FLAG);

	// 6a - Wait until BTF = 1
	while (!I2C_GetFlagStatus(i2c_ptr->_setup.I2Cx, I2C_FLAG_BTF));
	// 6b - Program the STOP
	I2C_GenerateSTOP(i2c_ptr->_setup.I2Cx, ENABLE);
	// 6c - Wait until STOP bit is cleared by hardware
	while (i2c_ptr->_setup.I2Cx->CR1 & I2C_CR1_STOP);

	return (1);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t SOOL_I2C_DMA_DmaTxIrqHandler(volatile SOOL_I2C_DMA *i2c_ptr) {

	i2c_ptr->_state_tx.finished = 1;

//	while (!I2C_GetFlagStatus(i2c_ptr->_setup.I2Cx, I2C_FLAG_BTF));
	I2C_GenerateSTOP(i2c_ptr->_setup.I2Cx, ENABLE);

	return (1);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t SOOL_I2C_DMA_DmaRxIrqHandler(volatile SOOL_I2C_DMA *i2c_ptr) {

	i2c_ptr->_state_rx.finished = 1;

	/* Disable DMA Channels - it is safe to do this after finished transfer and reception */
//	i2c_ptr->base_dma_rx.Stop(&i2c_ptr->base_dma_rx);
//	i2c_ptr->base_dma_tx.Stop(&i2c_ptr->base_dma_tx);

	return (1);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
