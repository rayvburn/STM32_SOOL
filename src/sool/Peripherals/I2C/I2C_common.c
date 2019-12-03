/*
 * I2C_common.c
 *
 *  Created on: 03.12.2019
 *      Author: user
 */

#include <sool/Peripherals/I2C/I2C_common.h>

uint8_t SOOL_Periph_I2C_InitBasic(	I2C_TypeDef*	I2Cx,
		uint16_t 		I2C_Ack,
		uint16_t		I2C_AcknowledgedAddress,
		uint32_t 		I2C_ClockSpeed /* <= 400 kHz */,
		uint16_t 		I2C_OwnAddress1) {

	// ---- clock -----------------------------------------------------------------------------
	/* Enable peripheral clock */
	if ( I2Cx == I2C1 ) {
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
	} else if ( I2Cx == I2C2 ) {
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);
	} else {
		return (0);
	}

	// ---- GPIO -----------------------------------------------------------------------------
	/* Initialize SDA and SCL pins, prepare DMA configuration */
	if ( I2Cx == I2C1 ) {

		/* Check remapping */
		if ( AFIO->MAPR & AFIO_MAPR_I2C1_REMAP ) { // Reference manual 0008 -> 9.3.9
			SOOL_Periph_GPIO_PinConfig_Initialize_AltFunction(GPIOB, GPIO_Pin_8, GPIO_Mode_AF_OD); 	// SCL
			SOOL_Periph_GPIO_PinConfig_Initialize_AltFunction(GPIOB, GPIO_Pin_9, GPIO_Mode_AF_OD); 	// SDA
		} else {
			SOOL_Periph_GPIO_PinConfig_Initialize_AltFunction(GPIOB, GPIO_Pin_6, GPIO_Mode_AF_OD); 	// SCL
			SOOL_Periph_GPIO_PinConfig_Initialize_AltFunction(GPIOB, GPIO_Pin_7, GPIO_Mode_AF_OD); 	// SDA
		}

	} else if ( I2Cx == I2C2 ) {

		/* Pin configuration */
		SOOL_Periph_GPIO_PinConfig_Initialize_AltFunction(GPIOB, GPIO_Pin_10, GPIO_Mode_AF_OD); 	// SCL
		SOOL_Periph_GPIO_PinConfig_Initialize_AltFunction(GPIOB, GPIO_Pin_11, GPIO_Mode_AF_OD); 	// SDA

	} else {

		return (0);

	}

	// ---- peripheral -----------------------------------------------------------------------------
	/* Enable I2Cx peripheral - another approach -> clock cannot be set then? TODO */
//	I2C_Cmd(I2Cx, ENABLE);

	/* TODO: Software reset
	 * @ref https://community.st.com/s/question/0D50X00009XkiPQSAZ/stm32f407-i2c-error-addr-bit-not-set-after-the-7bit-address-is-sent */
//	I2C_SoftwareResetCmd(I2Cx, ENABLE);

	/* Leftovers from non-object-oriented version */
	// software reset - prevents the bus being stuck
	// I2C_SoftwareResetLight(I2C_PERIPH_ID);	// !

//	/* Enable the selected I2C Clock stretching */
//	I2C_StretchClockCmd(I2Cx, ENABLE);

	/* Reset the I2C interface */
	I2C_DeInit(I2Cx);

	/* Initialize I2Cx peripheral */
	I2C_InitTypeDef i2c;
	I2C_StructInit(&i2c);
	i2c.I2C_Mode = I2C_Mode_I2C;
	i2c.I2C_DutyCycle = I2C_DutyCycle_2;
	i2c.I2C_Ack = I2C_Ack;
	i2c.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress; 	// 7-bit / 10-bit addressing mode
	i2c.I2C_ClockSpeed = I2C_ClockSpeed;
	i2c.I2C_OwnAddress1 = I2C_OwnAddress1;
	I2C_Init(I2Cx, &i2c);

	/* Enable I2Cx peripheral TODO */
	I2C_Cmd(I2Cx, ENABLE);

	return (1);

}
