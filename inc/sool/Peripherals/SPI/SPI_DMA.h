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

//	/**
//	 * Preloads queue before running multiple transfers (second and following ones are started in ISR).
//	 */
//	void AddToTxQueue(volatile SOOL_SPI_DMA*);

	/**
	 * This will return a new instance of SOOL_SPI_Device.
	 * SOOL_SPI_Device should have a unique identifier assigned so many devices of the same type
	 * could be connected at the same time and it can be distinguished which of them data belongs to. ??
	 * What to do with different buffer data types ??
	 * @param
	 * @return
	 */
	SOOL_SPI_Device (*AddDevice)(volatile SOOL_SPI_DMA*, const GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);

	/**
	 * Returns false when transmission is running or is about to start next data transfer (when
	 * sending many bytes).
	 * In other words, if an active transmission exists, returns false.
	 * @param
	 * @return
	 */
	uint8_t (*IsBusy)(volatile SOOL_SPI_DMA*);

//	/**
//	 * Loads data to SPI Tx buffer and sends it one by one
//	 * @param
//	 * @param
//
//	 * @return
//	 */
//	uint8_t (*Send)(volatile SOOL_SPI_DMA*, volatile SOOL_SPI_Device);

	/**
	 * @brief Send request
	 * @note SPI_Device's buffer should be prepared accordingly first (resized, filled with data etc)
	 * @note sizeof(data_element) is deducted from SPI constructor
	 * @param SOOL_SPI_DMA instance
	 * @param memory address (start)
	 * @param buffer length
	 * @return status
	 */
	uint8_t (*Send)(volatile SOOL_SPI_DMA*, SOOL_SPI_Device*, uint32_t, uint32_t);

//	/**
//	 * This should take SPI_Device's buffer, resize it accordingly (or shrink if needed)
//	 * and start DMA transfer
//	 * @param
//	 * @param
//	 * @param how many
//	 * @return
//	 */
//	uint8_t (*Read)(volatile SOOL_SPI_DMA*, volatile SOOL_SPI_Device, uint8_t);

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
	uint8_t (*Read)(volatile SOOL_SPI_DMA*, SOOL_SPI_Device*, uint32_t, uint32_t);

	// TODO: SwitchInterrupts(volatile SOOL_SPI_DMA*, FunctionalState)
	// TODO: TransmitReceive - add Blocking version with DMA

//	uint8_t (*TransmitReceive)

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

	// interrupt handlers
	uint8_t (*_DmaRxIrqHandler)(volatile SOOL_SPI_DMA*);
	uint8_t (*_DmaTxIrqHandler)(volatile SOOL_SPI_DMA*);

};

// FIXME: there must be an `unknown` ID for device, like `0` to recognize temporary situation
// when Read is called and in main loop GetNewDataDeviceID is checked

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// SPI does not have a buffer itself. Due to different data types, which can be sent via SPI,
// the buffer (RX or TX) is a separate component associated with a created sensor/IC etc.
//* @note Some peripherals may interference with each other, see DMA requests for each channel table
//* in reference manual (RM0008, Table 78. Summary of DMA1 requests for each channel, p. 282).
/**
 *
 * @param SPIx
 * @param SPI_Direction
 * @param SPI_DataSize
 * @param SPI_CPOL
 * @param SPI_CPHA
 * @param SPI_BaudRatePrescaler
 * @param SPI_FirstBit
 * @return
 */
extern volatile SOOL_SPI_DMA SOOL_Periph_SPI_DMA_Init(SPI_TypeDef *SPIx, uint16_t SPI_Direction,
		uint16_t SPI_DataSize, uint16_t SPI_CPOL, uint16_t SPI_CPHA,
		uint16_t SPI_BaudRatePrescaler, uint16_t SPI_FirstBit);

#endif /* INC_SOOL_PERIPHERALS_SPI_SPI_DMA_H_ */
