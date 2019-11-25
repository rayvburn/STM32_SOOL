/*
 * HX711Procedural.c
 *
 *  Created on: 22.11.2019
 *      Author: user
 */

#include "sool/Sensors/HX711/HX711Procedural.h"

#include "sool/Common/Delay.h"
#include "sool/Peripherals/GPIO/GPIO_common.h" // SOOL_Periph_GPIO_SetBits
#include "sool/Memory/Array/ArrayInt32.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t SOOL_HX711Procedural_Tare(volatile SOOL_HX711_Procedural *hx_ptr, uint8_t samples);
static uint8_t SOOL_HX711Procedural_IsDataReady(volatile SOOL_HX711_Procedural *hx_ptr);
static int32_t SOOL_HX711Procedural_Read(volatile SOOL_HX711_Procedural *hx_ptr);
//static uint8_t SOOL_HX711Procedural_CompensateDrift(volatile SOOL_HX711_Procedural *hx_ptr, int32_t *reading_ptr);
static void    SOOL_HX711Procedural_PowerSwitch(volatile SOOL_HX711_Procedural *hx_ptr, FunctionalState state);
static uint8_t SOOL_HX711Procedural_ExtiInterruptHandler(volatile SOOL_HX711_Procedural *hx_ptr);

// helper
static int32_t SOOL_HX711Procedural_ReadFull(volatile SOOL_HX711_Procedural *hx_ptr, uint8_t offset_calculation);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

volatile SOOL_HX711_Procedural SOOL_Sensor_HX711_InitProcedural(GPIO_TypeDef* dout_port, uint16_t dout_pin,
		  GPIO_TypeDef* sck_port, uint16_t sck_pin,
		  uint8_t gain, int32_t offset, int32_t increment_per_unit,
		  uint8_t enable_drift_compensation, uint8_t drift_threshold)
{

	/* HX711-based load cell instance */
	volatile SOOL_HX711_Procedural load_cell;

	/* DT/DOUT pin configuration */												    //   GPIO_Mode_IPU
	load_cell.base_dout = SOOL_Periph_GPIO_PinConfig_Initialize_Int(dout_port, dout_pin, GPIO_Mode_IN_FLOATING, EXTI_Trigger_Falling);

	/* SCK pin configuration */
	load_cell.base_sck = SOOL_Periph_GPIO_PinConfig_Initialize_NoInt(sck_port, sck_pin, GPIO_Mode_Out_PP);

	/* Apply the offset */
	load_cell._state.offset = offset;

	/* Set gain (in terms of additional bits number) according to a given `gain` value */
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

	/* Scale */
	load_cell._state.inc_per_unit = increment_per_unit;

	/* Drift */
	load_cell._state.enable_drift_comp = enable_drift_compensation;
	load_cell._drift.drift_threshold = drift_threshold;
	load_cell._drift.offset_neg = 0;
	load_cell._drift.offset_pos = 0;

	/* Assign methods */
//	load_cell.CompensateDrift = SOOL_HX711Procedural_CompensateDrift;
	load_cell.PowerSwitch = SOOL_HX711Procedural_PowerSwitch;
	load_cell.IsDataReady = SOOL_HX711Procedural_IsDataReady;
	load_cell.Read = SOOL_HX711Procedural_Read;
	load_cell.Tare = SOOL_HX711Procedural_Tare;
	load_cell._ExtiInterruptHandler = SOOL_HX711Procedural_ExtiInterruptHandler;

	return (load_cell);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t SOOL_HX711Procedural_Tare(volatile SOOL_HX711_Procedural *hx_ptr, uint8_t samples) {

	SOOL_Array_Int32 measurements = SOOL_Memory_Array_Int32_Init(samples);
	int64_t sum = 0;
	int32_t val = 0;

	for ( uint8_t i = 0; i < samples; i++ ) {
		while (!SOOL_HX711Procedural_IsDataReady(hx_ptr));
		val = SOOL_HX711Procedural_ReadFull(hx_ptr, 1);
		measurements.Add(&measurements, val);
		sum += val;
	}

	hx_ptr->_state.offset = sum / samples;
	return (1);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t SOOL_HX711Procedural_IsDataReady(volatile SOOL_HX711_Procedural *hx_ptr) {
	return (hx_ptr->_state.flag_data_ready);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static int32_t SOOL_HX711Procedural_Read(volatile SOOL_HX711_Procedural *hx_ptr) {
	return (SOOL_HX711Procedural_ReadFull(hx_ptr, 0));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static int32_t SOOL_HX711Procedural_ReadFull(volatile SOOL_HX711_Procedural *hx_ptr, uint8_t offset_calculation) {

	// data container
	int32_t buffer = 0;

	// blocks EXTI interrupts
	hx_ptr->base_dout.DisableEXTI(&hx_ptr->base_dout);
	hx_ptr->_state.flag_read_started = 1;

	// clock SCK line to shift data bits on the DT line
	for (uint8_t i = 0; i < 24; i++)    {

		SOOL_Periph_GPIO_SetBits(hx_ptr->base_sck.gpio.port, hx_ptr->base_sck.gpio.pin);
		SOOL_Common_DelayUs(1, SystemCoreClock);
		buffer = buffer << 1 ;
		SOOL_Periph_GPIO_ResetBits(hx_ptr->base_sck.gpio.port, hx_ptr->base_sck.gpio.pin);
		SOOL_Common_DelayUs(1, SystemCoreClock);

		if ( SOOL_Periph_GPIO_ReadInputDataBit(hx_ptr->base_dout._gpio.port, hx_ptr->base_dout._gpio.pin) ) {
			buffer++;
		}

	}

	// select gain for the next measurement
	for (int i = 0; i < hx_ptr->_state.gain; i++)	 {
		SOOL_Periph_GPIO_SetBits(hx_ptr->base_sck.gpio.port, hx_ptr->base_sck.gpio.pin);
		SOOL_Common_DelayUs(1, SystemCoreClock);
		SOOL_Periph_GPIO_ResetBits(hx_ptr->base_sck.gpio.port, hx_ptr->base_sck.gpio.pin);
		SOOL_Common_DelayUs(1, SystemCoreClock);
	}

	// 2s complement data
	buffer = buffer ^ 0x800000;

	// consider offset and scale if a normal measurement is performed
	if ( !offset_calculation ) {

		// apply scale factor and offset
		buffer = (buffer - hx_ptr->_state.offset) / hx_ptr->_state.inc_per_unit;

		// compensate drift if needed
		if ( hx_ptr->_state.enable_drift_comp ) {
			SOOL_Sensors_HX711_DriftCompensation(&buffer, &hx_ptr->_drift);
		}

	}

	// enable EXTI interrupts processing again
	hx_ptr->base_dout.EnableEXTI(&hx_ptr->base_dout);
	hx_ptr->_state.flag_read_started = 0;
	hx_ptr->_state.flag_data_ready = 0;

	return (buffer);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

//static uint8_t SOOL_HX711Procedural_CompensateDrift(volatile SOOL_HX711_Procedural *hx_ptr, int32_t *reading_ptr) {
//	return ();
//}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void SOOL_HX711Procedural_PowerSwitch(volatile SOOL_HX711_Procedural *hx_ptr, FunctionalState state) {

	switch (state) {

		case(ENABLE):
			SOOL_Periph_GPIO_ResetBits(hx_ptr->base_sck.gpio.port, hx_ptr->base_sck.gpio.pin);
			hx_ptr->base_dout.EnableEXTI(&hx_ptr->base_dout);
			break;

		case(DISABLE):
			hx_ptr->base_dout.DisableEXTI(&hx_ptr->base_dout);
			SOOL_Periph_GPIO_SetBits(hx_ptr->base_sck.gpio.port, hx_ptr->base_sck.gpio.pin);
			break;

	}

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t SOOL_HX711Procedural_ExtiInterruptHandler(volatile SOOL_HX711_Procedural *hx_ptr) {

	if ( hx_ptr->_state.flag_read_started ) {
		return (0);
	}

	/* DOUT falling edge detection */
	// indicate new data availability
	hx_ptr->_state.flag_data_ready = 1;
	return (1);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
