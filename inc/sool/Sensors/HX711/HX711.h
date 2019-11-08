/*
 * HX711.h
 *
 *  Created on: 08.11.2019
 *      Author: user
 */

#ifndef INC_SOOL_SENSORS_HX711_HX711_H_
#define INC_SOOL_SENSORS_HX711_HX711_H_

#include <sool/Peripherals/GPIO/PinConfig_Int.h>
#include <sool/Peripherals/GPIO/PinConfig_NoInt.h>
#include <stdint.h>

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

struct _SOOL_HX711StateStruct {

	uint8_t 			flag_data_ready;
	uint8_t 			flag_read_started;
	uint8_t				flag_offset_calculated;
	uint8_t				gain;
	int32_t 			offset;
	int32_t				inc_per_unit; 			// either grams or kilos, it's up to an user
	int32_t				data_last;				// ability to read the lastly measured value while already started a new measurement
	int32_t				data_temp;
	uint8_t				data_bits_left;			// how many interrupt calls to expect

};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

typedef struct _SOOL_HX711Struct SOOL_HX711;

struct _SOOL_HX711Struct {

	// ----------- base class section
	SOOL_PinConfig_Int 				base_dout;
	SOOL_PinConfig_NoInt			base_sck;

	// ----------- derived class section
	struct _SOOL_HX711StateStruct 	_state;

	/// @brief Measures and updates internal offset value. This is a blocking function, will hold
	/// the whole application until finishes the procedure.
	/// @param hx_ptr
	/// @param samples_num
	/// @return
	uint8_t		(*CalculateOffset)(volatile SOOL_HX711 *hx_ptr, uint8_t samples_num);

	uint8_t 	(*IsOffsetCalculated)(volatile SOOL_HX711 *hx_ptr);

	/// @brief Must forego the potential call of @ref StartMeasurement
	/// @param hx_ptr
	/// @return
	uint8_t		(*IsDeviceReady)(volatile SOOL_HX711 *hx_ptr);
	uint8_t		(*StartMeasurement)(volatile SOOL_HX711 *hx_ptr);
	uint8_t		(*IsMeasurementReady)(volatile SOOL_HX711 *hx_ptr);
	int32_t		(*GetLastMeasurement)(volatile SOOL_HX711 *hx_ptr);

	uint8_t 	(*_InterruptHandler)(volatile SOOL_HX711 *hx_ptr); // a routine fired in a proper ISR (firstly it must check if interrupt has been triggered on sensor's EXTI line)

};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/// @brief
/// @param dout_port
/// @param dout_pin
/// @param sck_port
/// @param sck_pin
/// @param gain
/// @param offset: offset is optional, call calculate offset after that fcn if needed
/// @param increment_per_unit
/// @return
/// @note Remember to enable NVIC of the interrupt-driven DOUT pin (EXTI is enabled during `StartMeasurement` routine
extern volatile SOOL_HX711 SOOL_Sensor_HX711_Init(GPIO_TypeDef* dout_port, uint16_t dout_pin, GPIO_TypeDef* sck_port,
												  uint16_t sck_pin, uint8_t gain, int32_t offset, int32_t increment_per_unit);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#endif /* INC_SOOL_SENSORS_HX711_HX711_H_ */
