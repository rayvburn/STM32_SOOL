/*
 * I2C_DMA.c
 *
 *  Created on: 28.11.2019
 *      Author: user
 */

#include "sool/Peripherals/I2C/I2C_DMA.h"
#include "sool/Peripherals/GPIO/PinConfig_AltFunction.h"
#include "stm32f10x_i2c.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

volatile SOOL_I2C_DMA SOOL_Periph_I2C_DMA_Init() {

	// params
	I2C_TypeDef*	I2Cx;
	uint16_t 		I2C_Ack;
	uint16_t		I2C_AcknowledgedAddress;
	uint32_t 		I2C_ClockSpeed; /* <= 400 kHz */
	uint16_t 		I2C_OwnAddress1;

	/* New instance */
	volatile SOOL_I2C_DMA i2c_dma;

	/* Enable peripheral clock */
	if ( I2Cx == I2C1 ) {
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
	} else if ( I2Cx == I2C2 ) {
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);
	} else {
		return (i2c_dma);
	}

	/* DMA configuration */
	DMA_TypeDef* dma_periph;
	DMA_Channel_TypeDef* dma_channel_rx;
	DMA_Channel_TypeDef* dma_channel_tx;

	/* Initialize SDA and SCL pins */
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
		SOOL_Periph_GPIO_PinConfig_Initialize_AltFunction(GPIOB, GPIO_Pin_10, GPIO_Mode_AF_OD); 	// SCL
		SOOL_Periph_GPIO_PinConfig_Initialize_AltFunction(GPIOB, GPIO_Pin_11, GPIO_Mode_AF_OD); 	// SDA
	}

	/* Leftovers from non-object-oriented version */
	// software reset - prevents the bus being stuck
	// I2C_SoftwareResetLight(I2C_PERIPH_ID);	// !

	/* Enable the selected I2C Clock stretching */
	I2C_StretchClockCmd(I2Cx, ENABLE);

	/* Initialize I2Cx peripheral */
	I2C_InitTypeDef i2c;
	I2C_StructInit(&i2c);
	i2c.I2C_Mode = I2C_Mode_I2C;
	i2c.I2C_DutyCycle = I2C_DutyCycle_2;
	i2c.I2C_Ack = I2C_Ack;
	i2c.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress;
	i2c.I2C_ClockSpeed = I2C_ClockSpeed;
	i2c.I2C_OwnAddress1 = I2C_OwnAddress1;
	I2C_Init(I2Cx, &i2c);

	/* Initialize DMA */
//	SOOL_DMA dma = SOOL_Periph_DMA_Init();

	/* Enable I2Cx peripheral */
	I2C_Cmd(I2Cx, ENABLE);

	/* Check if the bus is busy */
//	while ( I2C1->SR2 & I2C_FLAG_BUSY );

	return (i2c_dma);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
