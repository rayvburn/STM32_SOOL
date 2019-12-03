/*
 * I2C_Polling.h
 *
 *  Created on: 03.12.2019
 *      Author: user
 */

#ifndef INC_SOOL_PERIPHERALS_I2C_I2C_POLLING_H_
#define INC_SOOL_PERIPHERALS_I2C_I2C_POLLING_H_

#include "sool/Peripherals/I2C/I2C_common.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

struct _SOOL_I2C_Polling_SetupStruct {
	I2C_TypeDef   *	I2Cx;
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/* Forward declaration */
typedef struct _SOOL_I2C_Polling_Struct SOOL_I2C_Polling;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

struct _SOOL_I2C_Polling_Struct {

	struct _SOOL_I2C_Polling_SetupStruct	_setup;

	/**
	 * @brief Runs I2C transmission procedure
	 * @param i2c_ptr: class instance
	 * @param slave_address: address of the slave device (do not change address to select R/W)
	 * @param buf_tx: pointer to the buffer which values will be transmitted
	 * @param length: number of bytes to be transmitted
	 * @return 1 if operation finished successfully, 0 on error
	 */
	uint8_t (*Transmit)(SOOL_I2C_Polling *i2c_ptr, uint8_t slave_address, uint8_t* buf_tx, uint32_t length);

	/**
	 * @brief Runs I2C reception procedure
	 * @param i2c_ptr: class instance
	 * @param slave_address: base address of the slave device (do not change address to select R/W)
	 * @param buf_rx: pointer to the buffer which will be filled with received
	 * data (if everything runs smoothly)
	 * @param length: number of bytes to be received
	 * @return 1 if operation finished successfully, 0 on error
	 */
	uint8_t (*Receive) (SOOL_I2C_Polling *i2c_ptr, uint8_t slave_address, uint8_t* buf_rx, uint32_t length);

};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/**
 * @brief I2C controller running in the polling mode (waits for a flag to be set).
 * Single transfer (of 1 data byte) takes 150-200 us with 100 kHz clock.
 * @note This `class` disables interrupts during Receive procedure (for a few clock cycles),
 * so one must be careful when using this with interrupt-driven instances (interrupt
 * may not be caught).
 * @param I2Cx: specifies I2C peripheral by name.
 * @param I2C_Ack: Enables or disables the acknowledgement.
 * @param I2C_AcknowledgedAddress: Specifies if 7-bit or 10-bit address is acknowledged.
 * @param I2C_ClockSpeed: Specifies the clock frequency (must be lower than 400 kHz).
 * @param I2C_OwnAddress1: Specifies the first device own address.
 * @return `SOOL_I2C_Polling` class instance
 */
extern SOOL_I2C_Polling SOOL_Periph_I2C_Polling_Init(I2C_TypeDef* I2Cx, uint16_t I2C_Ack, uint16_t I2C_AcknowledgedAddress, uint32_t I2C_ClockSpeed, uint16_t I2C_OwnAddress1);


#endif /* INC_SOOL_PERIPHERALS_I2C_I2C_POLLING_H_ */
