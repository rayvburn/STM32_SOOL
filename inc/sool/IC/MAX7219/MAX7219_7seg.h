/*
 * MAX7219_7seg.h
 *
 *  Created on: 09.08.2019
 *      Author: user
 */

#ifndef INC_SOOL_IC_MAX7219_MAX7219_7SEG_H_
#define INC_SOOL_IC_MAX7219_MAX7219_7SEG_H_

#include "sool/Memory/Vector/VectorUint16.h"
#include "sool/Peripherals/SPI/SPI_DMA.h"
#include "sool/Peripherals/SPI/SPI_device.h"
#include "sool/IC/MAX7219/MAX7219_symbols.h"

// NOTE: supports only 7-segment displays, 8x8 matrix driver mode is not supported
// NOTE: cascade mode is not supported here (8 digits are handled at most)

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#define SOOL_MAX7219_DISABLE_DP	255u

struct _SOOL_MAX7219_SetupStruct {
	SOOL_Vector_Uint16	dots;	// stores IDs of displays which should have dots enabled
	uint8_t				show_dots;
	uint8_t 			disp_num;
	uint8_t				bcd_decode;
};

struct _SOOL_MAX7219_BufStruct {
	SOOL_Vector_Uint16  tx;
	SOOL_Vector_Uint16	rx;  // helper buffer useful to end a transfer after a certain amount of bytes has been sent
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/* Forward declaration */
typedef struct _SOOL_MAX7219_Struct SOOL_MAX7219;

/*
 * NOTE: MAX7219 seems not to keep up with communication when following byte-pairs
 * are sent without any interrupt one by one (DMA buffer). There is some delay
 * required after successful transfer.
 * The delay can be generated via WHILE loop (blocking). Another way to do so is to wait
 * for RX (MISO) line interrupt - a few extra SCK pulses will be generated until next
 * 16 bits will be written into RX buffer. This is much more efficient way but
 * requires calling buffer preparation procedure (for each transfer)
 * after RX interrupt occurs.
 */
struct _SOOL_MAX7219_Struct {

	// --------- base class ------------ // in fact it is rather a composition but let it be
	SOOL_SPI_DMA 						base_spi;
	SOOL_SPI_Device 					base_device;

	// --------------------------------- //
	struct _SOOL_MAX7219_SetupStruct	_setup;
	struct _SOOL_MAX7219_BufStruct		_buf;

	// --------------------------------- //

	uint8_t (*AddDotDisplay)(volatile SOOL_MAX7219 *max7219_ptr, uint8_t dot_disp_num);

	/// @brief Updates MAX7219 driver's internal state. Does not send any data to the device.
	/// Must be called before some Print() function - it does affect only the following
	/// commands.
	/// @param max7219_ptr
	/// @param state
	void (*ShowDots)(volatile SOOL_MAX7219 *max7219_ptr, uint8_t state);

	/// @brief Turns on or off the display.
	/// @param max7219_ptr
	/// @param shutdown: if true (ENABLE), the display will be turned on; if false (DISABLE), the display
	/// will be turned off (matches ST's ENABLE/DISABLE enum)
	/// @return
	uint8_t (*Shutdown)(volatile SOOL_MAX7219 *max7219_ptr, uint8_t shutdown);

	/// @brief Prints a given number on the display.
	/// @param max7219_ptr
	/// @param value
	/// @return 1 if operation successful
	uint8_t (*Print)(volatile SOOL_MAX7219 *max7219_ptr, int32_t value);

	/// @brief Prints a given array of characters on the display (if character is supported).
	/// @param max7219_ptr
	/// @param str: array of characters to print
	/// @param to_free: whether to free the allocated memory, set this 0
	/// if `const char*` is used (i.e. when text in `''` is used)
	/// @return 1 if operation successful (i.e. all characters are supported)
	uint8_t (*PrintString)(volatile SOOL_MAX7219 *max7219_ptr, char* str, uint8_t to_free);

	/// @brief Same as @ref Print() but uses only selected consecutive displays.
	/// @param max7219_ptr
	/// @param disp_from
	/// @param disp_to
	/// @param value
	/// @return
	uint8_t (*PrintSection)(volatile SOOL_MAX7219 *max7219_ptr, uint8_t disp_from, uint8_t disp_to, int32_t value);

	/// @brief Same as @ref PrintString() but uses only selected consecutive displays.
	/// @param max7219_ptr
	/// @param disp_from
	/// @param disp_to
	/// @param string
	/// @param to_free
	/// @return
	uint8_t (*PrintSectionString)(volatile SOOL_MAX7219 *max7219_ptr, uint8_t disp_from, uint8_t disp_to, char* string, uint8_t to_free);

	// interrupt service routines (ISRs)
	uint8_t (*_ReceptionCompleteIrqHandler)(volatile SOOL_MAX7219 *max7219_ptr);

};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/// @brief MAX7219 driver constructor.
/// @note Remember to enable base class' (base_spi) NVICs
/// @param SPIx: SPI instance
/// @param do_remap: flag telling whether remapping needs to be performed
/// @param GPIOx: GPIO port of the CS line
/// @param GPIO_Pin: GPIO pin of the CS line
/// @param disp_num: number of digits, scanning will be limited to this value
/// @return new SOOL_MAX7219 instance
extern volatile SOOL_MAX7219 SOOL_IC_MAX7219_Initialize(SPI_TypeDef *SPIx, uint8_t do_remap, GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, uint8_t disp_num);

/// @brief Configures MAX7219 driver instance to the default state.
/// @note This MUST be called after switching on the NVICs for SPI and DMAs -
/// otherwise the CS line will never be pulled up after successful transfer
/// because ISR (related to RX/MISO line) will not be called.
/// @note This blocks main program for about 20 milliseconds.
/// @param max7219_ptr: MAX7219 driver instance that needs to be configured
/// @param bcd_decode: if true, MAX7219 works in `Code B` decode mode, otherwise in `No decode mode`
/// @note Only full decode and no-decode modes are supported, yet 2 more modes are available.
/// @param test_mode: if true, MAX7219 works in `Display-test mode`
/// @return 1 if configuration was successful
uint8_t SOOL_IC_MAX7219_Configure(volatile SOOL_MAX7219 *max7219_ptr, uint8_t bcd_decode, uint8_t test_mode);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/* Example can be found at:
 * https://gitlab.com/frb-pow/002tubewaterflowmcu/commit/5e38cccb00211c88f02f15a73f9a49150a764e90
 */

/* Test code (without configuration procedure, see above for that):
 *
   	// MAX7219 Test
	while ( max7219.base_spi.IsBusy(&max7219.base_spi) );
	max7219.Print(&max7219, 124);

	SOOL_Common_Delay(2000, SystemCoreClock);
	max7219.Print(&max7219, 29);
	SOOL_Common_Delay(2000, SystemCoreClock);
	max7219.Print(&max7219, 3);
	SOOL_Common_Delay(2000, SystemCoreClock);
	max7219.PrintSection(&max7219, 0, 2, 111);
	max7219.PrintSection(&max7219, 3, 5, 444);
	SOOL_Common_Delay(2000, SystemCoreClock);
	max7219.PrintSection(&max7219, 3, 5, 111);
	max7219.PrintSection(&max7219, 0, 2, 444);
	SOOL_Common_Delay(2000, SystemCoreClock);
	max7219.PrintSectionString(&max7219, 3, 5, "---", 0);
	max7219.PrintSectionString(&max7219, 0, 2, "---", 0);
	SOOL_Common_Delay(2000, SystemCoreClock);
	max7219.ShowDots(&max7219, DISABLE);
	max7219.PrintSectionString(&max7219, 3, 5, "---", 0);
	max7219.PrintSectionString(&max7219, 0, 2, "---", 0);
	SOOL_Common_Delay(2000, SystemCoreClock);
	max7219.ShowDots(&max7219, ENABLE);
	max7219.PrintSectionString(&max7219, 3, 5, "---", 0);
	max7219.PrintSectionString(&max7219, 0, 2, "---", 0);
	SOOL_Common_Delay(2000, SystemCoreClock);
	max7219.Shutdown(&max7219, DISABLE);
 */

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#endif /* INC_SOOL_IC_MAX7219_MAX7219_7SEG_H_ */
