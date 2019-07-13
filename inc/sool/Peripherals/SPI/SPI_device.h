/*
 * SPI_device.h
 *
 *  Created on: 12.07.2019
 *      Author: user
 */

#ifndef INC_SOOL_PERIPHERALS_SPI_SPI_DEVICE_H_
#define INC_SOOL_PERIPHERALS_SPI_SPI_DEVICE_H_

#include "sool/Effectors/PinSwitch/PinSwitch.h"
#include "sool/Memory/BufferAbstract/BufferAbstract.h"

// ---------------------------------------------------

/**
 * @brief Used for switching the device's CS line
 */
typedef struct _SOOL_SPI_DeviceStruct SOOL_SPI_Device;

// ---------------------------------------------------

struct _SOOL_SPI_DeviceSetupStruct {
	SPI_TypeDef*		SPIx;
	uint8_t 			DEV_ID; // non-mutable, device identifier (counter incremented with each AddDevice call)
};

// ---------------------------------------------------

struct _SOOL_SPI_DeviceStruct {

	SOOL_PinSwitch						base;
	struct _SOOL_SPI_DeviceSetupStruct	_setup;

//	uint8_t (*AddData)(volatile SOOL_SPI_Device*);
//	uint8_t	(*GetID)(volatile SOOL_SPI_Device*);	// TODO: probably will not be used in main loop

};

// ---------------------------------------------------

// TODO: initializer?

#endif /* INC_SOOL_PERIPHERALS_SPI_SPI_DEVICE_H_ */
