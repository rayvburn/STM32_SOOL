/*
 * SPI_DMA.h
 *
 *  Created on: 12.07.2019
 *      Author: user
 */

#ifndef INC_SOOL_PERIPHERALS_SPI_SPI_DMA_H_
#define INC_SOOL_PERIPHERALS_SPI_SPI_DMA_H_

#include "sool/Peripherals/DMA/DMA.h"
#include "sool/Peripherals/SPI/SPI_device.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

struct _SOOL_SPI_DMA_SetupStruct {
	SPI_TypeDef   *	SPIx;
	uint8_t 		IRQn;
	GPIO_TypeDef  *	cs_port;	// for software control
	uint16_t 		cs_pin;
	uint8_t			size_of;	// size of data element
	uint8_t			device_counter;
};

struct _SOOL_SPI_DMA_StateStruct {
	uint8_t			operation; 		// 0 - reading, 1 - sending, 2 - send&receive
	SOOL_SPI_Device	*last_dev_ptr;
};

struct _SOOL_SPI_DMA_StateTxStruct {
	uint8_t 		finished;
//	uint8_t			dev_id;
};

// with DMA associating data with reading request is tricky (with non-blocking behavior)
struct _SOOL_SPI_DMA_StateRxStruct {
//	uint8_t 		left_to_read;
	uint8_t 		finished;
//	uint8_t			dev_id;
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/* Forward declaration */
typedef struct _SOOL_SPI_DMA_Struct SOOL_SPI_DMA;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/**
 * Implementation for SPI Master device
 */
struct _SOOL_SPI_DMA_Struct {

	// ---------------------------------------------
	SOOL_DMA							base_dma_rx;
	SOOL_DMA							base_dma_tx;
	// ---------------------------------------------

	struct _SOOL_SPI_DMA_SetupStruct	_setup;
	struct _SOOL_SPI_DMA_StateStruct	_state;
	struct _SOOL_SPI_DMA_StateTxStruct	_state_tx;
	struct _SOOL_SPI_DMA_StateRxStruct	_state_rx;

	void (*EnableNVIC)(volatile SOOL_SPI_DMA*);
	void (*DisableNVIC)(volatile SOOL_SPI_DMA*);

	/**
	 * This will return a new instance of SOOL_SPI_Device.
	 * SOOL_SPI_Device should have a unique identifier assigned so many devices of the same type
	 * could be connected at the same time and it can be distinguished which of them data belongs to. ??
	 * What to do with different buffer data types ??
	 * @param
	 * @return
	 */
	SOOL_SPI_Device (*AddDevice)(volatile SOOL_SPI_DMA*, GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);

	/**
	 * Returns false when transmission is running or is about to start next data transfer (when
	 * sending many bytes).
	 * In other words, if an active transmission exists, returns false.
	 * @param
	 * @return
	 */
	uint8_t (*IsBusy)(volatile SOOL_SPI_DMA*);

	/**
	 * @brief Send request
	 * @note SPI_Device's buffer should be prepared accordingly first (resized, filled with data etc)
	 * @note sizeof(data_element) is deducted from SPI constructor
	 * @note When only sending data TransferComplete interrupt flag is set when transfer
	 * has not finished yet, so it REQUIRES manual state change of device's CS pin in the main loop.
	 * @param SOOL_SPI_DMA instance
	 * @param memory address (start)
	 * @param buffer length
	 * @return status
	 */
	uint8_t (*Send)(volatile SOOL_SPI_DMA*, SOOL_SPI_Device*, uint32_t, uint32_t);

	/**
	 * @brief Generates read request, it must be preceded by Send request; during read request
	 * the MCU send some dummy data to force the other device (usually slave) to send meaningful data
	 * @note SPI_Device's buffer should be prepared accordingly first (resized, filled with data etc)
	 * and start DMA transfer
	 * @note sizeof(data_element) is deducted from SPI constructor
	 * @param SOOL_SPI_DMA instance
	 * @param memory address (start)
	 * @param number of elements to read
	 * @return status
	 */
	uint8_t (*Read)(volatile SOOL_SPI_DMA*, SOOL_SPI_Device*, uint32_t, uint32_t); // TODO: not tested

	// TODO: SwitchInterrupts(volatile SOOL_SPI_DMA*, FunctionalState)
	// TODO: TransmitReceive - add Blocking version with DMA

	/**
	 * @brief Sends data pointed by mem_addr_tx. Data type is known (deducted from the SPI initializer),
	 * number of data to send (buffer length) is set via `length` parameter.
	 * @note Both buffers (RX and TX) must have the same length. All received data will be saved
	 * in the RX buffer. After finished reading one must select meaningful data (i.e. those which
	 * are equal to device's buffer content) and discard those which device sends during Mater transmission.
	 * @param spi_ptr
	 * @param dev_ptr
	 * @param mem_addr_rx
	 * @param mem_addr_tx
	 * @param length
	 * @return
	 */
	uint8_t (*SendReceive)(volatile SOOL_SPI_DMA*, SOOL_SPI_Device *, uint32_t, uint32_t, uint32_t);

	/**
	 * @brief Checks whether a new (full) set of data arrived
	 * @param SOOL_SPI_DMA instance
	 * @return
	 */
	uint8_t (*IsNewData)(volatile SOOL_SPI_DMA*);

	/**
	 * @brief Returns id of the device which called Read
	 * @param
	 * @return
	 */
	uint8_t (*GetNewDataDeviceID)(volatile SOOL_SPI_DMA*);

	// interrupt service routines
	uint8_t (*_DmaRxIrqHandler)(volatile SOOL_SPI_DMA*);
	uint8_t (*_DmaTxIrqHandler)(volatile SOOL_SPI_DMA*);

};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/**
 * @note Some peripherals may interference with each other, see DMA requests for each channel table
 * in reference manual (RM0008, Table 78. Summary of DMA1 requests for each channel, p. 282).
 * @note SPI does not have a buffer itself. Due to different data types, which can be sent via SPI,
 * the buffer (RX or TX) is a separate component associated with a created sensor/IC etc. (device in general).
 * @note IMPORTANT: DO NOT USE SEND METHOD. TC flag is set too early (transfer is still in progress) so device's
 * CS cannot be set HIGH (idle state) until transfer fully finishes. Everything works fine with SendReceive method
 * but it takes buffer space.
 * @param SPIx
 * @param SPI_Direction
 * @param SPI_DataSize
 * @param SPI_CPOL
 * @param SPI_CPHA
 * @param SPI_BaudRatePrescaler
 * @param SPI_FirstBit
 * @return
 * @note Example of use can be found here:
 * https://gitlab.com/frb-pow/002tubewaterflowmcu/commit/d6620f1b1c9b17c5026e6bca05b277238e9560eb
 */
extern volatile SOOL_SPI_DMA SOOL_Periph_SPI_DMA_Init(SPI_TypeDef *SPIx, uint16_t SPI_Direction,
		uint16_t SPI_DataSize, uint16_t SPI_CPOL, uint16_t SPI_CPHA,
		uint16_t SPI_BaudRatePrescaler, uint16_t SPI_FirstBit);

#endif /* INC_SOOL_PERIPHERALS_SPI_SPI_DMA_H_ */
