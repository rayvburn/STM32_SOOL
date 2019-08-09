/*
 * MAX7219_7seg.c
 *
 *  Created on: 09.08.2019
 *      Author: user
 */

#include <sool/IC/MAX7219/MAX7219_7seg.h>
#include <stdlib.h> // itoa()

// Reference: https://github.com/lamik/MAX7219_digits_STM32_HAL/blob/master/Src/max7219_digits.c

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// NOTE: only SPI1 remap supported in STM32F103C8T6
// NOTE: this library requires a certain order of displays (digits) - first digit connected
// to the IC must be wired to Digit0 OUTPUT, the following one - to the Digit1 etc.
volatile SOOL_MAX7219 SOOL_IC_MAX7219_Initialize(SPI_TypeDef *SPIx, uint8_t do_remap,
		GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, uint8_t disp_num) {

	/* MAX7219 instance */
	volatile SOOL_MAX7219 max7219;

	/* Remap to change SPIx-related pins location */
	if ( do_remap ) {
		if ( SPIx == SPI1 ) {
			/* SCK: PB3, 	MISO: PB4,		MOSI: PB5 */
			AFIO->MAPR |= AFIO_MAPR_SPI1_REMAP;
		} else {
			// will throw some exception
			return (max7219);
		}
	}

	/* Buffer */
	// See Table 1 in (https://datasheets.maximintegrated.com/en/ds/MAX7219-MAX7221.pdf)
	// 16 bits - D15 ... D0
	SOOL_Vector_Uint16 buf = SOOL_Memory_Vector_Uint16_Init();
	for ( uint8_t i = 0; i < disp_num; i++ ) {
		// buffer structure is as follows:
		//  o each element of the vector is 16-bit number
		//  o 8 LSB (D7 - D0) store data to be send via SPI
		//  o 8 MSB (D15 - D8) store address of the register
		//  o (D15-D12) are `don't care` bits, setting them
		//	  do not have any effect
		buf.Add(&buf, 0);
	}

	/* Initialize SPI peripheral */
	volatile SOOL_SPI_DMA spi = SOOL_Periph_SPI_DMA_Init(SPIx, SPI_Direction_1Line_Tx,
														 SPI_DataSize_16b, SPI_CPOL_Low, SPI_CPHA_1Edge,
														 SPI_BaudRatePrescaler_64, SPI_FirstBit_MSB);

	/* Initialize SPI device class instance */
	SOOL_SPI_Device spi_device = spi.AddDevice(&spi, GPIOx, GPIO_Pin);

	/* Save `Setup` structure */
	max7219.base_spi = spi;
	max7219._setup.spi_device = spi_device;
//	max7219._setup.cascade_num = 0;

	/* Save `Buffer` structure */
	max7219._buf.tx = buf;
	max7219._buf.rx = buf;

	return (max7219);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

//uint8_t MAX7219_AddDeviceCascade(volatile SOOL_MAX7219 *max7219_ptr) {
//
////	uint8_t new_size = max7219_ptr->_setup.cascade_num + 1;
////
////	if ( SOOL_Memory_Vector_Uint16_Extend(max7219_ptr._buf.ptr, new_size) ) {
////
////		SOOL_Vector_Uint16 buf = SOOL_Memory_Vector_Uint16_Init();
////		*(max7219_ptr._buf.ptr + new_size) = buf;
////		max7219_ptr->_setup.cascade_num++;
////
////		return (1);
////
////	}
////
////	return (0);
//
//	max7219_ptr->_buf.tx.Add(&max7219_ptr->_buf.tx, 0);
//	max7219_ptr->_buf.rx.Add(&max7219_ptr->_buf.rx, 0);
//	return (1);
//
//}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// TODO: DISP_FROM must be smaller than DISP_TO, like DISP_FROM indicates the units whereas DISP_TO shows hundreds for example
// NOTE: DISP_TO inclusive!
uint8_t MAX7219_PrintSection(volatile SOOL_MAX7219 *max7219_ptr, uint8_t disp_to, uint8_t disp_from,
		int32_t value) {

	/* Allocate some memory */
	char *arr = malloc( (disp_from - disp_to) * sizeof(char));

	// check if allocation was successful
	if ( arr == NULL ) {
		return (0);
	}

	/* Convert value to a character array */
	itoa(value, arr, 10) ;

	/* Update buffer content */
	uint8_t disp_num = 0;
	for ( uint8_t i = disp_from; i <= disp_to; i++ ) {
		disp_num = (disp_from + i);
		MAX7219_SetDigit(max7219_ptr, disp_num, arr[i], disp_num == max7219_ptr->_setup.disp_dot);
	}

	/* Deallocate */
	free(arr);

	return (1);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

uint8_t MAX7219_SendData(volatile SOOL_MAX7219 *max7219_ptr) {

	// FIXME: cascade mode support
	uint8_t status = max7219_ptr->base_spi.SendReceive(&max7219_ptr->base_spi,
												   	   &max7219_ptr->_setup.spi_device,
													   (uint32_t)&max7219_ptr->_buf.rx._data[0],
													   (uint32_t)&max7219_ptr->_buf.tx._data[0], 1);
//													   max7219_ptr->_setup.cascade_num + 1); // FIXME ^
	return (status);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// Private, low-level function
// FIXME: Why use 2 x uint8_t when 1 x uint16_t possible?
uint8_t MAX7219_PrepareSendToDevice(volatile SOOL_MAX7219 *max7219_ptr, uint8_t dev_num, uint8_t reg,
		uint8_t data) {

//	SOOL_Vector_Uint16* dev_buf_ptr = max7219_ptr->_buf.ptr + dev_num;
//
//	// convert register address and data into 16 bits serial format which MAX7219 is able to read
//	uint16_t value = 0;
//	value |= data; // lower byte
//	uint16_t temp = reg << 8;
//	value |= temp; // higher byte
//
//	dev_buf_ptr->Set(dev_buf_ptr, (uint16_t)0, value);

	// convert register address and data into 16 bits serial format which MAX7219 is able to read
	uint16_t value = 0x00;
	value |= (uint16_t)data; // lower byte
	value |= (reg << 8); 	 // higher byte
	max7219_ptr->_buf.tx.Set(&max7219_ptr->_buf.tx, dev_num, value);

	return (0);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

uint8_t MAX7219_SetDigit(volatile SOOL_MAX7219 *max7219_ptr, uint8_t digit, uint8_t value, uint8_t dot) {

	/* Convert value from human-readable to MAX7219-readable data */

	/*
#define MAX7219_SYMBOL_0	0x7E
#define MAX7219_SYMBOL_1	0x30
#define MAX7219_SYMBOL_2	0x6D
#define MAX7219_SYMBOL_3	0x79
#define MAX7219_SYMBOL_4	0x33
#define MAX7219_SYMBOL_5	0x5B
#define MAX7219_SYMBOL_6	0x5F
#define MAX7219_SYMBOL_7	0x70
#define MAX7219_SYMBOL_8	0x7F
#define MAX7219_SYMBOL_9	0x7B
#define MAX7219_SYMBOL_LINE	0xA0
	 */

//	max7219_ptr->_buf.tx.Set(&max7219_ptr->_buf.tx, digit, value)
	return (1);
}
