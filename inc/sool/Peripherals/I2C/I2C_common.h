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

extern uint8_t SOOL_Periph_I2C_InitBasic(	I2C_TypeDef*	I2Cx,
		uint16_t 		I2C_Ack,
		uint16_t		I2C_AcknowledgedAddress,
		uint32_t 		I2C_ClockSpeed /* <= 400 kHz */,
		uint16_t 		I2C_OwnAddress1);

#endif /* INC_SOOL_PERIPHERALS_I2C_I2C_COMMON_H_ */
