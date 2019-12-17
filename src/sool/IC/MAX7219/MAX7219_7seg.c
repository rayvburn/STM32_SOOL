/*
 * MAX7219_7seg.c
 *
 *  Created on: 09.08.2019
 *      Author: user
 */

#include <sool/IC/MAX7219/MAX7219_7seg.h>
#include <stdlib.h> // itoa()
#include <sool/Maths/PowInt.h>
#include <sool/Common/Delay.h>

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// public methods
static uint8_t MAX7219_AddDotDisplay(volatile SOOL_MAX7219 *max7219_ptr, uint8_t dot_disp_num);
static void    MAX7219_ShowDots(volatile SOOL_MAX7219 *max7219_ptr, uint8_t state);
static uint8_t MAX7219_Shutdown(volatile SOOL_MAX7219 *max7219_ptr, uint8_t shutdown);
static uint8_t MAX7219_Print(volatile SOOL_MAX7219 *max7219_ptr, int32_t value);
static uint8_t MAX7219_PrintString(volatile SOOL_MAX7219 *max7219_ptr, char* str, uint8_t to_free);
static uint8_t MAX7219_PrintSection(volatile SOOL_MAX7219 *max7219_ptr, uint8_t disp_from, uint8_t disp_to, int32_t value);
static uint8_t MAX7219_PrintSectionString(volatile SOOL_MAX7219 *max7219_ptr, uint8_t disp_from, uint8_t disp_to, char* string, uint8_t to_free);
static uint8_t MAX7219_ReceptionCompleteIrqHandler(volatile SOOL_MAX7219 *max7219_ptr);

// private class functions
static uint8_t MAX7219_PrintSectionStringFull(volatile SOOL_MAX7219 *max7219_ptr, uint8_t disp_from, uint8_t disp_to, char* str, uint8_t extra_zero_pos, uint8_t to_free);
//static uint8_t MAX7219_ShutdownFull(volatile SOOL_MAX7219 *max7219_ptr, uint8_t shutdown, uint8_t send);
static uint8_t MAX7219_SetDigit(volatile SOOL_MAX7219 *max7219_ptr, uint8_t digit, char value, uint8_t dot);
static uint8_t MAX7219_SendData(volatile SOOL_MAX7219 *max7219_ptr);

// helper functions
static uint8_t MAX7219_FindDotPosition(volatile SOOL_MAX7219 *max7219_ptr, uint8_t disp_from, uint8_t disp_to);
static uint8_t MAX7219_ExtendBuffers(volatile SOOL_MAX7219 *max7219_ptr, uint16_t tx_value);
static void    MAX7219_EraseBuffers(volatile SOOL_MAX7219 *max7219_ptr);
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

	/* Initialize SPI peripheral */
	// NOTE: SPI_Direction must be set to allow 2 line operation. This provides possibility
	// to RX's ISR be called and transfer will be marked as completed.
	// Initially SPI_Direction was set to SPI_Direction_1Line_Tx but CS remained HIGH
	// after successful transfer.
	volatile SOOL_SPI_DMA spi = SOOL_Periph_SPI_DMA_Init(SPIx, SPI_Direction_2Lines_FullDuplex,
														 SPI_DataSize_16b, SPI_CPOL_Low, SPI_CPHA_1Edge,
														 SPI_BaudRatePrescaler_64, SPI_FirstBit_MSB);

	/* Initialize SPI device class instance */
	SOOL_SPI_Device spi_device = spi.AddDevice(&spi, GPIOx, GPIO_Pin);

	/* Save base `classes` */
	max7219.base_spi = spi;
	max7219.base_device = spi_device;

	/* Save `Setup` structure */
	max7219._setup.disp_num = disp_num;
	max7219._setup.dots = SOOL_Memory_Vector_Uint16_Init();
	max7219._setup.bcd_decode = 0;

	/* Save `Buffer` structure */
	max7219._buf.tx = SOOL_Memory_Vector_Uint16_Init(); // buf;
	max7219._buf.rx = SOOL_Memory_Vector_Uint16_Init(); // buf;

	/* Save methods pointers */
	max7219.AddDotDisplay = MAX7219_AddDotDisplay;
	max7219.Shutdown = MAX7219_Shutdown;
	max7219.Print = MAX7219_Print;
	max7219.PrintSection = MAX7219_PrintSection;
	max7219.PrintString = MAX7219_PrintString;
	max7219.PrintSectionString = MAX7219_PrintSectionString;
	max7219.ShowDots = MAX7219_ShowDots;
	max7219.Send = MAX7219_SendData;
	max7219._ReceptionCompleteIrqHandler = MAX7219_ReceptionCompleteIrqHandler;

	return (max7219);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

uint8_t SOOL_IC_MAX7219_Configure(volatile SOOL_MAX7219 *max7219_ptr, uint8_t bcd_decode, uint8_t test_mode) {

	/* Check parameter correctness */
	if (max7219_ptr->_setup.disp_num <= 0) {
		return (0);
	}

	/* Approximately 15 milliseconds are needed for stable operation of MAX7219 */
	SOOL_Common_Delay(20, SystemCoreClock);
	// This ^ may be confusing for the compiler which may interpret library's
	// default SystemCoreClock as firm whereas MCU this class is compiled
	// for may have a different value. This is not crucial though.

	/* Helper variable */
	uint16_t reg = 0;

	/* Decode mode configuration */
	reg  = (uint16_t)(0x09 << 8);			// Decode Mode register address
	// 0x00: no decode mode  (Table 4. Decode-Mode Register Examples (Address (Hex) = 0xX9))
	// 0xFF: BCD decode mode (Table 4. Decode-Mode Register Examples (Address (Hex) = 0xX9))
	if (bcd_decode) {
		max7219_ptr->_setup.bcd_decode = 1;
		reg |= (uint16_t)0xFF;
	} else {
		max7219_ptr->_setup.bcd_decode = 0;
		reg |= (uint16_t)0x00;
	}
//	max7219_ptr->_buf.tx.Add(&max7219_ptr->_buf.tx, reg);
//	max7219_ptr->_buf.rx.Add(&max7219_ptr->_buf.rx, 0);
	if ( !MAX7219_ExtendBuffers(max7219_ptr, reg) ) {
		return (0);
	}

	/* Intensity register configuration */
	reg  = (uint16_t)(0x0A << 8);			// Intensity register address
	reg |= (uint16_t)0x0F;					// max intensity (Table 7. Intensity Register Format (Address (Hex) = 0xXA))
//	max7219_ptr->_buf.tx.Add(&max7219_ptr->_buf.tx, reg);
//	max7219_ptr->_buf.rx.Add(&max7219_ptr->_buf.rx, 0);
	if ( !MAX7219_ExtendBuffers(max7219_ptr, reg) ) {
		return (0);
	}

	/* Scan limit configuration */
	reg  = (uint16_t)(0x0B << 8);			// Scan-Limit register address
	reg |= (uint16_t)(max7219_ptr->_setup.disp_num-1);	// set according to instance initializer (`constructor`) (Table 8. Scan-Limit Register Format (Address (Hex) = 0xXB))
//	max7219_ptr->_buf.tx.Add(&max7219_ptr->_buf.tx, reg);
//	max7219_ptr->_buf.rx.Add(&max7219_ptr->_buf.rx, 0);
	if ( !MAX7219_ExtendBuffers(max7219_ptr, reg) ) {
		return (0);
	}

	/* Display test mode configuration */
	// In  display-test mode, 8 digits are scanned.
	reg  = (uint16_t)(0x0F << 8);			// Display-Test register address
	// 0: normal operation  (Table 10. Display-Test Register Format(Address (Hex) = 0xXF))
	// 1: test mode 		(Table 10. Display-Test Register Format(Address (Hex) = 0xXF))
	(test_mode) ? (reg |= (uint16_t)1) : (reg |= (uint16_t)0);
//	max7219_ptr->_buf.tx.Add(&max7219_ptr->_buf.tx, reg);
//	max7219_ptr->_buf.rx.Add(&max7219_ptr->_buf.rx, 0);
	if ( !MAX7219_ExtendBuffers(max7219_ptr, reg) ) {
		return (0);
	}

	/* Shutdown mode configuration */
	// The display-test register operates in two modes: normal
	// and  display  test.
	// Display-test  mode  turns  all  LEDs  on by overriding, but not altering,
	// all controls and digit registers  (including  the  shutdown  register).
	if ( !test_mode ) {
//		if ( !MAX7219_ShutdownFull(max7219_ptr, DISABLE, DISABLE) ) {
		if ( !MAX7219_Shutdown(max7219_ptr, DISABLE) ) {
			return (0);
		}
	}

	/* Clear all digits */
	for ( uint8_t i = 0; i < max7219_ptr->_setup.disp_num; i++ ) {
		MAX7219_SetDigit(max7219_ptr, i, ' ', DISABLE); // hard-coded symbol - it's error-proof
	}

	/* Send prepared data to the MAX7219 */
	if ( !MAX7219_SendData(max7219_ptr) ) {
		return (0);
	}

	return (1);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void SOOL_IC_MAX7219_Startup(volatile SOOL_MAX7219* max7219_ptr) {
	max7219_ptr->base_spi.base_dma_tx.EnableNVIC(&max7219_ptr->base_spi.base_dma_tx);
	max7219_ptr->base_spi.base_dma_rx.EnableNVIC(&max7219_ptr->base_spi.base_dma_rx);
	max7219_ptr->base_spi.EnableNVIC(&max7219_ptr->base_spi);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t MAX7219_AddDotDisplay(volatile SOOL_MAX7219 *max7219_ptr, uint8_t dot_disp_num) {

	if ( dot_disp_num <= max7219_ptr->_setup.disp_num ) {
		max7219_ptr->_setup.dots.Add(&max7219_ptr->_setup.dots, dot_disp_num);
		return (1);
	}
	return (0);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void MAX7219_ShowDots(volatile SOOL_MAX7219 *max7219_ptr, uint8_t state) {
	max7219_ptr->_setup.show_dots = (uint8_t)state;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t MAX7219_Shutdown(volatile SOOL_MAX7219 *max7219_ptr, uint8_t shutdown) {

	uint16_t reg = 0;
	reg  = (uint16_t)(0x0C << 8);			// Shutdown mode register address
	// 0: shutdown mode 	(Table 3. Shutdown Register Format (Address (Hex) = 0xXC))
	// 1: normal operation 	(Table 3. Shutdown Register Format (Address (Hex) = 0xXC))
	(shutdown) ? (reg |= (uint16_t)1) : (reg |= (uint16_t)0);
//	max7219_ptr->_buf.tx.Add(&max7219_ptr->_buf.tx, reg);
//	max7219_ptr->_buf.rx.Add(&max7219_ptr->_buf.rx, 0);
	if ( !MAX7219_ExtendBuffers(max7219_ptr, reg) ) {
		return (0);
	}

//	/* Send prepared data to the MAX7219 if needed */
//	if ( send ) {
//		if ( !MAX7219_SendData(max7219_ptr) ) {
//			return (0);
//		}
//	}
	return (1);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

//static uint8_t MAX7219_ShutdownFull(volatile SOOL_MAX7219 *max7219_ptr, uint8_t shutdown, uint8_t send) {
//}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t MAX7219_Print(volatile SOOL_MAX7219 *max7219_ptr, int32_t value) {
	return (MAX7219_PrintSection(max7219_ptr, 0, (max7219_ptr->_setup.disp_num - 1), value));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t MAX7219_PrintString(volatile SOOL_MAX7219 *max7219_ptr, char* str, uint8_t to_free) {
	return (MAX7219_PrintSectionString(max7219_ptr, 0, (max7219_ptr->_setup.disp_num - 1), str, to_free));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// NOTE: DISP_FROM must be smaller than DISP_TO, like DISP_FROM indicates the units whereas DISP_TO shows hundreds for example
// NOTE: DISP_FROM is the index of the first digit the `value` will be printed on
// NOTE: DISP_TO inclusive!
// NOTE: `disp_from` and `disp_to` are digit indexes
static uint8_t MAX7219_PrintSection(volatile SOOL_MAX7219 *max7219_ptr, uint8_t disp_from, uint8_t disp_to,
		int32_t value) {

	/* Allocate some memory */
	char *str = (char*)(malloc( (disp_to - disp_from + 1) * sizeof(char)));

	// check if allocation was successful
	if ( str == NULL ) {
		return (0);
	}

	/* Convert value to a character array */
	itoa(value, str, 10);

	/* Check whether addition of 0 in front of the DOT is necessary */
	// check dot position within the given section
	uint8_t section_dot_pos = MAX7219_FindDotPosition(max7219_ptr, disp_from, disp_to);
	// TODO
//	section_dot_pos -= disp_from;

	// add extra zero if only floating point part defined
	uint8_t extra_zero_pos = SOOL_MAX7219_DISABLE_DP;
	// abandon operation when dots should not be shown
	if ( max7219_ptr->_setup.show_dots ) {

		if ( disp_from == 3 ) {
			int aaaa = 0;
			aaaa++;
		}

		// correct dot position according to the section's beginning
		if ( SOOL_Maths_PowInt(10, section_dot_pos - disp_from) > value ) {
			// add 0 in front
			extra_zero_pos = section_dot_pos;
		}
	}

	return (MAX7219_PrintSectionStringFull(max7219_ptr, disp_from, disp_to, str, extra_zero_pos, ENABLE));

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// `to_free` must be true (1/ENABLE) if the string was created via malloc() etc.
static uint8_t MAX7219_PrintSectionString(volatile SOOL_MAX7219 *max7219_ptr, uint8_t disp_from, uint8_t disp_to,
		char* string, uint8_t to_free) {

	return (MAX7219_PrintSectionStringFull(max7219_ptr, disp_from, disp_to, string, SOOL_MAX7219_DISABLE_DP, to_free));

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t MAX7219_ReceptionCompleteIrqHandler(volatile SOOL_MAX7219 *max7219_ptr) {

	/* Process interrupt in terms of SPI peripheral (called on TransferComplete event) */
	if ( !max7219_ptr->base_spi._DmaRxIrqHandler(&max7219_ptr->base_spi) ) {
		return (0);
	}

	/* It is assumed that only single dataframe (16-bits) was sent
	 * so clear the buffers' first elements */
	max7219_ptr->_buf.tx.Remove(&max7219_ptr->_buf.tx, 0);
	max7219_ptr->_buf.rx.Remove(&max7219_ptr->_buf.rx, 0);

	/* Start next transfer if buffer is not empty */
	if ( max7219_ptr->_buf.tx._info.size > 0 ) {
		MAX7219_SendData(max7219_ptr);
	} else if ( max7219_ptr->_buf.tx._info.size == 0 ) {
		// seems that RX buffer is not erased completely;
		// when TX buffer is empty, the RX's one still has
		// one element;
		// this is just a workaround, where is the problem?
		while (max7219_ptr->_buf.rx._info.size != 0) {
			max7219_ptr->_buf.rx.Remove(&max7219_ptr->_buf.rx, 0);
		}
	}

	return (1);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t MAX7219_SetDigit(volatile SOOL_MAX7219 *max7219_ptr, uint8_t digit, char value, uint8_t dot) {

	/* Convert value from human-readable to MAX7219-readable data */
	// prepare register address value (see Table 2. Register Address Map)
	uint8_t address = (digit + 1);

	// prepare MAX7219-readable data (half word, see `Table 5. Code B Font` for BCD decode mode
	// or `Table 6. No-Decode Mode Data Bits andCorresponding Segment Lines` for no decode mode
	uint8_t data = 0;

	// BCD decode mode enabled
	if ( max7219_ptr->_setup.bcd_decode ) {

		switch ( value ) {
			case('0'):
			case('1'):
			case('2'):
			case('3'):
			case('4'):
			case('5'):
			case('6'):
			case('7'):
			case('8'):
			case('9'):
					data = (uint8_t)(value - '0');
					break;
			case('-'):
					data = MAX7219_SYMBOL_LINE;
					break;
			case(' '):
					data = MAX7219_SYMBOL_BLANK;
					break;
			default:
					return (0); // character not supported
		}

	} else {

		// TODO:
		// Create MAX7219 SYMBOLS

	}

	/* Low level, SPI transfer related section */
	// set D7 according to `dot` variable which is 1 if DP should be enabled
	data |= (dot << 7);

	// prepare half-word variable
	uint16_t reg = 0;
	reg  = (uint16_t)(address << 8);
	reg |= (uint16_t)data;

//	// add value to the TX buffer
//	max7219_ptr->_buf.tx.Add(&max7219_ptr->_buf.tx, reg);
//
//	// add any value to the RX buffer so it has the same size as TX buffer
//	max7219_ptr->_buf.rx.Add(&max7219_ptr->_buf.rx, 0);

	if ( !MAX7219_ExtendBuffers(max7219_ptr, reg) ) {
		return (0);
	}

	return (1);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t MAX7219_SendData(volatile SOOL_MAX7219 *max7219_ptr) {

	// debugging
	if ( max7219_ptr->_buf.tx._info.size != max7219_ptr->_buf.rx._info.size ) {
		size_t tx_size = max7219_ptr->_buf.tx._info.size;
		size_t rx_size = max7219_ptr->_buf.rx._info.size;
		int err = 0;
		err++;
	}

	// TODO: cascade mode support?
	uint8_t status = max7219_ptr->base_spi.SendReceive(&max7219_ptr->base_spi,
												   	   &max7219_ptr->base_device,
													   (uint32_t)&max7219_ptr->_buf.rx._data[0],
													   (uint32_t)&max7219_ptr->_buf.tx._data[0],
													   1);
	return (status);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t MAX7219_PrintSectionStringFull(volatile SOOL_MAX7219 *max7219_ptr, uint8_t disp_from,
		uint8_t disp_to, char* str, uint8_t extra_zero_pos, uint8_t to_free) {

	/* Unsupported character flag */
	uint8_t fail_flag = 0;

	/* Helper variable to start from the string's end */
	int8_t str_idx = strlen(str);

	/* Delete all buffers' elements */
//	MAX7219_EraseBuffers(max7219_ptr);

	// iterate over a given display section
	for ( uint8_t i = disp_from; i <= disp_to; i++ ) {

		// add 0 in front of the `dot` because only floating part is given
		if ( extra_zero_pos == i ) {
			MAX7219_SetDigit(max7219_ptr, i, '0', 1); // this character is always valid
			continue;
		}

		// decrement index of the character to-be-send
		if ( --str_idx < 0 ) {
			// skip the `for` loop when it is known in advance that
			// there are some excessive digits that need to be hidden;
			// `space` character turns off a digit
			MAX7219_SetDigit(max7219_ptr, i, ' ', 0);
			continue;
		}

		// check whether DP segment should be enabled
		uint8_t show_dot = 0; // reset to 0 after each iteration
		// if dots are desired to be shown, execute the `for` loop
		if ( max7219_ptr->_setup.show_dots ) {
			for ( uint8_t j = 0; j < max7219_ptr->_setup.dots._info.size; j++ ) {

				uint8_t dot_disp = max7219_ptr->_setup.dots.Get(&max7219_ptr->_setup.dots, j);
				if ( i == dot_disp ) {
					show_dot = 1;
					break; // stop the inner `for` loop
				}

			}
		}

		// update buffer content
		if ( !MAX7219_SetDigit(max7219_ptr, i, str[str_idx], show_dot) ) {
			fail_flag = 1;
			break;
		}

	}

	/* Deallocate */
	if ( to_free ) {
		free(str);
	}

	if ( !fail_flag ) {
//		return (MAX7219_SendData(max7219_ptr));
		return (1);
	}
	return (0);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - private functions - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t MAX7219_FindDotPosition(volatile SOOL_MAX7219 *max7219_ptr, uint8_t disp_from, uint8_t disp_to) {

	// iterate over a given display section
	for ( uint8_t i = disp_from; i <= disp_to; i++ ) {

		// check whether DP segment is within section's range
		for ( uint8_t j = 0; j < max7219_ptr->_setup.dots._info.size; j++ ) {

			uint8_t dot_disp = max7219_ptr->_setup.dots.Get(&max7219_ptr->_setup.dots, j);
			if ( i == dot_disp ) {
				return (i);
				break;
			}

		}

	}
	return (SOOL_MAX7219_DISABLE_DP);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t MAX7219_ExtendBuffers(volatile SOOL_MAX7219 *max7219_ptr, uint16_t tx_value) {

	// FIXME: wait until DMA transfer finishes, modification of buffers during
	// DMA operation will produce memory-related errors later on
	while (max7219_ptr->base_spi.IsBusy(&max7219_ptr->base_spi));

	// add value to the TX buffer
	if ( !max7219_ptr->_buf.tx.Add(&max7219_ptr->_buf.tx, tx_value) ) {
		return (0);
	}

	// add any value to the RX buffer so it has the same size as TX buffer
	if ( !max7219_ptr->_buf.rx.Add(&max7219_ptr->_buf.rx, 0) ) {
		// remove the just added TX value
		max7219_ptr->_buf.tx.Remove(&max7219_ptr->_buf.tx, max7219_ptr->_buf.tx._info.size - 1);
		return (0);
	}
	return (1);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void MAX7219_EraseBuffers(volatile SOOL_MAX7219 *max7219_ptr) {
	/* Prepare buffers */
	uint16_t buf_init_size = max7219_ptr->_buf.tx._info.size;
	for ( uint16_t i = 0; i < buf_init_size; i++ ) {
		max7219_ptr->_buf.tx.Remove(&max7219_ptr->_buf.tx, 0);
		max7219_ptr->_buf.rx.Remove(&max7219_ptr->_buf.rx, 0);
	}
}
