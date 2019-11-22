/*
 * HX711Procedural.h
 *
 *  Created on: 22.11.2019
 *      Author: user
 */

#ifndef INC_SOOL_SENSORS_HX711_HX711PROCEDURAL_H_
#define INC_SOOL_SENSORS_HX711_HX711PROCEDURAL_H_

#include <sool/Peripherals/GPIO/PinConfig_Int.h>	// DT
#include <sool/Peripherals/GPIO/PinConfig_NoInt.h>	// SCK
#include <stdint.h>
#include <sool/Sensors/HX711/HX711_common.h>		// drift

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

struct _SOOL_HX711ProceduralStateStruct {

	uint8_t				gain;
	int32_t 			offset;					// sensor offset
	int32_t				inc_per_unit;			// scale
	uint8_t 			flag_data_ready;		// helper flag
	uint8_t				flag_read_started;		// helper flag
	uint8_t 			enable_drift_comp;

};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

typedef struct _SOOL_HX711ProceduralStruct SOOL_HX711_Procedural;

struct _SOOL_HX711ProceduralStruct {

	// ----------- base class section
	SOOL_PinConfig_Int 						base_dout;
	SOOL_PinConfig_NoInt					base_sck;

	// ----------- derived class section
	struct _SOOL_HX711ProceduralStateStruct _state;
	struct _SOOL_HX711DriftStruct			_drift;

	/// @brief Reads data `samples` number of times; MCU blockage time is samples-number-dependent
	uint8_t		(*Tare)(volatile SOOL_HX711_Procedural *hx_ptr, uint8_t samples);

	/// @brief Clears the internal flag so a call to this method
	/// makes impossible to read the same data twice
	uint8_t		(*IsDataReady)(volatile SOOL_HX711_Procedural *hx_ptr);

	/// @brief Reads data using a serial interface of the IC;
	/// blocks MCU for about 60 microseconds
	/// @param hx_ptr: load cell controller instance
	/// @return a measured value
	int32_t		(*Read)(volatile SOOL_HX711_Procedural *hx_ptr);

//	/// @brief Compensates the load cell drift according to the thresold
//	uint8_t		(*CompensateDrift)(volatile SOOL_HX711_Procedural *hx_ptr, int32_t *reading_ptr);

	/// @brief A routine fired in a proper ISR (if the incoming interrupt has been triggered
	/// on the sensor EXTI line)
	/// @param hx_ptr
	/// @return
	uint8_t 	(*_ExtiInterruptHandler)(volatile SOOL_HX711_Procedural *hx_ptr);

};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/**
 * @brief Constructor of the HX711 controller with a blocking behaviour. Beware that calling a Read method
 * will block MCU for about 60 us.
 * @param dout_port
 * @param dout_pin
 * @param sck_port
 * @param sck_pin
 * @param gain
 * @param offset
 * @param increment_per_unit
 * @param enable_drift_compensation: whether to compensate drift of each reading; when this option
 * is disabled (`0`) one can still compensate drift of e.g. an averaged value of measurements
 * manually calling @ref SOOL_Sensors_HX711_DriftCompensation and passing averaged value as
 * a @ref reading_ptr argument
 * @param drift_threshold: has no effect if @ref enable_drift_compensation is `0` (false)
 * @return
 */
extern volatile SOOL_HX711_Procedural SOOL_Sensor_HX711_InitProcedural(GPIO_TypeDef* dout_port, uint16_t dout_pin,
		  GPIO_TypeDef* sck_port, uint16_t sck_pin,
		  uint8_t gain, int32_t offset, int32_t increment_per_unit,
		  uint8_t enable_drift_compensation, uint8_t drift_threshold);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#endif /* INC_SOOL_SENSORS_HX711_HX711PROCEDURAL_H_ */
