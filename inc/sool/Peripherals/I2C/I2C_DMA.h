/*
 * I2C_DMA.h
 *
 *  Created on: 28.11.2019
 *      Author: user
 */

#ifndef INC_SOOL_PERIPHERALS_I2C_I2C_DMA_H_
#define INC_SOOL_PERIPHERALS_I2C_I2C_DMA_H_

#include "sool/Peripherals/DMA/DMA.h"
#include "sool/Peripherals/I2C/I2C_common.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

struct _SOOL_I2C_DMA_SetupStruct {

	I2C_TypeDef   *	I2Cx;

	// TODO / FIXME
	uint8_t 		IRQn_EV;
	uint8_t 		IRQn_ER;

};

struct _SOOL_I2C_DMA_StateStruct {
};

struct _SOOL_I2C_DMA_StateTxStruct {
	uint8_t 		finished;
};

// with DMA associating data with reading request is tricky (with non-blocking behavior)
struct _SOOL_I2C_DMA_StateRxStruct {
	uint8_t 		finished;
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
	 * @brief Checks whether a new (full) set of data arrived
	 * @param SOOL_I2C_DMA instance
	 * @return
	 */
	uint8_t (*IsNewData)(volatile SOOL_I2C_DMA*);

	// POLLING
	uint8_t (*Transfer)(volatile SOOL_I2C_DMA*, uint8_t I2C_Direction, uint8_t slave_address, uint32_t buf_tx_addr, uint32_t length, uint32_t buf_rx_addr);

	// TODO
	uint8_t (*SendReceive)(volatile SOOL_I2C_DMA *i2c_ptr, uint8_t slave_address, uint32_t buf_tx_addr, uint32_t length, uint32_t buf_rx_addr);

	uint8_t (*MasterTransmitter)(volatile SOOL_I2C_DMA *i2c_ptr, uint8_t slave_address, uint32_t buf_tx_addr, uint32_t length, uint32_t buf_rx_addr);
	uint8_t (*MasterReceiver)(volatile SOOL_I2C_DMA *i2c_ptr, uint8_t slave_address, uint32_t buf_tx_addr, uint32_t length, uint32_t buf_rx_addr);

	// interrupt service routines
	uint8_t (*_DmaRxIrqHandler)(volatile SOOL_I2C_DMA*);
	uint8_t (*_DmaTxIrqHandler)(volatile SOOL_I2C_DMA*);

};

/**
 *
 * @return
 * @note I2C uses channels 4 and 5 OR 6 and 7 of DMA controller. Check Table 78. of RM0008 to verify
 * if there is no collision among peripherals (I2C1 with DMA cannot be used along with USART2,
 * whereas I2C2 cannot be used along with USART1).
 */
extern volatile SOOL_I2C_DMA SOOL_Periph_I2C_DMA_Init(I2C_TypeDef* I2Cx, uint16_t I2C_Ack,
		uint16_t I2C_AcknowledgedAddress, uint32_t I2C_ClockSpeed, uint16_t I2C_OwnAddress1);

#endif /* INC_SOOL_PERIPHERALS_I2C_I2C_DMA_H_ */
