/*
 * I2C.c
 *
 *  Created on: 09.10.2018
 *      Author: user
 */

#include "I2C.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint16_t timeout = I2C_TIMEOUT_VALUE;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void I2C_Config() {

	// init RCCs
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB1PeriphClockCmd(I2C_APB_RCC_ID, ENABLE);

	// init gpio BEFORE I2C CLOCK
#ifdef I2C_PINS_BEFORE_CLOCK_INIT
	GPIO_InitTypeDef gpio;
	GPIO_StructInit(&gpio);
	gpio.GPIO_Pin = I2C_SCL_PIN | I2C_SDA_PIN; // SCL, SDA
	gpio.GPIO_Mode = GPIO_Mode_AF_OD;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &gpio);
#endif

	// software reset - prevents the bus being stuck
	// I2C_SoftwareResetLight(I2C_PERIPH_ID);	// !

	// init I2C peripheral
	I2C_StretchClockCmd(I2C_PERIPH_ID, ENABLE);
	I2C_InitTypeDef i2c;
	I2C_StructInit(&i2c);
	i2c.I2C_Mode = I2C_Mode_I2C;
	i2c.I2C_DutyCycle = I2C_DutyCycle_2;
	i2c.I2C_Ack = I2C_Ack_Enable;
	i2c.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	i2c.I2C_ClockSpeed = I2C_CLOCK_SPEED;
	I2C_Init(I2C_PERIPH_ID, &i2c);

#ifdef I2C_PROCEDURAL_APPROACH

	#if !defined(I2C_USE_DMA_TRANSMIT) && !defined(I2C_USE_DMA_READ)

	I2C_ITConfig(I2C_PERIPH_ID, I2C_IT_BUF | I2C_IT_EVT | I2C_IT_ERR, ENABLE);
	NVIC_InitTypeDef nvic;
	nvic.NVIC_IRQChannel = I2C_ERR_IRQ_ID | I2C_EVT_IRQ_ID;
	nvic.NVIC_IRQChannelPreemptionPriority = 15;	// highest priority
	nvic.NVIC_IRQChannelSubPriority = 15;			// highest priority
	nvic.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvic);

	#endif

#else

	// I2C_ITConfig(I2C_PERIPH_ID, I2C_IT_BUF | I2C_IT_EVT | I2C_IT_ERR, ENABLE);
	NVIC_InitTypeDef nvic;
	nvic.NVIC_IRQChannel = I2C_ERR_IRQ_ID | I2C_EVT_IRQ_ID;
	nvic.NVIC_IRQChannelPreemptionPriority = 0;
	nvic.NVIC_IRQChannelSubPriority = 0;
	nvic.NVIC_IRQChannelCmd = ENABLE;
	// NVIC_Init(&nvic);

#endif

	I2C_Cmd(I2C_PERIPH_ID, ENABLE);

#ifndef I2C_PINS_BEFORE_CLOCK_INIT

	GPIO_InitTypeDef gpio;
	GPIO_StructInit(&gpio);
	gpio.GPIO_Pin = I2C_SCL_PIN | I2C_SDA_PIN; // SCL, SDA
	gpio.GPIO_Mode = GPIO_Mode_AF_OD;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &gpio);

#endif

	// checks if bus is not BUSY
	// I2C_SR2
	while ( I2C1->SR2 & I2C_FLAG_BUSY );

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

ErrorStatus I2C_GenerateStart_Blocking(I2C_TypeDef *I2Cx) {

	/*
	// check if bus is not busy
	while ( I2Cx->SR2 & I2C_FLAG_BUSY ) {
		// try to reset?
		return ERROR;
	}

	// Figure 273, ST's RM0008 (
	I2Cx->CR1 |= I2C_CR1_START;				// generate start

	while( !( I2Cx->SR1 & I2C_SR1_SB )); 	// wait for SB flag to be written (EV5)
	return SUCCESS;
	*/

	ErrorStatus status = SUCCESS;
	uint16_t tries_counter = 0;
	while ( status == SUCCESS ) {

		// check if bus is not busy
		while ( I2Cx->SR2 & I2C_FLAG_BUSY ) {
			// try to reset?
			return ERROR;
		}

		// Figure 273, ST's RM0008 (
		I2Cx->CR1 |= I2C_CR1_START;				// generate start

		while( !( I2Cx->SR1 & I2C_SR1_SB )) { 	// wait for SB flag to be written (EV5)
			I2C_SoftwareResetLight(I2Cx);
			tries_counter++;
			if ( tries_counter > 100 ) {
				return ERROR;
			}
		}
		return SUCCESS;

	}

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

ErrorStatus I2C_WriteSlave7BitAddress_Blocking(I2C_TypeDef *I2Cx, uint8_t slave_address) {

	uint32_t temp;
	temp = I2C1->SR1;						// read the SR1 register...
	I2Cx->DR = slave_address;				// and write slave's address into DR register to clear SB flag
	while(  ( I2Cx->SR1 & I2C_SR1_SB ));	// wait until SB flag is cleared by hardware

	// EV6
	while( !( I2Cx->SR1 & I2C_SR1_ADDR ));	// wait for ADDR (EV6) which is set after Ack received from slave
	temp = I2Cx->SR1;						// read SR1 and then
	temp = I2Cx->SR2;						// SR2 to clear ADDR flag
	while(  ( I2Cx->SR1 & I2C_SR1_ADDR ));	// wait for ADDR to be cleared by hardware...
	return SUCCESS;

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

ErrorStatus I2C_SendData_Blocking(I2C_TypeDef *I2Cx, uint8_t data) {

	// EV8_1
	while( !( I2Cx->SR1 & I2C_SR1_TXE ));	// wait for setting the TxE (EV8_1) bit
	I2Cx->DR = data;						// write register value to DR - EV8_1 means shift register empty, data reg empty, waits for writing data1 in DR
	while( !( I2Cx->SR1 & I2C_SR1_BTF ));	// wait for BTF (byte transfer finished) to be set
	return SUCCESS;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

ErrorStatus I2C_InitializeReading_Blocking(I2C_TypeDef *I2Cx, uint8_t slave_address) {

	// !! -- VL6180X-specific -- !!
	//I2Cx->CR1 |= I2C_CR1_ACK;

	uint32_t temp;
	I2Cx->CR1 |= I2C_CR1_START;				// generate repeated start
	while( !( I2Cx->SR1 & I2C_SR1_SB ));	// ...
	temp = I2Cx->SR1;
	I2Cx->DR = slave_address | 0x01;
	while( !( I2Cx->SR1 & I2C_SR1_ADDR ));
	temp = I2Cx->SR1;
	temp = I2Cx->SR2;
	while(  ( I2Cx->SR1 & I2C_SR1_ADDR ));
	// I2C into master receiver mode

	return SUCCESS;

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

ErrorStatus I2C_HandleReadingEvents_Blocking(I2C_TypeDef *I2Cx, uint8_t bytes_to_receive) {

	uint32_t temp;

	// cases described in RM0008, p. 760-761
	/* Legend:
	   X -> means that it is handled in this function
	   - -> means that it is managed in the reading
	 */

	if ( bytes_to_receive > 2 ) {

		// needs to be handled differently as the number of bytes left to read goes down

	} else if ( bytes_to_receive == 2 ) {

		/* Case of TWO bytes to be received:
		X Set POS and ACK
		X Wait for the ADDR flag to be set
		X Clear ADDR
		– Clear ACK
		- Wait for BTF to be set
		– Program STOP
		– Read DR twice */

		I2Cx->CR1 |= I2C_CR1_POS;
		I2Cx->CR1 |= I2C_CR1_ACK;
		while( !( I2Cx->SR1 & I2C_SR1_ADDR ));
		temp = I2Cx->SR1;
		temp = I2Cx->SR2;
		while(  ( I2Cx->SR1 & I2C_SR1_ADDR ));	// ADDR cleared


	} else if ( bytes_to_receive == 1 ) {

		/* Case of a SINGLE byte to be received:
		X In the ADDR event, clear the ACK bit.
		X Clear ADDR
		– Program the STOP/START bit.
		– Read the data after the RxNE flag is set.
		 */

		while( !( I2Cx->SR1 & I2C_SR1_ADDR )); 	// check if ADDR event occurred
		I2Cx->CR1 &= ~(I2C_CR1_ACK);			// clear ACK
		temp = I2Cx->SR1;						// clear ADDR
		temp = I2Cx->SR2;						// ...
		while(  ( I2Cx->SR1 & I2C_SR1_ADDR ));	// ADDR cleared

	}

	return SUCCESS;

}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

ErrorStatus I2C_ReadData_Blocking(I2C_TypeDef *I2Cx, uint16_t *data, uint8_t len) {

	/*
	while( len )
	{
	   if( len == 1 )
		   I2Cx->CR1 &= ~I2C_CR1_ACK;

	   while( !( I2Cx->SR1 & I2C_SR1_RXNE ));
	   *( data++ ) = I2Cx->DR;
	   // data_test = I2Cx->DR;

	   len--;
	}
	return SUCCESS;
	*/

	// Method 2 p 759 RM0008
/*
	uint8_t temp = 0;
	uint8_t to_workaround = 0;

	if ( len > 2 ) {
		to_workaround = 1; // DataN-2 is not read, p.759
	}

	while( len )
	{

		if ( to_workaround ) {

			if( len == 2 ) {
				I2Cx->CR1 &= ~I2C_CR1_ACK;
			}

		} else {

			if ( len == 1 ) {
				I2Cx->CR1 &= ~I2C_CR1_ACK;
			}

		}

	   while( !( I2Cx->SR1 & I2C_SR1_RXNE ));

	   // p.760 RM0008
	   if ( to_workaround && len == 3 ) {
		   while (!(I2Cx->SR1 & I2C_SR1_BTF));
		   I2Cx->CR1 &= ~I2C_CR1_ACK;
		   *( data++ ) = I2Cx->DR;
		   len -= 2;
		   continue;
	   }

	   if ( I2Cx->SR1 & I2C_SR1_BTF ) {
		   temp = I2Cx->SR1; // just in case, to reset BTF
	   }
	   *( data++ ) = I2Cx->DR;


	   if ( to_workaround && len == 1 ) {
		   I2Cx->CR1 |= I2C_CR1_STOP;
		   *( data++ ) = I2Cx->DR;
	   }


	   len--;
	}
	return SUCCESS;
*/

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// for better readability

	// cases described in RM0008, p. 760-761
	/* Legend:
	   . -> means that it was handled in the handlevent function
	   X -> means that it is handled in this function
	 */

	if ( len > 2 ) {

		// workaround method, p.760 RM0008 - DataN-2 not read
		// needs to be handled differently as the number of bytes left to read goes down
		uint8_t bytes_remaining = len;

		while ( bytes_remaining ) {

			if ( bytes_remaining == 3 ) {

				/*
				When 3 bytes remain to be read:
				• RxNE = 1 => Nothing (DataN-2 not read).
				• DataN-1 received
				• BTF = 1 because both shift and data registers are full: DataN-2 in DR and DataN-1 in
				the shift register => SCL tied low: no other data will be received on the bus.
				• Clear ACK bit
				• Read DataN-2 in DR => This will launch the DataN reception in the shift register
				• DataN received (with a NACK)
				• Program START/STOP
				• Read DataN-1
				• RxNE = 1
				• Read DataN
				 */

				while( !( I2Cx->SR1 & I2C_SR1_RXNE ));	// RXNE not set yet, so wait
				while(  ( I2Cx->SR1 & I2C_SR1_RXNE )) {	// RXNE already set, wait for BTF to be set
					// check if DataN-1 was received
					if ( I2Cx->SR1 & I2C_SR1_BTF ) {
						break;							// break internal while loop
					}
				}
				I2Cx->CR1 &= ~I2C_CR1_ACK;
				*( data++ ) = I2Cx->DR;
				I2Cx->CR1 |= I2C_CR1_STOP;
				*( data++ ) = I2Cx->DR;
				while( !( I2Cx->SR1 & I2C_SR1_RXNE ));
				*( data++ ) = I2Cx->DR;

				return SUCCESS;							// end the whole reading data loop and return from function

			} else {

				// Figure 275.
				while( !( I2Cx->SR1 & I2C_SR1_RXNE ));	// wait for EV7
				*( data++ ) = I2Cx->DR;					// clear EV7 flag by reading DR register

			}

			bytes_remaining--;
		}


	} else if ( len == 2  ) {

		// 2 bytes transmitted only, p. 761

		/* Case of TWO bytes to be received:
		. Set POS and ACK
		. Wait for the ADDR flag to be set
		. Clear ADDR
		X Clear ACK
		X Wait for BTF to be set
		X Program STOP
		X Read DR twice */

	   I2Cx->CR1 &= ~I2C_CR1_ACK;
	   while( !( I2Cx->SR1 & I2C_SR1_BTF ));
	   I2Cx->CR1 |= I2C_CR1_STOP;
	   *( data   ) = I2Cx->DR;
	   *( data+1 ) = I2Cx->DR;


	} else if ( len == 1 ) {

		// single byte transmitted from slave, p. 761

		/* Case of a SINGLE byte to be received:
		. In the ADDR event, clear the ACK bit.
		. Clear ADDR
		X Program the STOP/START bit.
		X Read the data after the RxNE flag is set.
		 */

		I2Cx->CR1 |= I2C_CR1_STOP;
		while( !( I2Cx->SR1 & I2C_SR1_RXNE ));
		*( data   ) = I2Cx->DR;

	}

	return SUCCESS;


}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

ErrorStatus I2C_GenerateStop_Blocking(I2C_TypeDef *I2Cx) {

	I2Cx->CR1 |= I2C_CR1_STOP;
	return SUCCESS;

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

ErrorStatus I2C_MasterTransmit (I2C_TypeDef *I2Cx, uint8_t slave_address, uint8_t start_reg,
								uint8_t *data, uint8_t len) {

	uint32_t temp;

	// check if bus is not busy
	while ( I2Cx->SR2 & I2C_FLAG_BUSY ) {
		// try to reset?
		return ERROR;
	}

	// Figure 273, ST's RM0008 (
	I2Cx->CR1 |= I2C_CR1_START;				// generate start

	while( !( I2Cx->SR1 & I2C_SR1_SB )); 	// wait for SB flag to be written (EV5)
	temp = I2C1->SR1;						// read the SR1 register...
	I2Cx->DR = slave_address;				// and write slave's address into DR register to clear SB flag
	while(  ( I2Cx->SR1 & I2C_SR1_SB ));	// wait until SB flag is cleared by hardware

	while( !( I2Cx->SR1 & I2C_SR1_ADDR ));	// wait for ADDR (EV6) which is set after Ack received from slave
	temp = I2Cx->SR1;						// read SR1 and then
	temp = I2Cx->SR2;						// SR2 to clear ADDR flag
	while(  ( I2Cx->SR1 & I2C_SR1_ADDR ));	// wait for ADDR to be cleared by hardware...

	while( !( I2Cx->SR1 & I2C_SR1_TXE ));	// wait for setting the TxE (EV8_1) bit
	I2Cx->DR = start_reg;					// write register value to DR - EV8_1 means shift register empty, data reg empty, waits for writing data1 in DR
	while( !( I2Cx->SR1 & I2C_SR1_BTF ));	// wait for BTF to be set

	I2Cx->CR1 |= I2C_CR1_START;				// generate repeated start
	while( !( I2Cx->SR1 & I2C_SR1_SB ));	// ...
	temp = I2Cx->SR1;
	I2Cx->DR = slave_address | 0x01;
	while( !( I2Cx->SR1 & I2C_SR1_ADDR ));
	temp = I2Cx->SR1;
	temp = I2Cx->SR2;

	while( len )
	{
	   if( len == 1 )
		   I2Cx->CR1 &= ~I2C_CR1_ACK;

	   while( !( I2Cx->SR1 & I2C_SR1_RXNE ));
	   *( data++ ) = I2Cx->DR;

	   len--;
	}

	I2Cx->CR1 |= I2C_CR1_STOP;
	return SUCCESS;

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

ErrorStatus I2C_MasterReceive (I2C_TypeDef *I2Cx, uint8_t slave_address, uint8_t start_reg,
							   uint8_t *data, uint8_t len) {

	uint32_t temp;

	// check if bus is not busy
	while ( I2Cx->SR2 & I2C_FLAG_BUSY ) {
		// try to reset?
		return ERROR;
	}

	// Figure 273, ST's RM0008 (
	I2Cx->CR1 |= I2C_CR1_START;				// generate start

	while( !( I2Cx->SR1 & I2C_SR1_SB )); 	// wait for SB flag to be written (EV5)
	temp = I2C1->SR1;						// read the SR1 register...
	I2Cx->DR = slave_address;				// and write slave's address into DR register to clear SB flag
	while(  ( I2Cx->SR1 & I2C_SR1_SB ));	// wait until SB flag is cleared by hardware

	while( !( I2Cx->SR1 & I2C_SR1_ADDR ));	// wait for ADDR (EV6) which is set after Ack received from slave
	temp = I2Cx->SR1;						// read SR1 and then
	temp = I2Cx->SR2;						// SR2 to clear ADDR flag
	while(  ( I2Cx->SR1 & I2C_SR1_ADDR ));	// wait for ADDR to be cleared by hardware...

	while( !( I2Cx->SR1 & I2C_SR1_TXE ));	// wait for setting the TxE (EV7) bit
	I2Cx->DR = start_reg;					// write register value to DR
	while( !( I2Cx->SR1 & I2C_SR1_BTF ));	// wait for BTF to be set

	I2Cx->CR1 |= I2C_CR1_START;				// generate repeated start
	while( !( I2Cx->SR1 & I2C_SR1_SB ));	// ...
	temp = I2Cx->SR1;
	I2Cx->DR = slave_address | 0x01;		// LSB high to read data
	while( !( I2Cx->SR1 & I2C_SR1_ADDR ));
	temp = I2Cx->SR1;
	temp = I2Cx->SR2;
	while(  ( I2Cx->SR1 & I2C_SR1_ADDR ));	// wait for ADDR to be cleared by hardware...

	while( len )
	{
	   if( len == 1 )
		   I2Cx->CR1 &= ~I2C_CR1_ACK;

	   while( !( I2Cx->SR1 & I2C_SR1_RXNE ));
	   *( data++ ) = I2Cx->DR;

	   len--;
	}

	I2Cx->CR1 |= I2C_CR1_STOP;
	return SUCCESS;

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

ErrorStatus I2C_SoftwareResetProcedure	(I2C_TypeDef *i2c_id) {

	// 1. Disable the I2C peripheral by clearing the PE bit in I2Cx_CR1 register
	I2C_Cmd(i2c_id, DISABLE);

	// 2. Configure the SCL and SDA I/Os as General Purpose Output Open-Drain, High level (Write 1 to GPIOx_ODR)
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, DISABLE);
	GPIO_InitTypeDef gpio;
	GPIO_StructInit(&gpio);
	gpio.GPIO_Pin = I2C_SCL_PIN | I2C_SDA_PIN; // SCL, SDA
	gpio.GPIO_Mode = GPIO_Mode_Out_OD;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	GPIO_Init(GPIOB, &gpio);
	GPIO_SetBits(GPIOB, I2C_SCL_PIN);
	GPIO_SetBits(GPIOB, I2C_SDA_PIN);

	// 3. Check SCL and SDA High level in GPIOx_IDR.
	while ( GPIO_ReadInputDataBit(GPIOB, I2C_SCL_PIN ) != 1 );
	while ( GPIO_ReadInputDataBit(GPIOB, I2C_SDA_PIN ) != 1 );

	// 4. Configure the SDA I/O as General Purpose Output Open-Drain, Low level (Write 0 to GPIOx_ODR)
	GPIO_StructInit(&gpio);
	gpio.GPIO_Pin = I2C_SDA_PIN; // SDA
	gpio.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_Init(GPIOB, &gpio);
	GPIO_ResetBits(GPIOB, I2C_SDA_PIN);

	// 5. Check SDA Low level in GPIOx_IDR
	while ( GPIO_ReadInputDataBit(GPIOB, I2C_SDA_PIN ) != 0 );

	// 6. Configure the SCL I/O as General Purpose Output Open-Drain, Low level (Write 0 to GPIOx_ODR)
	GPIO_StructInit(&gpio);
	gpio.GPIO_Pin = I2C_SCL_PIN; // SCL
	gpio.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_Init(GPIOB, &gpio);
	GPIO_ResetBits(GPIOB, I2C_SCL_PIN);

	// 7. Check SCL Low level in GPIOx_IDR
	while ( GPIO_ReadInputDataBit(GPIOB, I2C_SCL_PIN ) != 0 );

	// 8. Configure the SCL I/O as General Purpose Output Open-Drain, High level (Write 1 to GPIOx_ODR)
	GPIO_StructInit(&gpio);
	gpio.GPIO_Pin = I2C_SCL_PIN; // SCL
	gpio.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_Init(GPIOB, &gpio);
	GPIO_SetBits(GPIOB, I2C_SCL_PIN);

	// 9. Check SCL High level in GPIOx_IDR.
	while ( GPIO_ReadInputDataBit(GPIOB, I2C_SCL_PIN ) != 1 );

	// 10. Configure the SDA I/O as General Purpose Output Open-Drain , High level (Write 1 to GPIOx_ODR).
	GPIO_StructInit(&gpio);
	gpio.GPIO_Pin = I2C_SDA_PIN; // SDA
	gpio.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_Init(GPIOB, &gpio);
	GPIO_SetBits(GPIOB, I2C_SDA_PIN);

	// 11. Check   SDA   High   level in GPIOx_IDR.
	while ( GPIO_ReadInputDataBit(GPIOB, I2C_SDA_PIN ) != 1 );

	// 12. Configure the SCL and SDA I/Os as Alternate function Open-Drain.
	GPIO_StructInit(&gpio);
	gpio.GPIO_Pin = I2C_SCL_PIN | I2C_SDA_PIN; // SCL, SDA
	gpio.GPIO_Mode = GPIO_Mode_AF_OD;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &gpio);

	// 13. Set SWRST bit in I2Cx_CR1 register.	// I2C1->CR1 |= CR1_SWRST_Set;
	I2C_SoftwareResetCmd(i2c_id, ENABLE);

	// 14. Clear SWRST bit in I2Cx_CR1 register
	I2C_SoftwareResetCmd(i2c_id, DISABLE);

	// 15.  Enable the I2C peripheral by setting the PE bit in I2Cx_CR1 register
	I2C_Cmd(i2c_id, ENABLE);

	return SUCCESS;

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

uint8_t I2C_TimeoutHandler(I2C_TypeDef *i2c_id) {

	return 0;

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

inline void I2C_TimeoutReset() {
	timeout = I2C_TIMEOUT_VALUE;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

ErrorStatus I2C_SoftwareResetLight(I2C_TypeDef *i2c_id) {

	I2C_SoftwareResetCmd(i2c_id, ENABLE);
	for ( int i = 0; i < 10; i++ ) {};
	I2C_SoftwareResetCmd(i2c_id, DISABLE);
	for ( int i = 0; i < 10; i++ ) {};
	return SUCCESS;

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void I2C1_ER_IRQHandler() {

	// I2C_SR1
	if ( I2C_GetITStatus(I2C_PERIPH_ID, I2C_IT_BERR) == SET) {
		I2C_ClearITPendingBit(I2C_PERIPH_ID, I2C_IT_BERR);

	} else if (I2C_GetITStatus(I2C_PERIPH_ID, I2C_IT_OVR) == SET) {
		I2C_ClearITPendingBit(I2C_PERIPH_ID, I2C_IT_OVR);

	} else if (I2C_GetITStatus(I2C_PERIPH_ID, I2C_IT_AF) == SET) {
		I2C_ClearITPendingBit(I2C_PERIPH_ID, I2C_IT_AF);

	} else if (I2C_GetITStatus(I2C_PERIPH_ID, I2C_IT_ARLO) == SET) {
		I2C_ClearITPendingBit(I2C_PERIPH_ID, I2C_IT_ARLO);
	}

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void I2C1_EV_IRQHandler() {

	if ( I2C_GetITStatus(I2C_PERIPH_ID, I2C_IT_ADD10) == SET) {
		I2C_ClearITPendingBit(I2C_PERIPH_ID, I2C_IT_ADD10);

	} else if (I2C_GetITStatus(I2C_PERIPH_ID, I2C_IT_ADDR) == SET) {
		I2C_ClearITPendingBit(I2C_PERIPH_ID, I2C_IT_ADDR);

	} else if (I2C_GetITStatus(I2C_PERIPH_ID, I2C_IT_BTF) == SET) {
		I2C_ClearITPendingBit(I2C_PERIPH_ID, I2C_IT_BTF);

	} else if (I2C_GetITStatus(I2C_PERIPH_ID, I2C_IT_BUF) == SET) {
		I2C_ClearITPendingBit(I2C_PERIPH_ID, I2C_IT_BUF);

	} else if (I2C_GetITStatus(I2C_PERIPH_ID, I2C_IT_ERR) == SET) {
		I2C_ClearITPendingBit(I2C_PERIPH_ID, I2C_IT_ERR);

	} else if (I2C_GetITStatus(I2C_PERIPH_ID, I2C_IT_EVT) == SET) {
		I2C_ClearITPendingBit(I2C_PERIPH_ID, I2C_IT_EVT);

	} else if (I2C_GetITStatus(I2C_PERIPH_ID, I2C_IT_PECERR) == SET) {
		I2C_ClearITPendingBit(I2C_PERIPH_ID, I2C_IT_PECERR);

	} else if (I2C_GetITStatus(I2C_PERIPH_ID, I2C_IT_RXNE) == SET) {
		I2C_ClearITPendingBit(I2C_PERIPH_ID, I2C_IT_RXNE);

	} else if (I2C_GetITStatus(I2C_PERIPH_ID, I2C_IT_SB) == SET) {
		I2C_ClearITPendingBit(I2C_PERIPH_ID, I2C_IT_SB);

	} else if (I2C_GetITStatus(I2C_PERIPH_ID, I2C_IT_SMBALERT) == SET) {
		I2C_ClearITPendingBit(I2C_PERIPH_ID, I2C_IT_SMBALERT);

	} else if (I2C_GetITStatus(I2C_PERIPH_ID, I2C_IT_STOPF) == SET) {
		I2C_ClearITPendingBit(I2C_PERIPH_ID, I2C_IT_STOPF);

	} else if (I2C_GetITStatus(I2C_PERIPH_ID, I2C_IT_TIMEOUT) == SET) {
		I2C_ClearITPendingBit(I2C_PERIPH_ID, I2C_IT_TIMEOUT);

	} else if (I2C_GetITStatus(I2C_PERIPH_ID, I2C_IT_TXE) == SET) {
		I2C_ClearITPendingBit(I2C_PERIPH_ID, I2C_IT_TXE);
	}

}


