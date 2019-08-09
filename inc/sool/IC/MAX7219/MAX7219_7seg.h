/*
 * MAX7219_7seg.h
 *
 *  Created on: 09.08.2019
 *      Author: user
 */

#ifndef INC_SOOL_IC_MAX7219_MAX7219_7SEG_H_
#define INC_SOOL_IC_MAX7219_MAX7219_7SEG_H_

#include "sool/Peripherals/SPI/SPI_DMA.h"
#include "sool/Peripherals/SPI/SPI_device.h"

// NOTE: supports only 7-segment displays, 8x8 matrix driver mode is not supported
// NOTE: cascade mode is not supported here (8 digits are handled at most)

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#define SOOL_MAX7219_NO_DOT	255u

struct _SOOL_MAX7219_SetupStruct {
	SOOL_SPI_Device spi_device;
	uint8_t			disp_dot;
//	uint8_t 		cascade_num;
};

struct _SOOL_MAX7219_BufStruct {
//	SOOL_Vector_Uint16* ptr; // number of vectors equal to number of MAX7219s connected in cascade mode
	SOOL_Vector_Uint16  tx;
	SOOL_Vector_Uint16	rx;  // helper buffer useful to end a transfer after a certain amount of bytes has been sent
							 // FIXME: size of the rx!
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/* Forward declaration */
typedef struct _SOOL_MAX7219_Struct SOOL_MAX7219;

struct _SOOL_MAX7219_Struct {

	// --------- base class ------------ // in fact it is rather a composition but let it be
	SOOL_SPI_DMA 						base_spi;

	// --------------------------------- //
	struct _SOOL_MAX7219_SetupStruct	_setup;
	struct _SOOL_MAX7219_BufStruct		_buf;

	// --------------------------------- //
	uint8_t (*AddDeviceCascade)(volatile SOOL_MAX7219*);

	// SetDotDisplay TODO:

	// TODO:
	uint8_t (*Print)(volatile SOOL_MAX7219*, uint32_t number);

};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

//volatile SOOL_MAX7219


#endif /* INC_SOOL_IC_MAX7219_MAX7219_7SEG_H_ */
