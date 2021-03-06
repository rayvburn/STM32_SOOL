/*
 * I2C_Polling.c
 *
 *  Created on: 03.12.2019
 *      Author: user
 */

#include <sool/Peripherals/I2C/I2C_Polling.h>

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t SOOL_I2C_Transmit(SOOL_I2C_Polling *i2c_ptr, uint8_t slave_address, uint8_t* buf_tx, uint32_t length);
static uint8_t SOOL_I2C_Receive(SOOL_I2C_Polling *i2c_ptr, uint8_t slave_address, uint8_t* buf_rx, uint32_t length);

// helpers
static uint8_t SOOL_I2C_Receive1Byte(I2C_TypeDef* I2Cx, uint8_t SlaveAddress, uint8_t* pBuffer);
static uint8_t SOOL_I2C_Receive2Bytes(I2C_TypeDef* I2Cx, uint8_t SlaveAddress, uint8_t* pBuffer);
static uint8_t SOOL_I2C_ReceiveMoreThan2Bytes(I2C_TypeDef* I2Cx, uint8_t SlaveAddress, uint8_t* pBuffer, uint32_t NumByteToRead);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

extern SOOL_I2C_Polling SOOL_Periph_I2C_Polling_Init(I2C_TypeDef* I2Cx, uint16_t I2C_Ack,
		uint16_t I2C_AcknowledgedAddress, uint32_t I2C_ClockSpeed, uint16_t I2C_OwnAddress1)
{

	/* New instance */
	SOOL_I2C_Polling i2c;

	/* GPIO and I2C peripheral init */
	SOOL_Periph_I2C_InitBasic(I2Cx, I2C_Ack, I2C_AcknowledgedAddress, I2C_ClockSpeed, I2C_OwnAddress1);

	/* Save `Setup` structure fields */
	i2c._setup.I2Cx = I2Cx;

	/* Save function pointers */
	i2c.Transmit = SOOL_I2C_Transmit;
	i2c.Receive  = SOOL_I2C_Receive;

	/* Check if the bus is busy */
	while ( I2Cx->SR2 & I2C_FLAG_BUSY );

	return (i2c);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


static uint8_t SOOL_I2C_Transmit(SOOL_I2C_Polling *i2c_ptr, uint8_t slave_address, uint8_t* buf_tx, uint32_t length) {

	uint32_t Timeout = 0xFFFF;
	uint8_t Address = 0x00;
	uint32_t temp = 0;

	I2C_TypeDef* I2Cx = i2c_ptr->_setup.I2Cx;
	uint8_t SlaveAddress = slave_address;
	uint32_t NumByteToWrite = length;
	uint8_t* pBuffer = buf_tx;

	// - - - - - - - - - - - - - - - - - - - - -

	/* Enable Error IT (used in all modes: DMA, Polling and Interrupts */
	I2Cx->CR2 |= I2C_IT_ERR;

	// - - - - - - - - - - - - - - - - - - - - -

	/* I2Cx Master Transmission using Polling */
	Timeout = 0xFFFF;
	/* Send START condition */
	I2Cx->CR1 |= I2C_CR1_START; // CR1_START_Set;
	/* Wait until SB flag is set: EV5 */
	while ((I2Cx->SR1&0x0001) != 0x0001)
	{
		if (Timeout-- == 0)
			return (0);
	}

	/* Send slave address */
	/* Reset the address bit0 for write*/
	SlaveAddress &= (uint16_t)(~(uint16_t)I2C_OAR1_ADD0); // OAR1_ADD0_Reset;
	Address = SlaveAddress;
	/* Send the slave address */
	I2Cx->DR = Address;
	Timeout = 0xFFFF;
	/* Wait until ADDR is set: EV6 */
	while ((I2Cx->SR1 &0x0002) != 0x0002)
	{
		if (Timeout-- == 0)
			return (0);
	}

	/* Clear ADDR flag by reading SR2 register */
	temp = I2Cx->SR2;
	/* Write the first data in DR register (EV8_1) */
	I2Cx->DR = *pBuffer;
	/* Increment */
	pBuffer++;
	/* Decrement the number of bytes to be written */
	NumByteToWrite--;
	/* While there is data to be written */
	while (NumByteToWrite--)
	{
		/* Poll on BTF to receive data because in polling mode we can not guarantee the
		  EV8 software sequence is managed before the current byte transfer completes */
		while ((I2Cx->SR1 & 0x00004) != 0x000004);
		/* Send the current byte */
		I2Cx->DR = *pBuffer;
		/* Point to the next byte to be written */
		pBuffer++;
	}
	/* EV8_2: Wait until BTF is set before programming the STOP */
	while ((I2Cx->SR1 & 0x00004) != 0x000004);
	/* Send STOP condition */
	I2Cx->CR1 |= I2C_CR1_STOP; // CR1_STOP_Set;
	/* Make sure that the STOP bit is cleared by Hardware */
	while ((I2Cx->CR1&0x200) == 0x200);

	return (1);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t SOOL_I2C_Receive(SOOL_I2C_Polling *i2c_ptr, uint8_t slave_address, uint8_t* buf_rx, uint32_t length) {

	/* I2Cx Master Reception using Polling */
	// Taken from ST's I2C optimized examples
	// https://github.com/KingArthurZ3/DeadReckoning-Bare/blob/master/01_Accelerometer/STM32F10x_AN2824_FW_V4.0.0/Project/OptimizedI2Cexamples/src/I2CRoutines.c

	/* Enable I2C errors interrupts (used in all modes: Polling, DMA and Interrupts */
	i2c_ptr->_setup.I2Cx->CR2 |= I2C_IT_ERR;

	// Reception routine as in AN2824 (I2C master programming examples (DMA, interrupts, polling))
	if (length == 1) {
		return (SOOL_I2C_Receive1Byte(i2c_ptr->_setup.I2Cx, slave_address, buf_rx));
	} else if (length == 2) {
		return (SOOL_I2C_Receive2Bytes(i2c_ptr->_setup.I2Cx, slave_address, buf_rx));
	} else {
		return (SOOL_I2C_ReceiveMoreThan2Bytes(i2c_ptr->_setup.I2Cx, slave_address, buf_rx, length));
	}

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t SOOL_I2C_ReceiveMoreThan2Bytes(I2C_TypeDef* I2Cx, uint8_t SlaveAddress, uint8_t* pBuffer, uint32_t NumByteToRead) {

	uint32_t Timeout = 0xFFFF;
	uint8_t Address = 0x00;
	uint32_t temp = 0;

	// - - - - - - - - - - - - - - - - - - - - -

    /* Send START condition */
    I2Cx->CR1 |= I2C_CR1_START; // CR1_START_Set;
    /* Wait until SB flag is set: EV5 */
    while ((I2Cx->SR1&0x0001) != 0x0001) {
        if (Timeout-- == 0) {
            return (0);
        }
    }

    Timeout = 0xFFFF;
    /* Send slave address */
    /* Reset the address bit0 for write */
    SlaveAddress |= I2C_OAR1_ADD0; // OAR1_ADD0_Set;;
    Address = SlaveAddress;
    /* Send the slave address */
    I2Cx->DR = Address;
    /* Wait until ADDR is set: EV6 */
    while ((I2Cx->SR1&0x0002) != 0x0002)
    {
        if (Timeout-- == 0)
        	return (0);
    }
    /* Clear ADDR by reading SR2 status register */
    temp = I2Cx->SR2;
    /* While there is data to be read */
    while (NumByteToRead)
    {
        /* Receive bytes from first byte until byte N-3 */
        if (NumByteToRead != 3)
        {
            /* Poll on BTF to receive data because in polling mode we can not guarantee the
            EV7 software sequence is managed before the current byte transfer completes */
            while ((I2Cx->SR1 & 0x00004) != 0x000004);
            /* Read data */
            *pBuffer = I2Cx->DR;
            /* */
            pBuffer++;
            /* Decrement the read bytes counter */
            NumByteToRead--;
        }

        /* it remains to read three data: data N-2, data N-1, Data N */
        if (NumByteToRead == 3)
        {

            /* Wait until BTF is set: Data N-2 in DR and data N -1 in shift register */
            while ((I2Cx->SR1 & 0x00004) != 0x000004);
            /* Clear ACK */
            I2Cx->CR1 &= (uint16_t)(~(uint16_t)I2C_CR1_ACK); // CR1_ACK_Reset;

            /* Disable IRQs around data reading and STOP programming because of the
            limitation ? */
            __disable_irq();
            /* Read Data N-2 */
            *pBuffer = I2Cx->DR;
            /* Increment */
            pBuffer++;
            /* Program the STOP */
            I2Cx->CR1 |= I2C_CR1_STOP; // CR1_STOP_Set;
            /* Read DataN-1 */
            *pBuffer = I2Cx->DR;
            /* Re-enable IRQs */
            __enable_irq();
            /* Increment */
            pBuffer++;
            /* Wait until RXNE is set (DR contains the last data) */
            while ((I2Cx->SR1 & 0x00040) != 0x000040);
            /* Read DataN */
            *pBuffer = I2Cx->DR;
            /* Reset the number of bytes to be read by master */
            NumByteToRead = 0;

        }
    }
    /* Make sure that the STOP bit is cleared by Hardware before CR1 write access */
    while ((I2Cx->CR1&0x200) == 0x200);
    /* Enable Acknowledgement to be ready for another reception */
    I2Cx->CR1 |= I2C_CR1_ACK; // CR1_ACK_Set;

    return (1);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t SOOL_I2C_Receive2Bytes(I2C_TypeDef* I2Cx, uint8_t SlaveAddress, uint8_t* pBuffer) {

	uint32_t Timeout = 0xFFFF;
	uint8_t Address = 0x00;
	uint32_t temp = 0;

	// - - - - - - - - - - - - - - - - - - - - -

	/* Set POS bit */
	I2Cx->CR1 |= I2C_CR1_POS; // CR1_POS_Set;
	Timeout = 0xFFFF;
	/* Send START condition */
	I2Cx->CR1 |= I2C_CR1_START; // CR1_START_Set;
	/* Wait until SB flag is set: EV5 */
	while ((I2Cx->SR1&0x0001) != 0x0001)
	{
		if (Timeout-- == 0)
			return (0);
	}
	Timeout = 0xFFFF;
	/* Send slave address */
	/* Set the address bit0 for read */
	SlaveAddress |= I2C_OAR1_ADD0; // OAR1_ADD0_Set;
	Address = SlaveAddress;
	/* Send the slave address */
	I2Cx->DR = Address;
	/* Wait until ADDR is set: EV6 */
	while ((I2Cx->SR1&0x0002) != 0x0002)
	{
		if (Timeout-- == 0)
			return (0);
	}
	/* EV6_1: The acknowledge disable should be done just after EV6,
	that is after ADDR is cleared, so disable all active IRQs around ADDR clearing and
	ACK clearing */
	__disable_irq();
	/* Clear ADDR by reading SR2 register  */
	temp = I2Cx->SR2;
	/* Clear ACK */
	I2Cx->CR1 &= (uint16_t)(~(uint16_t)I2C_CR1_ACK); // CR1_ACK_Reset;
	/*Re-enable IRQs */
	__enable_irq();
	/* Wait until BTF is set */
	while ((I2Cx->SR1 & 0x00004) != 0x000004);
	/* Disable IRQs around STOP programming and data reading because of the limitation ?*/
	__disable_irq();
	/* Program the STOP */
	I2C_GenerateSTOP(I2Cx, ENABLE);
	/* Read first data */
	*pBuffer = I2Cx->DR;
	/* Re-enable IRQs */
	__enable_irq();
	/**/
	pBuffer++;
	/* Read second data */
	*pBuffer = I2Cx->DR;
	/* Make sure that the STOP bit is cleared by Hardware before CR1 write access */
	while ((I2Cx->CR1&0x200) == 0x200);
	/* Enable Acknowledgement to be ready for another reception */
	I2Cx->CR1  |= I2C_CR1_ACK; // CR1_ACK_Set;
	/* Clear POS bit */
	I2Cx->CR1  &= (uint16_t)(~(uint16_t)I2C_CR1_POS); // CR1_POS_Reset;

	return (1);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t SOOL_I2C_Receive1Byte(I2C_TypeDef* I2Cx, uint8_t SlaveAddress, uint8_t* pBuffer) {

	uint32_t Timeout = 0xFFFF;
	uint8_t Address = 0x00;
	uint32_t temp = 0;

	// - - - - - - - - - - - - - - - - - - - - -

	Timeout = 0xFFFF;
	/* Send START condition */
	I2Cx->CR1 |= I2C_CR1_START; // CR1_START_Set;
	/* Wait until SB flag is set: EV5  */
	while ((I2Cx->SR1&0x0001) != 0x0001)
	{
		if (Timeout-- == 0)
			return (0);
	}
	/* Send slave address */
	/* Reset the address bit0 for read */
	SlaveAddress |= I2C_OAR1_ADD0; // OAR1_ADD0_Set;
	Address = SlaveAddress;
	/* Send the slave address */
	I2Cx->DR = Address;
	/* Wait until ADDR is set: EV6_3, then program ACK = 0, clear ADDR
	and program the STOP just after ADDR is cleared. The EV6_3
	software sequence must complete before the current byte end of transfer.*/
	/* Wait until ADDR is set */
	Timeout = 0xFFFF;
	while ((I2Cx->SR1&0x0002) != 0x0002)
	{
		if (Timeout-- == 0)
			return (0);
	}
	/* Clear ACK bit */
	I2Cx->CR1 &= (uint16_t)(~(uint16_t)I2C_CR1_ACK); // CR1_ACK_Reset;
	/* Disable all active IRQs around ADDR clearing and STOP programming because the EV6_3
	software sequence must complete before the current byte end of transfer */
	__disable_irq();
	/* Clear ADDR flag */
	temp = I2Cx->SR2;
	/* Program the STOP */
	I2Cx->CR1 |= I2C_CR1_STOP; // CR1_STOP_Set;
	/* Re-enable IRQs */
	__enable_irq();
	/* Wait until a data is received in DR register (RXNE = 1) EV7 */
	while ((I2Cx->SR1 & 0x00040) != 0x000040);
	/* Read the data */
	*pBuffer = I2Cx->DR;
	/* Make sure that the STOP bit is cleared by Hardware before CR1 write access */
	while ((I2Cx->CR1&0x200) == 0x200);
	/* Enable Acknowledgement to be ready for another reception */
	I2Cx->CR1 |= I2C_CR1_ACK; // CR1_ACK_Set;

	return (1);

}
