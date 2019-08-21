/*
 * MAX7219_7seg.c
 *
 *  Created on: 09.08.2019
 *      Author: user
 */

#include <sool/IC/MAX7219/MAX7219_7seg.h>
#include <stdlib.h> // itoa()
#include <sool/Maths/PowInt.h>

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t MAX7219_AddDotDisplay(volatile SOOL_MAX7219 *max7219_ptr, uint8_t dot_disp_num);
static uint8_t MAX7219_Print(volatile SOOL_MAX7219 *max7219_ptr, int32_t value);
static uint8_t MAX7219_PrintSection(volatile SOOL_MAX7219 *max7219_ptr, uint8_t disp_from, uint8_t disp_to, int32_t value);
static uint8_t MAX7219_PrintDots(volatile SOOL_MAX7219 *max7219_ptr, uint8_t disp_from, uint8_t disp_to, uint8_t dots_num);

static uint8_t MAX7219_SetDigit(volatile SOOL_MAX7219 *max7219_ptr, uint8_t digit, char value, uint8_t dot);
static uint8_t MAX7219_SendData(volatile SOOL_MAX7219 *max7219_ptr);

//static uint8_t MAX7219_InitializeOperation(volatile SOOL_MAX7219 *max_7219_ptr, uint8_t digits_to_scan);
static uint8_t MAX7219_FindDotPosition(volatile SOOL_MAX7219 *max7219_ptr, uint8_t disp_from, uint8_t disp_to);
static uint8_t MAX7219_TurnOffExcessiveDigits(volatile SOOL_MAX7219 *max7219_ptr, uint8_t disp_from, uint8_t disp_to, uint8_t length);

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

	/* Save `Buffer` structure */
	max7219._buf.tx = SOOL_Memory_Vector_Uint16_Init(); // buf;
	max7219._buf.rx = SOOL_Memory_Vector_Uint16_Init(); // buf;

	/* Save methods pointers */
	max7219.AddDotDisplay = MAX7219_AddDotDisplay;
	max7219.Print = MAX7219_Print;
	max7219.PrintDots = MAX7219_PrintDots;
	max7219.PrintSection = MAX7219_PrintSection;

//	/* Initialize MAX7219 settings to default */
//	if ( !MAX7219_InitializeOperation(&max7219, disp_num) ) {
//		// TODO: throw some exception
//	}

	return (max7219);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

extern uint8_t SOOL_IC_MAX7219_ConfigureDefault(volatile SOOL_MAX7219 *max7219_ptr) {

	/* Check parameter correctness */
	if (max7219_ptr->_setup.disp_num <= 0) {
		return (0);
	}

//	/* Helper variable */
//	uint16_t reg = 0;
//
//	/* Decode mode configuration */
//	reg  = (uint16_t)(0x09 << 8);			// Decode Mode register address
//	reg |= (uint16_t)0;						// no decode mode (Table 4. Decode-Mode Register Examples (Address (Hex) = 0xX9))
//	max7219_ptr->_buf.tx.Add(&max7219_ptr->_buf.tx, reg);
//	max7219_ptr->_buf.rx.Add(&max7219_ptr->_buf.rx, 0);
//
//	/* Intensity register configuration */
//	reg  = (uint16_t)(0x0A << 8);			// Intensity register address
//	reg |= (uint16_t)0x0F;					// max intensity (Table 7. Intensity Register Format (Address (Hex) = 0xXA))
//	max7219_ptr->_buf.tx.Add(&max7219_ptr->_buf.tx, reg);
//	max7219_ptr->_buf.rx.Add(&max7219_ptr->_buf.rx, 0);
//
//	/* Scan limit configuration */
//	reg  = (uint16_t)(0x0B << 8);			// Scan-Limit register address
//	reg |= (uint16_t)(max7219_ptr->_setup.disp_num-1);	// set according to instance initializer (`constructor`) (Table 8. Scan-Limit Register Format (Address (Hex) = 0xXB))
//	max7219_ptr->_buf.tx.Add(&max7219_ptr->_buf.tx, reg);
//	max7219_ptr->_buf.rx.Add(&max7219_ptr->_buf.rx, 0);
//
//	/* Display test mode configuration */
//	reg  = (uint16_t)(0x0F << 8);			// Display-Test register address
//	reg |= (uint16_t)0;						// normal operation (Table 10. Display-Test Register Format(Address (Hex) = 0xXF))
//	max7219_ptr->_buf.tx.Add(&max7219_ptr->_buf.tx, reg);
//	max7219_ptr->_buf.rx.Add(&max7219_ptr->_buf.rx, 0);
//
//	/* Shutdown mode configuration */
//	reg  = (uint16_t)(0x0C << 8);			// Shutdown mode register address
//	reg |= (uint16_t)1;						// normal operation (Table 3. Shutdown Register Format (Address (Hex) = 0xXC))
//	max7219_ptr->_buf.tx.Add(&max7219_ptr->_buf.tx, reg);
//	max7219_ptr->_buf.rx.Add(&max7219_ptr->_buf.rx, 0);
//
//	/* Clear all digits */
//	for ( uint8_t i = 0; i < max7219_ptr->_setup.disp_num; i++ ) {
//		MAX7219_SetDigit(max7219_ptr, i, ' ', DISABLE); // hard-coded symbol - it's error-proof
//	}

	// ----------------------------------------------------------
/*
	// Arduino-library-based V1
	int i = 0;
	while ( i++ < 500000 );

	// The  display  drivercan  be  programmed  while  in  shutdown  mode,  andshutdown  mode  can  be  overridden  by  the  display-testfunction

	// 3072 = 0x0C00 -> ShutdownRegister, power-saving mode
	max7219_ptr->_buf.tx.Add(&max7219_ptr->_buf.tx, 3072);
	max7219_ptr->_buf.rx.Add(&max7219_ptr->_buf.rx, 0);

	if ( !MAX7219_SendData(max7219_ptr) ) {
		return (0);
	}

	// some time to get into shutdown mode
	i = 0;
	while ( i++ < 200000 );
	while ( max7219_ptr->base_spi.IsBusy(&max7219_ptr->base_spi) );
	while ( i++ < 200000 ); // wait until full reception occurs
	max7219_ptr->_buf.tx.Remove(&max7219_ptr->_buf.tx, 0);
	max7219_ptr->_buf.rx.Remove(&max7219_ptr->_buf.rx, 0);



	// 3840 = 0x0F00 -> Display-Test, normal operation
	max7219_ptr->_buf.tx.Add(&max7219_ptr->_buf.tx, 3840);
	max7219_ptr->_buf.rx.Add(&max7219_ptr->_buf.rx, 0);

	// 2823 = 0x0B07 -> ScanLimit, 0-7 digits
	max7219_ptr->_buf.tx.Add(&max7219_ptr->_buf.tx, 2823);
	max7219_ptr->_buf.rx.Add(&max7219_ptr->_buf.rx, 0);

	// 2304 = 0x0900 -> DecodeMode, NoDecode
	max7219_ptr->_buf.tx.Add(&max7219_ptr->_buf.tx, 2304);
	max7219_ptr->_buf.rx.Add(&max7219_ptr->_buf.rx, 0);

	if ( !MAX7219_SendData(max7219_ptr) ) {
		return (0);
	}

	// some time to get into shutdown mode
	i = 0;
	while ( i++ < 200000 );
	while ( max7219_ptr->base_spi.IsBusy(&max7219_ptr->base_spi) );
	while ( i++ < 200000 ); // wait until full reception occurs
	max7219_ptr->_buf.tx.Remove(&max7219_ptr->_buf.tx, 0);
	max7219_ptr->_buf.rx.Remove(&max7219_ptr->_buf.rx, 0);
	max7219_ptr->_buf.tx.Remove(&max7219_ptr->_buf.tx, 0);
	max7219_ptr->_buf.rx.Remove(&max7219_ptr->_buf.rx, 0);
	max7219_ptr->_buf.tx.Remove(&max7219_ptr->_buf.tx, 0);
	max7219_ptr->_buf.rx.Remove(&max7219_ptr->_buf.rx, 0);



	// 3073 = 0x0C01 -> ShutdownRegister, NormalOperation
	max7219_ptr->_buf.tx.Add(&max7219_ptr->_buf.tx, 3073);
	max7219_ptr->_buf.rx.Add(&max7219_ptr->_buf.rx, 0);
*/

	// ----------------------------------------------------------
	// Arduino library based V2

	int i = 0;
	while ( i++ < 500000 );




	// 2823 = 0x0B07 -> ScanLimit, 0-7 digits
	max7219_ptr->_buf.tx.Add(&max7219_ptr->_buf.tx, 2823);
	max7219_ptr->_buf.rx.Add(&max7219_ptr->_buf.rx, 0);
	if ( !MAX7219_SendData(max7219_ptr) ) {
		return (0);
	}
	i = 0;
	while ( i++ < 100000 );
	while ( max7219_ptr->base_spi.IsBusy(&max7219_ptr->base_spi) );
	while ( i++ < 100000 ); // wait until full reception occurs
	max7219_ptr->_buf.tx.Remove(&max7219_ptr->_buf.tx, 0);
	max7219_ptr->_buf.rx.Remove(&max7219_ptr->_buf.rx, 0);




	// 2304 = 0x0900 -> DecodeMode, NoDecode
	max7219_ptr->_buf.tx.Add(&max7219_ptr->_buf.tx, 2304);
	max7219_ptr->_buf.rx.Add(&max7219_ptr->_buf.rx, 0);
	if ( !MAX7219_SendData(max7219_ptr) ) {
		return (0);
	}
	i = 0;
	while ( i++ < 100000 );
	while ( max7219_ptr->base_spi.IsBusy(&max7219_ptr->base_spi) );
	while ( i++ < 100000 ); // wait until full reception occurs
	max7219_ptr->_buf.tx.Remove(&max7219_ptr->_buf.tx, 0);
	max7219_ptr->_buf.rx.Remove(&max7219_ptr->_buf.rx, 0);




	// 3073 = 0x0C01 -> ShutdownRegister, NormalOperation
	max7219_ptr->_buf.tx.Add(&max7219_ptr->_buf.tx, 3073);
	max7219_ptr->_buf.rx.Add(&max7219_ptr->_buf.rx, 0);



	/* Send prepared data to the MAX7219 */
	if ( !MAX7219_SendData(max7219_ptr) ) {
		return (0);
	}

	return (1);

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

static uint8_t MAX7219_Print(volatile SOOL_MAX7219 *max7219_ptr, int32_t value) {

	if ( !MAX7219_PrintSection(max7219_ptr, 0, (max7219_ptr->_setup.disp_num - 1), value) ) {
		return (0);
	}
	return (1);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// TODO: DISP_FROM must be smaller than DISP_TO, like DISP_FROM indicates the units whereas DISP_TO shows hundreds for example
// NOTE: DISP_FROM is the index of the first digit the `value` will be printed on
// NOTE: DISP_TO inclusive!
// NOTE: `disp_from` and `disp_to` are digit indexes
static uint8_t MAX7219_PrintSection(volatile SOOL_MAX7219 *max7219_ptr, uint8_t disp_from, uint8_t disp_to,
		int32_t value) {

	/* Allocate some memory */
	char *arr = malloc( (disp_to - disp_from + 1) * sizeof(char));

	// check if allocation was successful
	if ( arr == NULL ) {
		return (0);
	}

	/* Convert value to a character array */
	itoa(value, arr, 10);

	/* Unsupported character flag */
	uint8_t fail_flag = 0;

	/* Helper variable to start from the string's end */
	int8_t str_idx = strlen(arr);

	/* Prepare buffers */
	uint16_t buf_init_size = max7219_ptr->_buf.tx._info.size;
	for ( uint16_t i = 0; i < buf_init_size; i++ ) {
		max7219_ptr->_buf.tx.Remove(&max7219_ptr->_buf.tx, 0);
		max7219_ptr->_buf.rx.Remove(&max7219_ptr->_buf.rx, 0);
	}
//	max7219_ptr->_buf.tx.Clear(&max7219_ptr->_buf.tx);
//	max7219_ptr->_buf.rx.Clear(&max7219_ptr->_buf.rx);
	uint8_t show_dot = 0;

	/* Check whether addition of 0 in front of the DOT is necessary */
	// check dot position within the given section
	uint8_t section_dot_pos = MAX7219_FindDotPosition(max7219_ptr, disp_from, disp_to);
	uint8_t extra_zero_pos = SOOL_MAX7219_DISABLE_DP;
	if ( SOOL_Maths_PowInt(10, section_dot_pos) > value ) {
		// add 0 in front
		extra_zero_pos = section_dot_pos;
	}

//	/* Turn off excessive digits */
//	if ( MAX7219_TurnOffExcessiveDigits(max7219_ptr, disp_from, disp_to, str_idx) ) {
//
//	}

	// iterate over a given display section
	for ( uint8_t i = disp_from; i <= disp_to; i++ ) {

		// add 0 in front of the `dot`
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
		for ( uint8_t j = 0; j < max7219_ptr->_setup.dots._info.size; j++ ) {

			uint8_t dot_disp = max7219_ptr->_setup.dots.Get(&max7219_ptr->_setup.dots, j);
			if ( i == dot_disp ) {
				show_dot = 1;
				break; // stop the inner `for` loop
			}

		}

		// update buffer content
		if ( !MAX7219_SetDigit(max7219_ptr, i, arr[str_idx], show_dot) ) {
			fail_flag = 1;
			break;
		}

	}

	/* Deallocate */
	free(arr);

	if ( !fail_flag ) {
		// FIXME: check whether SendData is called rarely enough for SPI to keep up with each transfer
		return (MAX7219_SendData(max7219_ptr));
	}
	return (0);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t MAX7219_PrintDots(volatile SOOL_MAX7219 *max7219_ptr, uint8_t disp_from, uint8_t disp_to,
		uint8_t dots_num) {

	// TODO:
//	// iterate over a given display section
//	for ( uint8_t i = disp_from; i <= disp_to; i++ ) {
//		MAX7219_SetDigit(max7219_ptr, i, ' ', 1)
//	}
	return (0);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t MAX7219_SetDigit(volatile SOOL_MAX7219 *max7219_ptr, uint8_t digit, char value, uint8_t dot) {

	/* Convert value from human-readable to MAX7219-readable data */
	// prepare register address value (see Table 2. Register Address Map)
	uint8_t address = (digit + 1);

	// prepare MAX7219-readable data (half word, see Table 5. Code B Font)
	uint8_t data = 0;
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

	/* Low level, SPI transfer related section */
	// set D7 according to `dot` variable which is 1 if DP should be enabled
	data |= (dot << 7);

	// prepare half-word variable
	uint16_t reg = 0;
	reg  = (uint16_t)(address << 8);
	reg |= (uint16_t)data;

	// add value to the TX buffer
	max7219_ptr->_buf.tx.Add(&max7219_ptr->_buf.tx, reg);

	// add any value to the RX buffer so it has the same size as TX buffer
	max7219_ptr->_buf.rx.Add(&max7219_ptr->_buf.rx, 0);

	return (1);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t MAX7219_SendData(volatile SOOL_MAX7219 *max7219_ptr) {

	// TODO: cascade mode support?
	uint8_t status = max7219_ptr->base_spi.SendReceive(&max7219_ptr->base_spi,
												   	   &max7219_ptr->base_device,
													   (uint32_t)&max7219_ptr->_buf.rx._data[0],
													   (uint32_t)&max7219_ptr->_buf.tx._data[0],
													   (uint32_t)max7219_ptr->_buf.tx._info.size);
	return (status);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - private functions - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//static uint8_t MAX7219_InitializeOperation(volatile SOOL_MAX7219 *max7219_ptr, uint8_t digits_to_scan) {
//
//
//}

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
// FIXME? is needed?
static uint8_t MAX7219_TurnOffExcessiveDigits(volatile SOOL_MAX7219 *max7219_ptr, uint8_t disp_from, uint8_t disp_to, uint8_t length) {

	int8_t num_excessive_digits = (disp_to - disp_from + 1) - length;

	if ( num_excessive_digits > 0 ) {

		for ( uint8_t i = disp_to; i >= (disp_to - num_excessive_digits); i-- ) {
			if ( !MAX7219_SetDigit(max7219_ptr, i, ' ', 0) ) {

			}
		}

		return (num_excessive_digits);

	} else {

		return (0);

	}

}
