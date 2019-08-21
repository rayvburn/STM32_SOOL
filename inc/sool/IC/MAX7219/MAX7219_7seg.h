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
	uint8_t 			disp_num;
};

struct _SOOL_MAX7219_BufStruct {
	SOOL_Vector_Uint16  tx;
	SOOL_Vector_Uint16	rx;  // helper buffer useful to end a transfer after a certain amount of bytes has been sent
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/* Forward declaration */
typedef struct _SOOL_MAX7219_Struct SOOL_MAX7219;

struct _SOOL_MAX7219_Struct {

	// --------- base class ------------ // in fact it is rather a composition but let it be
	SOOL_SPI_DMA 						base_spi;
	SOOL_SPI_Device 					base_device;

	// --------------------------------- //
	struct _SOOL_MAX7219_SetupStruct	_setup;
	struct _SOOL_MAX7219_BufStruct		_buf;

	// --------------------------------- //

	uint8_t (*AddDotDisplay)(volatile SOOL_MAX7219 *max7219_ptr, uint8_t dot_disp_num);
	uint8_t (*Print)(volatile SOOL_MAX7219 *max7219_ptr, int32_t value);
	uint8_t (*PrintSection)(volatile SOOL_MAX7219 *max7219_ptr, uint8_t disp_from, uint8_t disp_to, int32_t value);
	uint8_t (*PrintDots)(volatile SOOL_MAX7219 *max7219_ptr, uint8_t disp_from, uint8_t disp_to, uint8_t dots_num);

};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

extern volatile SOOL_MAX7219 SOOL_IC_MAX7219_Initialize(SPI_TypeDef *SPIx, uint8_t do_remap, GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, uint8_t disp_num);

#endif /* INC_SOOL_IC_MAX7219_MAX7219_7SEG_H_ */
