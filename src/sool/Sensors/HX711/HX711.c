/*
 * HX711.c
 *
 *  Created on: 08.11.2019
 *      Author: user
 */

#include "sool/Sensors/HX711/HX711.h"
#include "sool/Common/Delay.h"
#include "sool/Memory/Array/ArrayInt32.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t	SOOL_HX711_CalculateOffset(volatile SOOL_HX711 *hx_ptr, uint8_t samples_num);
static uint8_t 	SOOL_HX711_IsOffsetCalculated(volatile SOOL_HX711 *hx_ptr);

static uint8_t 	SOOL_HX711_IsDeviceReady(volatile SOOL_HX711 *hx_ptr);
static uint8_t	SOOL_HX711_StartMeasurement(volatile SOOL_HX711 *hx_ptr);
static uint8_t	SOOL_HX711_IsMeasurementReady(volatile SOOL_HX711 *hx_ptr);
static int32_t	SOOL_HX711_GetLastMeasurement(volatile SOOL_HX711 *hx_ptr);

static uint8_t 	SOOL_HX711_InterruptHandler(volatile SOOL_HX711 *hx_ptr);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// helpers
/// \brief Shifts the bit and returns a new value (0 or 1)
/// \param hx_ptr: load cell instance
/// \return
static uint8_t 	SOOL_HX711_ShiftDataBit(volatile SOOL_HX711 *hx_ptr);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

volatile SOOL_HX711 SOOL_Sensor_HX711_Init(GPIO_TypeDef* dout_port, uint16_t dout_pin, GPIO_TypeDef* sck_port,
				  uint16_t sck_pin, uint8_t gain, int32_t offset, int32_t increment_per_unit) {

	volatile SOOL_HX711 load_cell;

	//
	load_cell.base_dout = SOOL_Periph_GPIO_PinConfig_Initialize_Int(dout_port, dout_pin, GPIO_Mode_IN_FLOATING, EXTI_Trigger_Rising_Falling);
	load_cell.base_sck  = SOOL_Periph_GPIO_PinConfig_Initialize_NoInt(sck_port, sck_pin, GPIO_Mode_Out_PP);

	// PD_SCK high (blocks data reception)
	SOOL_Periph_GPIO_SetBits(sck_port, sck_pin);

	// very very unlikely that the sensor offset will be 0 for any application
	if ( offset != 0 ) {
		load_cell._state.offset = offset;
		load_cell._state.flag_offset_calculated = 1;
	}

	// set gain (in terms of bits) according to a given `gain` value
	switch (gain) {

		case(128):
			load_cell._state.gain = 1;
			break;
		case(64):
			load_cell._state.gain = 3;
			break;
		case(32):
			load_cell._state.gain = 2;
			break;
		default: // 128 is default
			load_cell._state.gain = 1;
			break;

	}

	// scale
	load_cell._state.inc_per_unit = increment_per_unit;

	// Pull PD_SCK low - necessary for data retrieval
	SOOL_Periph_GPIO_ResetBits(sck_port, sck_pin);

	// methods
	load_cell.CalculateOffset = SOOL_HX711_CalculateOffset;
	load_cell.GetLastMeasurement = SOOL_HX711_GetLastMeasurement;
	load_cell.IsDeviceReady = SOOL_HX711_IsDeviceReady;
	load_cell.IsMeasurementReady = SOOL_HX711_IsMeasurementReady;
	load_cell.IsOffsetCalculated = SOOL_HX711_IsOffsetCalculated;
	load_cell.StartMeasurement = SOOL_HX711_StartMeasurement;

	load_cell._InterruptHandler = SOOL_HX711_InterruptHandler;

	// state
	load_cell._state.data_bits_left = 1;
	load_cell._state.data_last = 0;
	load_cell._state.data_temp = 0;
	load_cell._state.flag_data_ready = 0;
	load_cell._state.flag_read_started = 0;

	return (load_cell);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t SOOL_HX711_CalculateOffset(volatile SOOL_HX711 *hx_ptr, uint8_t samples_num) {

	// data storage
	SOOL_Array_Int32 data = SOOL_Memory_Array_Int32_Init(samples_num);

	// wait until ready
	while ( !hx_ptr->IsDeviceReady(hx_ptr) );

	// collect as many samples as required
	for ( uint8_t i = 0; i < samples_num; i++ ) {

		while (!hx_ptr->StartMeasurement(hx_ptr));
		while (!hx_ptr->IsMeasurementReady(hx_ptr));
		data.Add(&data, hx_ptr->GetLastMeasurement(hx_ptr));

	}

	// calculate average
	int64_t avg = 0;
	for ( uint8_t i = 0; i < samples_num; i++ ) {
		avg += data._data[i];
	}
	avg /= samples_num;

	// update offset
	hx_ptr->_state.offset = (int32_t)avg;

	return (1);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t SOOL_HX711_IsOffsetCalculated(volatile SOOL_HX711 *hx_ptr) {
	return (hx_ptr->_state.flag_offset_calculated);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t SOOL_HX711_IsDeviceReady(volatile SOOL_HX711 *hx_ptr) {

	/* When  output  data  is  not  ready  for  retrieval,  digital  output  pin  DOUT  is  high.
	 * Serial  clock  input PD_SCK should be low. */

	// set PD_SCK to low
	SOOL_Periph_GPIO_ResetBits(hx_ptr->base_sck.gpio.port, hx_ptr->base_sck.gpio.pin);

	if ( SOOL_Periph_GPIO_ReadInputDataBit(hx_ptr->base_dout._gpio.port, hx_ptr->base_dout._gpio.pin) ) {
		return (0); // not ready (because DOUT is high)
	}

	return (1); 	// ready (DOUT is low)

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t SOOL_HX711_StartMeasurement(volatile SOOL_HX711 *hx_ptr) {

	if ( hx_ptr->_state.flag_read_started ) {
		return (0); // let the previous reading finish
	}

	/* Prepare an indicator for InterruptHandler */
	hx_ptr->_state.flag_read_started = 0;

	/* Enable EXTI on the DOUT line */
	hx_ptr->base_dout.EnableEXTI(&hx_ptr->base_dout);

    /* When output data is not ready for retrieval, digital output pin DOUT is high
     * Serial clock input PD_SCK should be low. */
	SOOL_Periph_GPIO_ResetBits(hx_ptr->base_sck.gpio.port, hx_ptr->base_sck.gpio.pin);

	/* Reset internal state (except the `flag_read_started`) */
	hx_ptr->_state.data_bits_left = 1; // 1 to avoid overflow (just in case), see the ISR
	hx_ptr->_state.flag_data_ready = 0;
	hx_ptr->_state.data_temp = 0;

	return (1);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t SOOL_HX711_IsMeasurementReady(volatile SOOL_HX711 *hx_ptr) {
	uint8_t temp = hx_ptr->_state.flag_data_ready;
	hx_ptr->_state.flag_data_ready = 0;
	return (temp);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static int32_t SOOL_HX711_GetLastMeasurement(volatile SOOL_HX711 *hx_ptr) {
	return (hx_ptr->_state.data_last);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t SOOL_HX711_InterruptHandler(volatile SOOL_HX711 *hx_ptr) {

	if ( hx_ptr->_state.flag_read_started == 0 ) {

		// when DOUT goes to low, it indicates data is ready for retrieval.
		if ( SOOL_Periph_GPIO_ReadInputDataBit(hx_ptr->base_dout._gpio.port, hx_ptr->base_dout._gpio.pin) != 0 ) {
			return (0); // device not ready now
		}

		hx_ptr->_state.flag_read_started = 1;
		hx_ptr->_state.data_bits_left = 24 + 1; // 24 is a sensor-specific value, +1 to avoid overflow after subtraction, see the stop condition below

	} else {

		// check if reading is in progress
		if ( --hx_ptr->_state.data_bits_left > 0 ) {

			// shift a new data bit to the buffer
			SOOL_HX711_ShiftDataBit(hx_ptr);

		} else {

			// The 25th pulse at PD_SCK input will pull DOUT pin back to high.
			// PD_SCK clock pulses should not be less than 25 or more than 27
			// within one conversion period, to avoid causing serial communication error.
			for ( uint8_t i = 0; i < hx_ptr->_state.gain; i++ ) {
				SOOL_Periph_GPIO_SetBits(hx_ptr->base_sck.gpio.port, hx_ptr->base_sck.gpio.pin);
				// TODO: some small delay MAY need to be applied
				SOOL_Periph_GPIO_ResetBits(hx_ptr->base_sck.gpio.port, hx_ptr->base_sck.gpio.pin);
			}

			// finished reading, disable EXTI on the DOUT line
			hx_ptr->base_dout.DisableEXTI(&hx_ptr->base_dout);

			//
			hx_ptr->_state.data_temp ^= 0x800000;

			// convert raw value to appropriate units
			hx_ptr->_state.data_last = (hx_ptr->_state.data_temp + hx_ptr->_state.offset) / hx_ptr->_state.inc_per_unit;

			// update state
			hx_ptr->_state.flag_data_ready = 1;
			hx_ptr->_state.flag_read_started = 0;

		}

	}

	return (1);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// helper
static uint8_t SOOL_HX711_ShiftDataBit(volatile SOOL_HX711 *hx_ptr) { // leaves SCK low

	/* Each PD_SCK pulse shifts out one bit, starting with the MSB bit first,
	 * until all 24 bits are shifted  out. */
	SOOL_Periph_GPIO_SetBits(hx_ptr->base_sck.gpio.port, hx_ptr->base_sck.gpio.pin);
	hx_ptr->_state.data_temp <<= 1; // shift left
	// TODO: some small delay MAY need to be applied
	SOOL_Periph_GPIO_ResetBits(hx_ptr->base_sck.gpio.port, hx_ptr->base_sck.gpio.pin);

	// TODO: some small delay MAY need to be applied
	uint8_t new_bit = SOOL_Periph_GPIO_ReadInputDataBit(hx_ptr->base_dout._gpio.port, hx_ptr->base_dout._gpio.pin);
	hx_ptr->_state.data_temp |= new_bit;

	return (new_bit);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/*
 * //SOOL_Common_DelayUs(1, SystemCoreClock); // incorporation of a timer for such task is an overkill
 */
