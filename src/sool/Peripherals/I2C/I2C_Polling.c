/*
 * I2C_Polling.c
 *
 *  Created on: 03.12.2019
 *      Author: user
 */

#include <sool/Peripherals/I2C/I2C_Polling.h>

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t SOOL_I2C_Transmit(SOOL_I2C_Polling *i2c_ptr, uint8_t slave_address, uint32_t length, uint8_t* buf_tx);
static uint8_t SOOL_I2C_Receive(SOOL_I2C_Polling *i2c_ptr, uint8_t slave_address, uint32_t length, uint8_t* buf_rx);

// helpers
static uint8_t SOOL_I2C_Receive1Byte(I2C_TypeDef* I2Cx, uint8_t SlaveAddress, uint8_t* pBuffer);
static uint8_t SOOL_I2C_Receive2Bytes(I2C_TypeDef* I2Cx, uint8_t SlaveAddress, uint8_t* pBuffer);
static uint8_t SOOL_I2C_ReceiveMoreThan2Bytes(I2C_TypeDef* I2Cx, uint8_t SlaveAddress, uint8_t* pBuffer, uint32_t NumByteToRead);

typedef enum {
	I2C_POLLING_ERROR = 256u
};

/* I2C START mask */
#define CR1_START_Set           ((uint16_t)0x0100)
#define CR1_START_Reset         ((uint16_t)0xFEFF)

#define CR1_POS_Set           ((uint16_t)0x0800)
#define CR1_POS_Reset         ((uint16_t)0xF7FF)

/* I2C ADD0 mask */
#define OAR1_ADD0_Set           ((uint16_t)0x0001)
#define OAR1_ADD0_Reset         ((uint16_t)0xFFFE)

/* I2C ACK mask */
#define CR1_ACK_Set             ((uint16_t)0x0400)
#define CR1_ACK_Reset           ((uint16_t)0xFBFF)

/* I2C STOP mask */
#define CR1_STOP_Set            ((uint16_t)0x0200)
#define CR1_STOP_Reset          ((uint16_t)0xFDFF)

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

SOOL_I2C_Polling SOOL_Periph_I2C_Polling_Init(	I2C_TypeDef*	I2Cx,
		uint16_t 		I2C_Ack,
		uint16_t		I2C_AcknowledgedAddress,
		uint32_t 		I2C_ClockSpeed /* <= 400 kHz */,
		uint16_t 		I2C_OwnAddress1)
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


static uint8_t SOOL_I2C_Transmit(SOOL_I2C_Polling *i2c_ptr, uint8_t slave_address, uint32_t length, uint8_t* buf_tx) {

	I2C_GenerateSTART(i2c_ptr->_setup.I2Cx, ENABLE);
	while (!I2C_GetFlagStatus(i2c_ptr->_setup.I2Cx, I2C_FLAG_SB));							// wait for EV5

	I2C_Send7bitAddress(i2c_ptr->_setup.I2Cx, slave_address, I2C_Direction_Transmitter);
	while (!I2C_GetFlagStatus(i2c_ptr->_setup.I2Cx, I2C_FLAG_ADDR));						// wait for EV6
	uint16_t sr1 = i2c_ptr->_setup.I2Cx->SR1;
	uint16_t sr2 = i2c_ptr->_setup.I2Cx->SR2;
	// ADDR=1, cleared by reading SR1 register followed by reading SR2


	// DMA should take care of this part
	while (!I2C_GetFlagStatus(i2c_ptr->_setup.I2Cx, I2C_FLAG_TXE));							// wait for EV8(_1)
	I2C_SendData(i2c_ptr->_setup.I2Cx, 0x01);
	while (!I2C_GetFlagStatus(i2c_ptr->_setup.I2Cx, I2C_FLAG_TXE));							// wait for EV8
	I2C_SendData(i2c_ptr->_setup.I2Cx, 0x04);
	while (!(I2C_GetFlagStatus(i2c_ptr->_setup.I2Cx, I2C_FLAG_TXE) &&
			 I2C_GetFlagStatus(i2c_ptr->_setup.I2Cx, I2C_FLAG_BTF)));						// wait for EV8_2
	I2C_GenerateSTOP(i2c_ptr->_setup.I2Cx, ENABLE);

	return (1);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t SOOL_I2C_Receive(SOOL_I2C_Polling *i2c_ptr, uint8_t slave_address, uint32_t length, uint8_t* buf_rx) {

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

    /* Send START condition */
    I2Cx->CR1 |= CR1_START_Set;
    /* Wait until SB flag is set: EV5 */
    while ((I2Cx->SR1&0x0001) != 0x0001) {
        if (Timeout-- == 0) {
            return (I2C_POLLING_ERROR);
        }
    }

    Timeout = 0xFFFF;
    /* Send slave address */
    /* Reset the address bit0 for write */
    SlaveAddress |= OAR1_ADD0_Set;;
    Address = SlaveAddress;
    /* Send the slave address */
    I2Cx->DR = Address;
    /* Wait until ADDR is set: EV6 */
    while ((I2Cx->SR1&0x0002) != 0x0002)
    {
        if (Timeout-- == 0)
        	return (I2C_POLLING_ERROR);
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
            I2Cx->CR1 &= CR1_ACK_Reset;

            /* Disable IRQs around data reading and STOP programming because of the
            limitation ? */
            __disable_irq();
            /* Read Data N-2 */
            *pBuffer = I2Cx->DR;
            /* Increment */
            pBuffer++;
            /* Program the STOP */
            I2Cx->CR1 |= CR1_STOP_Set;
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
    I2Cx->CR1 |= CR1_ACK_Set;

    return (1);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t SOOL_I2C_Receive2Bytes(I2C_TypeDef* I2Cx, uint8_t SlaveAddress, uint8_t* pBuffer) {

	uint32_t Timeout = 0xFFFF;
	uint8_t Address = 0x00;
	uint32_t temp = 0;

	/* Set POS bit */
	I2Cx->CR1 |= CR1_POS_Set;
	Timeout = 0xFFFF;
	/* Send START condition */
	I2Cx->CR1 |= CR1_START_Set;
	/* Wait until SB flag is set: EV5 */
	while ((I2Cx->SR1&0x0001) != 0x0001)
	{
		if (Timeout-- == 0)
			return (I2C_POLLING_ERROR);
	}
	Timeout = 0xFFFF;
	/* Send slave address */
	/* Set the address bit0 for read */
	SlaveAddress |= OAR1_ADD0_Set;
	Address = SlaveAddress;
	/* Send the slave address */
	I2Cx->DR = Address;
	/* Wait until ADDR is set: EV6 */
	while ((I2Cx->SR1&0x0002) != 0x0002)
	{
		if (Timeout-- == 0)
			return (I2C_POLLING_ERROR);
	}
	/* EV6_1: The acknowledge disable should be done just after EV6,
	that is after ADDR is cleared, so disable all active IRQs around ADDR clearing and
	ACK clearing */
	__disable_irq();
	/* Clear ADDR by reading SR2 register  */
	temp = I2Cx->SR2;
	/* Clear ACK */
	I2Cx->CR1 &= CR1_ACK_Reset;
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
	I2Cx->CR1  |= CR1_ACK_Set;
	/* Clear POS bit */
	I2Cx->CR1  &= CR1_POS_Reset;

	return (1);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t SOOL_I2C_Receive1Byte(I2C_TypeDef* I2Cx, uint8_t SlaveAddress, uint8_t* pBuffer) {

	uint32_t Timeout = 0xFFFF;
	uint8_t Address = 0x00;
	uint32_t temp = 0;

	Timeout = 0xFFFF;
	/* Send START condition */
	I2Cx->CR1 |= CR1_START_Set;
	/* Wait until SB flag is set: EV5  */
	while ((I2Cx->SR1&0x0001) != 0x0001)
	{
		if (Timeout-- == 0)
			return (I2C_POLLING_ERROR);
	}
	/* Send slave address */
	/* Reset the address bit0 for read */
	SlaveAddress |= OAR1_ADD0_Set;
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
			return (I2C_POLLING_ERROR);
	}
	/* Clear ACK bit */
	I2Cx->CR1 &= CR1_ACK_Reset;
	/* Disable all active IRQs around ADDR clearing and STOP programming because the EV6_3
	software sequence must complete before the current byte end of transfer */
	__disable_irq();
	/* Clear ADDR flag */
	temp = I2Cx->SR2;
	/* Program the STOP */
	I2Cx->CR1 |= CR1_STOP_Set;
	/* Re-enable IRQs */
	__enable_irq();
	/* Wait until a data is received in DR register (RXNE = 1) EV7 */
	while ((I2Cx->SR1 & 0x00040) != 0x000040);
	/* Read the data */
	*pBuffer = I2Cx->DR;
	/* Make sure that the STOP bit is cleared by Hardware before CR1 write access */
	while ((I2Cx->CR1&0x200) == 0x200);
	/* Enable Acknowledgement to be ready for another reception */
	I2Cx->CR1 |= CR1_ACK_Set;

	return (1);

}
