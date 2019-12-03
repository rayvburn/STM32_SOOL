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

	uint8_t (*Transmit)(SOOL_I2C_Polling *i2c_ptr, uint8_t slave_address, uint32_t length, uint8_t* buf_tx);
	uint8_t (*Receive) (SOOL_I2C_Polling *i2c_ptr, uint8_t slave_address, uint32_t length, uint8_t* buf_rx);

};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

extern SOOL_I2C_Polling SOOL_Periph_I2C_Polling_Init(	I2C_TypeDef*	I2Cx,
		uint16_t 		I2C_Ack,
		uint16_t		I2C_AcknowledgedAddress,
		uint32_t 		I2C_ClockSpeed /* <= 400 kHz */,
		uint16_t 		I2C_OwnAddress1);


#endif /* INC_SOOL_PERIPHERALS_I2C_I2C_POLLING_H_ */
