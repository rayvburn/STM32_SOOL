/*
 * HX711.h
 *
 *  Created on: 08.11.2019
 *      Author: user
 */

#ifndef INC_SOOL_SENSORS_HX711_HX711_H_
#define INC_SOOL_SENSORS_HX711_HX711_H_

#include <sool/Peripherals/GPIO/PinConfig_Int.h>
#include <sool/Peripherals/GPIO/PinConfig_AltFunction.h>
#include <sool/Peripherals/TIM/TimerOutputCompare.h>
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
	int8_t				data_bits_left;			// how many interrupt calls to expect (signed to avoid overflow)

};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

typedef struct _SOOL_HX711Struct SOOL_HX711;

struct _SOOL_HX711Struct {

	// ----------- base class section
	SOOL_PinConfig_Int 				base_dout;
	SOOL_TimerOutputCompare			base_tim_sck;

	// ----------- derived class section
	struct _SOOL_HX711StateStruct 	_state;

	uint8_t		(*IsDataReady)(volatile SOOL_HX711 *hx_ptr);
	int32_t		(*GetData)(volatile SOOL_HX711 *hx_ptr);

	// DEPRECATED: BLOCKING IMPLEMENTATION - blocking for about 30 us (safer because blocks interrupts, see power-down mode)

	uint8_t		(*_TimerInterruptHandler)(volatile SOOL_HX711 *hx_ptr);
	uint8_t 	(*_ExtiInterruptHandler)(volatile SOOL_HX711 *hx_ptr); // a routine fired in a proper ISR (firstly it must check if interrupt has been triggered on sensor's EXTI line)

};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/// @note This implementation requires separate timer for the sensor
/// @note Timer TIMx interrupt (update event) is enabled and must be handled in the application (IRQHandlers setup)
/// @note SCK pin must be wired according to the Timer channel
/// @note Timer must be configured in such a way providing at most 500 kHz frequency (may be lower,
/// but should be higher than 20 kHz due to entrance into power-down mode)
/// @note SCK port and pin could've been automated but better stick to explicit arguments
/// so the user knows what he does
extern volatile SOOL_HX711 SOOL_Sensor_HX711_Init(GPIO_TypeDef* dout_port, uint16_t dout_pin,
												  GPIO_TypeDef* sck_port, uint16_t sck_pin,
												  TIM_TypeDef* TIMx, uint16_t channel,
												  uint8_t gain, int32_t offset, int32_t increment_per_unit);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#endif /* INC_SOOL_SENSORS_HX711_HX711_H_ */
