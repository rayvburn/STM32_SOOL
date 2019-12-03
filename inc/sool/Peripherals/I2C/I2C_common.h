/*
 * I2C_common.h
 *
 *  Created on: 03.12.2019
 *      Author: user
 */

#ifndef INC_SOOL_PERIPHERALS_I2C_I2C_COMMON_H_
#define INC_SOOL_PERIPHERALS_I2C_I2C_COMMON_H_

#include "sool/Peripherals/GPIO/PinConfig_AltFunction.h"
#include "stm32f10x_i2c.h"
#include "stdint.h"

/**
 * @brief Initializes I2C peripheral GPIO clock and pins as well as peripheral itself
 * @param I2Cx: specifies I2C peripheral by name.
 * @param I2C_Ack: Enables or disables the acknowledgement.
 * @param I2C_AcknowledgedAddress: Specifies if 7-bit or 10-bit address is acknowledged.
 * @param I2C_ClockSpeed: Specifies the clock frequency (must be lower than 400 kHz).
 * @param I2C_OwnAddress1: Specifies the first device own address.
 * @return
 */
extern uint8_t SOOL_Periph_I2C_InitBasic(	I2C_TypeDef*	I2Cx,
		uint16_t 		I2C_Ack,
		uint16_t		I2C_AcknowledgedAddress,
		uint32_t 		I2C_ClockSpeed /* <= 400 kHz */,
		uint16_t 		I2C_OwnAddress1);

#endif /* INC_SOOL_PERIPHERALS_I2C_I2C_COMMON_H_ */
