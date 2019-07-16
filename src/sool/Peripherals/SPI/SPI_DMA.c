/*
 * SPI_DMA.c
 *
 *  Created on: 12.07.2019
 *      Author: user
 */

#include "sool/Peripherals/SPI/SPI_DMA.h"
#include "sool/Peripherals/GPIO/GPIO_common.h"
#include "sool/Peripherals/NVIC/NVIC.h"
#include "stm32f10x.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void SOOL_SPI_DMA_EnableNVIC(volatile SOOL_SPI_DMA *spi_ptr);
static void SOOL_SPI_DMA_DisableNVIC(volatile SOOL_SPI_DMA *spi_ptr);
static SOOL_SPI_Device SOOL_SPI_DMA_AddDevice(volatile SOOL_SPI_DMA *spi_ptr, GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);

static uint8_t SOOL_SPI_DMA_IsBusy(volatile SOOL_SPI_DMA *spi_ptr);
static uint8_t SOOL_SPI_DMA_Send(volatile SOOL_SPI_DMA *spi_ptr, SOOL_SPI_Device *dev_ptr, uint32_t mem_addr, uint32_t length);

static uint8_t SOOL_SPI_DMA_Read(volatile SOOL_SPI_DMA *spi_ptr, SOOL_SPI_Device *dev_ptr, uint32_t mem_addr, uint32_t length);
static uint8_t SOOL_SPI_DMA_IsNewData(volatile SOOL_SPI_DMA *spi_ptr);
static uint8_t SOOL_SPI_DMA_GetNewDataDeviceID(volatile SOOL_SPI_DMA *spi_ptr);

static uint8_t SOOL_SPI_DMA_TransmitReceive(volatile SOOL_SPI_DMA *spi_ptr, SOOL_SPI_Device *dev_ptr, uint32_t mem_addr_rx, uint32_t mem_addr_tx, uint32_t length);

static uint8_t SOOL_SPI_DMA_DmaRxIrqHandler(volatile SOOL_SPI_DMA *spi_ptr);
static uint8_t SOOL_SPI_DMA_DmaTxIrqHandler(volatile SOOL_SPI_DMA *spi_ptr);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

//typedef enum {
//	SPI_DMA_OPERATION_RECEIVE = 0u,
//	SPI_DMA_OPERATION_SEND,
//	SPI_DMA_OPERATION_SEND_RCV,
//	SPI_DMA_OPERATION_NONE,
//};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// NSS hardware mode unsupported
// Slave mode unsupported thus (uint16_t SPI_Mode) deleted from parameters
volatile SOOL_SPI_DMA SOOL_Periph_SPI_DMA_Init(SPI_TypeDef *SPIx, uint16_t SPI_Direction,
		uint16_t SPI_DataSize, uint16_t SPI_CPOL, uint16_t SPI_CPHA,
		uint16_t SPI_BaudRatePrescaler, uint16_t SPI_FirstBit) {

	/* Create new object */
	volatile SOOL_SPI_DMA spi_dma;

	/* Enable Alternate function clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,  ENABLE);

	/* Enable port clock */
	(SPIx == SPI1) ? (RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE)) : (0);
	(SPIx == SPI2) ? (RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE)) : (0);

	/* Enable peripheral clock */
	(SPIx == SPI1) ? (RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1,  ENABLE)) : (0);
	(SPIx == SPI2) ? (RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2,  ENABLE)) : (0);

	/* Peripheral pins and DMA configuration variables */
	GPIO_TypeDef *spi_port_main;					// GPIO port (SCK, MISO, MOSI share the same port)
	GPIO_TypeDef *spi_port_cs;						// GPIO port of NSS (chip select)
	uint16_t sck_pin, mosi_pin, miso_pin, cs_pin; 	// GPIO pins
	uint8_t spi_irqn;

	DMA_TypeDef* dma_periph;
	DMA_Channel_TypeDef* dma_channel_rx;
	DMA_Channel_TypeDef* dma_channel_tx;

	/* According to SPIx, set variables to match MCU wiring */
	if ( SPIx == SPI1 ) {

		// check remap register
		if ( AFIO->MAPR | AFIO_MAPR_SPI1_REMAP ) {

			// remapped
			spi_port_main = GPIOB;
			spi_port_cs = GPIOA;
			cs_pin = GPIO_Pin_15;
			sck_pin = GPIO_Pin_3;
			miso_pin = GPIO_Pin_4;
			mosi_pin = GPIO_Pin_5;

		} else {

			// no remap made
			spi_port_main = GPIOA;
			spi_port_cs = GPIOA;
			cs_pin = GPIO_Pin_4;
			sck_pin = GPIO_Pin_5;
			miso_pin = GPIO_Pin_6;
			mosi_pin = GPIO_Pin_7;

		}

		spi_irqn = SPI1_IRQn;

		dma_periph = DMA1;
		dma_channel_rx = DMA1_Channel2;
		dma_channel_tx = DMA1_Channel3;

	} else if ( SPIx == SPI2 ) {

		spi_port_main = GPIOB;
		spi_port_cs = GPIOB;
		cs_pin = GPIO_Pin_12;
		sck_pin = GPIO_Pin_13;
		miso_pin = GPIO_Pin_14;
		mosi_pin = GPIO_Pin_15;

		spi_irqn = SPI2_IRQn;

		dma_periph = DMA1;
		dma_channel_rx = DMA1_Channel4;
		dma_channel_tx = DMA1_Channel5;

	}

	/* Initialize GPIO pins of the SPIx */
	GPIO_InitTypeDef gpio;
	GPIO_StructInit(&gpio);
	gpio.GPIO_Pin = sck_pin | mosi_pin; // SCK, MOSI
	gpio.GPIO_Mode = GPIO_Mode_AF_PP;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(spi_port_main, &gpio);

	gpio.GPIO_Pin = miso_pin; 			// MISO
	gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(spi_port_main, &gpio);

//	// NOTE: CHIP SELECT PIN INITIALIZATION DELETED BECAUSE HARDWARE SLAVE MANAGEMENT IS NOT SUPPORTED HERE
//	// CHIP SELECT PIN IS INITIALIZED VIA SOOL_SPI_Device `class` initializer
//	gpio.GPIO_Pin = cs_pin; 			// CS
//	gpio.GPIO_Mode = GPIO_Mode_Out_PP;
//	GPIO_Init(spi_port_cs, &gpio);
//
//	/* Set CS pin to high state */
//	SOOL_Periph_GPIO_SetBits(spi_port_cs, cs_pin);	//	GPIO_SetBits(spi_port, cs_pin);

	/* Reset the SPI interface */
	SPI_I2S_DeInit(SPIx);

	/* Configure and initialize SPIx peripheral */
	SPI_InitTypeDef spi;
	SPI_StructInit(&spi);
	spi.SPI_Direction = SPI_Direction;
	spi.SPI_Mode = SPI_Mode_Master;
	spi.SPI_DataSize = SPI_DataSize;
	spi.SPI_CPOL = SPI_CPOL;
	spi.SPI_CPHA = SPI_CPHA;
	spi.SPI_NSS = SPI_NSS_Soft;	// software slave management
	spi.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler;
	spi.SPI_FirstBit = SPI_FirstBit;
	SPI_Init(SPIx, &spi);

//	/* Software slave management ?? */
//	SPIx->CR1 |= SPI_CR1_SSM;

	/* SPIx interrupts are not enabled because DMA takes care of clearing of all flags */

	/* Prepare some values for DMA channel configuration */
	uint32_t dma_periph_data_size = 0x00;
	uint32_t dma_memory_data_size = 0x00;
	uint8_t size_of = 0; 					// additionally deduct sizeof(data_element)
	if ( SPI_DataSize == SPI_DataSize_8b ) {
		dma_periph_data_size = DMA_PeripheralDataSize_Byte;
		dma_memory_data_size = DMA_MemoryDataSize_Byte;
		size_of = 1;
	} else if ( SPI_DataSize == SPI_DataSize_16b ) {
		dma_periph_data_size = DMA_PeripheralDataSize_HalfWord;
		dma_memory_data_size = DMA_MemoryDataSize_HalfWord;
		size_of = 2;
	} else {
		// this should not happen
		return (spi_dma);
	}

	/* Configure and initialize DMA channels */
	// RX
	SOOL_DMA dma_rx = SOOL_Periph_DMA_Init(dma_periph, dma_channel_rx, DMA_DIR_PeripheralSRC,
			DMA_PeripheralInc_Disable, DMA_MemoryInc_Disable, dma_periph_data_size,
			dma_memory_data_size, DMA_Mode_Normal, DMA_Priority_High, DMA_M2M_Disable,
			ENABLE, DISABLE, ENABLE);

	// TX
	SOOL_DMA dma_tx = SOOL_Periph_DMA_Init(dma_periph, dma_channel_tx, DMA_DIR_PeripheralDST,
			DMA_PeripheralInc_Disable, DMA_MemoryInc_Enable, dma_periph_data_size,
			dma_memory_data_size, DMA_Mode_Normal, DMA_Priority_High, DMA_M2M_Disable,
			ENABLE, DISABLE, ENABLE);

	/* Enable the SPI RX & TX DMA requests */
	SPI_I2S_DMACmd(SPIx, SPI_I2S_DMAReq_Rx | SPI_I2S_DMAReq_Tx, ENABLE);

	/* Enable SPI peripheral */
	SPI_Cmd(SPIx, ENABLE);

	/* Initialize SPIx's NVIC */
	NVIC_InitTypeDef nvic;
	nvic.NVIC_IRQChannel = spi_irqn;
	nvic.NVIC_IRQChannelPreemptionPriority = 0;
	nvic.NVIC_IRQChannelSubPriority = 0;
	nvic.NVIC_IRQChannelCmd = DISABLE;
	NVIC_Init(&nvic);

	// - - - - - - - - - - - - - - - -

	/* Save base classes */
	spi_dma.base_dma_rx = dma_rx;
	spi_dma.base_dma_tx = dma_tx;

	/* Save `Setup` structure fields */
	spi_dma._setup.SPIx = SPIx;
	spi_dma._setup.IRQn = spi_irqn;
	spi_dma._setup.cs_pin = cs_pin;
	spi_dma._setup.cs_port = spi_port_cs;
	spi_dma._setup.device_counter = 0;
	spi_dma._setup.size_of = size_of;

	/* Save `StateRx` structure fields */
//	spi_dma._state_rx.dev_id = 0;
	spi_dma._state_rx.finished = 0;

	/* Save `StateTx` structure fields */
//	spi_dma._state_tx.dev_id = 0;
	spi_dma._state_tx.finished = 0;

	/* Save `State` structure fields */

	/* Save function pointers */
	spi_dma.AddDevice = SOOL_SPI_DMA_AddDevice;
	spi_dma.EnableNVIC = SOOL_SPI_DMA_EnableNVIC;
	spi_dma.DisableNVIC = SOOL_SPI_DMA_DisableNVIC;
	spi_dma.GetNewDataDeviceID = SOOL_SPI_DMA_GetNewDataDeviceID;
	spi_dma.IsBusy = SOOL_SPI_DMA_IsBusy;
	spi_dma.IsNewData = SOOL_SPI_DMA_IsNewData;
	spi_dma.Read = SOOL_SPI_DMA_Read;
	spi_dma.Send = SOOL_SPI_DMA_Send;
	spi_dma.SendReceive = SOOL_SPI_DMA_TransmitReceive;
	spi_dma._DmaRxIrqHandler = SOOL_SPI_DMA_DmaRxIrqHandler;
	spi_dma._DmaTxIrqHandler = SOOL_SPI_DMA_DmaTxIrqHandler;

	return (spi_dma);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static void SOOL_SPI_DMA_EnableNVIC(volatile SOOL_SPI_DMA *spi_ptr) {
	SOOL_Periph_NVIC_Enable(spi_ptr->_setup.IRQn);
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static void SOOL_SPI_DMA_DisableNVIC(volatile SOOL_SPI_DMA *spi_ptr){
	SOOL_Periph_NVIC_Disable(spi_ptr->_setup.IRQn);
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static volatile SOOL_SPI_Device SOOL_SPI_DMA_AddDevice(volatile SOOL_SPI_DMA *spi_ptr,
													   GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin) {

	/* Create a new instance */
	volatile SOOL_SPI_Device new_dev;

	/* Create a new instance of PinSwitch */
	SOOL_PinConfig_NoInt pin_cfg = SOOL_Periph_GPIO_PinConfig_Initialize_NoInt(GPIOx, GPIO_Pin, GPIO_Mode_Out_PP);
	SOOL_PinSwitch switcher = SOOL_Effector_PinSwitch_Init(pin_cfg);

	/* Set device in IDLE state (SetHigh) */
	switcher.SetHigh(&switcher);

	/* Fill structure fields */
	new_dev._setup.SPIx = spi_ptr->_setup.SPIx;
	new_dev._setup.DEV_ID = ++spi_ptr->_setup.device_counter; // 0 is reserved for UNKNOWN
	new_dev.base = switcher;

	return (new_dev);

}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static uint8_t SOOL_SPI_DMA_IsBusy(volatile SOOL_SPI_DMA *spi_ptr) {
	if ( spi_ptr->base_dma_rx.IsRunning(&spi_ptr->base_dma_rx) ||
		 spi_ptr->base_dma_tx.IsRunning(&spi_ptr->base_dma_tx) ) {
		return (1);
	}
	return (0);
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static uint8_t SOOL_SPI_DMA_Send(volatile SOOL_SPI_DMA *spi_ptr, SOOL_SPI_Device *dev_ptr,
								 uint32_t mem_addr, uint32_t length) {

	/* Check if SPI is busy at the moment */
//	if ( SOOL_SPI_DMA_IsBusy(spi_ptr) ) { return (0); }
	if ( spi_ptr->base_dma_tx.IsRunning(&spi_ptr->base_dma_tx) ) {
		return (0);
	}
	/* Check number of data elements */
	if ( length == 0 ) {
		return (0);
	}

	/* Reset */
	spi_ptr->_state_tx.finished = 0;

	/* Save state */
	spi_ptr->_state.operation = 1;
	spi_ptr->_state.last_dev_ptr = dev_ptr;
//	spi_ptr->_state_tx.dev_id = dev_ptr->_setup.DEV_ID;

	/* Configure DMA transfer */
	spi_ptr->base_dma_tx.Stop(&spi_ptr->base_dma_tx);

	spi_ptr->base_dma_tx.SetPeriphBaseAddr(&spi_ptr->base_dma_tx, (uint32_t)&spi_ptr->_setup.SPIx->DR);
	spi_ptr->base_dma_tx.SetMemoryBaseAddr(&spi_ptr->base_dma_tx, mem_addr);
	spi_ptr->base_dma_tx.SetBufferSize(&spi_ptr->base_dma_tx, length);

	if ( length > 1 ) {
		spi_ptr->base_dma_tx.SetMemoryInc(&spi_ptr->base_dma_tx, ENABLE);
	} else {
		spi_ptr->base_dma_tx.SetMemoryInc(&spi_ptr->base_dma_tx, DISABLE);
	}

	dev_ptr->base.SetLow(&dev_ptr->base);
	spi_ptr->base_dma_tx.Start(&spi_ptr->base_dma_tx);

	/* Return status of request */
	return (1);

}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static uint8_t SOOL_SPI_DMA_Read(volatile SOOL_SPI_DMA *spi_ptr, SOOL_SPI_Device *dev_ptr,
								 uint32_t mem_addr, uint32_t length) {

	/* Check if SPI is busy at the moment */
//	if ( SOOL_SPI_DMA_IsBusy(spi_ptr) ) { return (0); }
	if ( spi_ptr->base_dma_rx.IsRunning(&spi_ptr->base_dma_rx) ) {
		return (0);
	}

	/* Reset */
	spi_ptr->_state_rx.finished = 0;

	/* Save state */
	spi_ptr->_state.operation = 0; // SHOULD IT BE USED TO DISTINGUISH INSIDE ISR WHICH OPERATION JUST FINISHED?
								   // AND ON THAT INFORMATION CHIP SELECT LINE SHOULD BE PULLED LOW?
								   // HOW ABOUT TRANSMIT_RECEIVE procedure?
	spi_ptr->_state.last_dev_ptr = dev_ptr;
//	spi_ptr->_state_rx.dev_id = dev_ptr->_setup.DEV_ID;

	/* Configure DMA transfer */
	spi_ptr->base_dma_rx.Stop(&spi_ptr->base_dma_rx);

	spi_ptr->base_dma_tx.SetPeriphBaseAddr(&spi_ptr->base_dma_rx, (uint32_t)&spi_ptr->_setup.SPIx->DR);
	spi_ptr->base_dma_rx.SetMemoryBaseAddr(&spi_ptr->base_dma_rx, mem_addr);
	spi_ptr->base_dma_rx.SetBufferSize(&spi_ptr->base_dma_rx, length);
	if ( length > 1 ) {
		spi_ptr->base_dma_rx.SetMemoryInc(&spi_ptr->base_dma_rx, ENABLE);
	} else {
		spi_ptr->base_dma_rx.SetMemoryInc(&spi_ptr->base_dma_rx, DISABLE);
	}

	dev_ptr->base.SetLow(&dev_ptr->base);
	spi_ptr->base_dma_rx.Start(&spi_ptr->base_dma_rx);

	/* Return status of request */
	return (1);

}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static uint8_t SOOL_SPI_DMA_IsNewData(volatile SOOL_SPI_DMA *spi_ptr) {
	return (spi_ptr->_state_rx.finished);
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static uint8_t SOOL_SPI_DMA_GetNewDataDeviceID(volatile SOOL_SPI_DMA *spi_ptr) {
//	return (spi_ptr->_state_rx.dev_id);
	return (spi_ptr->_state.last_dev_ptr->_setup.DEV_ID);
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static uint8_t SOOL_SPI_DMA_TransmitReceive(volatile SOOL_SPI_DMA *spi_ptr, SOOL_SPI_Device *dev_ptr,
		uint32_t mem_addr_rx, uint32_t mem_addr_tx, uint32_t length) {

	/* Check if SPI is busy at the moment */
	if ( SOOL_SPI_DMA_IsBusy(spi_ptr) ) {
		return (0);
	}
	/* Check number of data elements */
	if ( length == 0 ) {
		return (0);
	}

	/* Reset */
	spi_ptr->_state_rx.finished = 0;
	spi_ptr->_state_tx.finished = 0;

	/* Save state */
	spi_ptr->_state.operation = 2;
	spi_ptr->_state.last_dev_ptr = dev_ptr;
//	spi_ptr->_state_rx.dev_id = dev_ptr->_setup.DEV_ID;

	/* Disable both DMA Channels */ // not needed, it is checked in SOOL_SPI_DMA_IsBusy()

	/* Configure DMA RX */
	spi_ptr->base_dma_tx.SetPeriphBaseAddr(&spi_ptr->base_dma_rx, (uint32_t)&spi_ptr->_setup.SPIx->DR);
	spi_ptr->base_dma_rx.SetMemoryBaseAddr(&spi_ptr->base_dma_rx, mem_addr_rx);
	spi_ptr->base_dma_rx.SetBufferSize(&spi_ptr->base_dma_rx, length);

	/* Configure DMA TX */
	spi_ptr->base_dma_tx.SetPeriphBaseAddr(&spi_ptr->base_dma_tx, (uint32_t)&spi_ptr->_setup.SPIx->DR);
	spi_ptr->base_dma_tx.SetMemoryBaseAddr(&spi_ptr->base_dma_tx, mem_addr_tx);
	spi_ptr->base_dma_tx.SetBufferSize(&spi_ptr->base_dma_tx, length);

	/* Set buffers length, for RX and TX channels it is the same */
	if ( length > 1 ) {
		spi_ptr->base_dma_rx.SetMemoryInc(&spi_ptr->base_dma_rx, ENABLE);
		spi_ptr->base_dma_tx.SetMemoryInc(&spi_ptr->base_dma_tx, ENABLE);
	} else {
		spi_ptr->base_dma_rx.SetMemoryInc(&spi_ptr->base_dma_rx, DISABLE);
		spi_ptr->base_dma_tx.SetMemoryInc(&spi_ptr->base_dma_tx, DISABLE);
	}

	/* Pull CS line LOW */
	dev_ptr->base.SetLow(&dev_ptr->base);

	/* Enable RX channel first, next TX to start DMA transfer */
	spi_ptr->base_dma_rx.Start(&spi_ptr->base_dma_rx);
	spi_ptr->base_dma_tx.Start(&spi_ptr->base_dma_tx);

	return (1);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Called on TransferComplete event
static uint8_t SOOL_SPI_DMA_DmaRxIrqHandler(volatile SOOL_SPI_DMA *spi_ptr) {
	spi_ptr->_state.last_dev_ptr->base.SetHigh(&spi_ptr->_state.last_dev_ptr->base);
	spi_ptr->_state_rx.finished = 1;
	return (1);
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Called on TransferComplete event
static uint8_t SOOL_SPI_DMA_DmaTxIrqHandler(volatile SOOL_SPI_DMA *spi_ptr) {
	/* Push CS line HIGH back again when only sending data (1)
	 * DEPRECATED: TC flag is set while data still going through MOSI! */
//	if ( spi_ptr->_state.operation == 1 ) {
//		spi_ptr->_state.last_dev_ptr->base.SetHigh(&spi_ptr->_state.last_dev_ptr->base);
//	}
	spi_ptr->_state_tx.finished = 1;
	return (1);
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
