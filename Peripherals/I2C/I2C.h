/*
 * I2C_Config.h
 *
 *  Created on: 09.10.2018
 *      Author: user
 */

#ifndef I2C_H_
#define I2C_H_

#include "../CMSIS/device/stm32f10x.h"
// #include "DMA.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#define I2C_CLOCK_SPEED			50000u
#define I2C_APB_RCC_ID			RCC_APB1Periph_I2C1	// RCC_APB1Periph_I2C1
#define I2C_PERIPH_ID			I2C1				// I2C1
#define I2C_SCL_PIN				GPIO_Pin_8			// GPIO_Pin_8
#define I2C_SDA_PIN				GPIO_Pin_9			// GPIO_Pin_9
#define I2C_ERR_IRQ_ID			I2C1_ER_IRQn		// I2C1_ER_IRQn
#define I2C_EVT_IRQ_ID			I2C1_EV_IRQn		// I2C1_EV_IRQn

#define I2C_TIMEOUT_VALUE		1000u
#define I2C_ERROR_IDENTIFIER 	0u
// #define I2C_PROCEDURAL_APPROACH
// #define I2C_PINS_BEFORE_CLOCK_INIT

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

extern void 	I2C_Config();

extern ErrorStatus I2C_GenerateStart_Blocking(I2C_TypeDef *I2Cx);
extern ErrorStatus I2C_WriteSlave7BitAddress_Blocking(I2C_TypeDef *I2Cx, uint8_t slave_address);
extern ErrorStatus I2C_SendData_Blocking(I2C_TypeDef *I2Cx, uint8_t data);
extern ErrorStatus I2C_InitializeReading_Blocking(I2C_TypeDef *I2Cx, uint8_t slave_address);
extern ErrorStatus I2C_HandleReadingEvents_Blocking(I2C_TypeDef *I2Cx, uint8_t bytes_to_receive);
extern ErrorStatus I2C_ReadData_Blocking(I2C_TypeDef *I2Cx, uint16_t *data, uint8_t len);
extern ErrorStatus I2C_GenerateStop_Blocking(I2C_TypeDef *I2Cx);



extern ErrorStatus I2C_MasterTransmit (I2C_TypeDef *I2Cx, uint8_t slave_address, uint8_t start_reg, uint8_t *data, uint8_t len);
extern ErrorStatus I2C_MasterReceive  (I2C_TypeDef *I2Cx, uint8_t slave_address, uint8_t start_reg, uint8_t *data, uint8_t len);

// extern ErrorStatus I2C_BeginMasterTransmitterNoInternalReg (uint8_t address, uint8_t reg, uint8_t *data, uint8_t len);
// extern ErrorStatus I2C_BeginMasterTransmitterBackup (uint8_t address, uint8_t reg, uint8_t *data, uint8_t len);
// extern ErrorStatus I2C_BeginMasterReceiver (I2C_TypeDef *i2c_id, uint8_t address, uint8_t data);

extern ErrorStatus 	I2C_SoftwareResetProcedure	(I2C_TypeDef *i2c_id);
extern ErrorStatus 	I2C_SoftwareResetLight		(I2C_TypeDef *i2c_id);
extern uint8_t 	I2C_TimeoutHandler				(I2C_TypeDef *i2c_id);
extern void 	I2C_TimeoutReset();

extern void		I2C1_ER_IRQHandler();		// adjust peripheral's ID!
extern void		I2C1_EV_IRQHandler();		// adjust peripheral's ID!

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#endif /* I2C_H_ */
