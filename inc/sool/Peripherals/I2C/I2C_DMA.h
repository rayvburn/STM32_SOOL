/*
 * I2C_DMA.h
 *
 *  Created on: 28.11.2019
 *      Author: user
 */

#ifndef INC_SOOL_PERIPHERALS_I2C_I2C_DMA_H_
#define INC_SOOL_PERIPHERALS_I2C_I2C_DMA_H_

#include "sool/Peripherals/DMA/DMA.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

struct _SOOL_I2C_DMA_SetupStruct {
	I2C_TypeDef   *	I2Cx;
	uint8_t 		IRQn;
	uint8_t			size_of;	// size of data element
};

struct _SOOL_I2C_DMA_StateStruct {
};

struct _SOOL_I2C_DMA_StateTxStruct {
};

// with DMA associating data with reading request is tricky (with non-blocking behavior)
struct _SOOL_I2C_DMA_StateRxStruct {
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/* Forward declaration */
typedef struct _SOOL_I2C_DMA_Struct SOOL_I2C_DMA;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/**
 * Implementation for I2C Master device
 */
struct _SOOL_I2C_DMA_Struct {

	// ---------------------------------------------
	SOOL_DMA							base_dma_rx;
	SOOL_DMA							base_dma_tx;
	// ---------------------------------------------

	struct _SOOL_I2C_DMA_SetupStruct	_setup;
	struct _SOOL_I2C_DMA_StateStruct	_state;
	struct _SOOL_I2C_DMA_StateTxStruct	_state_tx;
	struct _SOOL_I2C_DMA_StateRxStruct	_state_rx;

	void (*EnableNVIC)(volatile SOOL_I2C_DMA*);
	void (*DisableNVIC)(volatile SOOL_I2C_DMA*);

	/**
	 * Returns false when transmission is running or is about to start next data transfer (when
	 * sending many bytes).
	 * In other words, if an active transmission exists, returns false.
	 * @param
	 * @return
	 */
	uint8_t (*IsBusy)(volatile SOOL_I2C_DMA*);

	/**
	 * TODO: verify
	 * @brief Sends data pointed by mem_addr_tx. Data type is known (deducted from the SPI initializer),
	 * number of data to send (buffer length) is set via `length` parameter.
	 * @note Both buffers (RX and TX) must have the same length. All received data will be saved
	 * in the RX buffer. After finished reading one must select meaningful data (i.e. those which
	 * are equal to device's buffer content) and discard those which device sends during Mater transmission.
	 * @param spi_ptr
	 * @param mem_addr_rx
	 * @param mem_addr_tx
	 * @param length
	 * @return
	 */
	uint8_t (*SendReceive)(volatile SOOL_I2C_DMA*, uint32_t, uint32_t, uint32_t);


	uint8_t (*Transmit)(volatile SOOL_I2C_DMA *i2c_ptr);
	uint8_t (*Receive);


	/**
	 * @brief Checks whether a new (full) set of data arrived
	 * @param SOOL_I2C_DMA instance
	 * @return
	 */
	uint8_t (*IsNewData)(volatile SOOL_I2C_DMA*);

	// interrupt service routines
	uint8_t (*_DmaRxIrqHandler)(volatile SOOL_I2C_DMA*);
	uint8_t (*_DmaTxIrqHandler)(volatile SOOL_I2C_DMA*);

};

extern volatile SOOL_I2C_DMA SOOL_Periph_I2C_DMA_Init();

#endif /* INC_SOOL_PERIPHERALS_I2C_I2C_DMA_H_ */
