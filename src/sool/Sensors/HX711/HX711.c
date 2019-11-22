/*
 * HX711.c
 *
 *  Created on: 08.11.2019
 *      Author: user
 */

#include "sool/Sensors/HX711/HX711.h"
#include "sool/Peripherals/GPIO/GPIO_common.h" // SOOL_Periph_GPIO_SetBits
#include "sool/Memory/Array/ArrayInt32.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t	SOOL_HX711_TimerInterruptHandler(volatile SOOL_HX711 *hx_ptr);
static uint8_t 	SOOL_HX711_ExtiInterruptHandler(volatile SOOL_HX711 *hx_ptr);

static uint8_t	SOOL_HX711_IsDataReady(volatile SOOL_HX711 *hx_ptr);
static int32_t 	SOOL_HX711_GetData(volatile SOOL_HX711 *hx_ptr);
static uint8_t	SOOL_HX711_Tare(volatile SOOL_HX711 *hx_ptr, uint8_t samples);

// helper
static uint8_t SOOL_HX711_ReadBit(volatile SOOL_HX711 *hx_ptr) ;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

volatile SOOL_HX711 SOOL_Sensor_HX711_Init(GPIO_TypeDef* dout_port, uint16_t dout_pin,
										   GPIO_TypeDef* sck_port, uint16_t sck_pin,
										   TIM_TypeDef* TIMx, uint16_t channel,
										   uint8_t gain, int32_t offset, int32_t increment_per_unit)
{

	/* HX711-based load cell instance */
	volatile SOOL_HX711 load_cell;

	/* DT / DOUT pin configuration */										          // GPIO_Mode_IN_FLOATING
	load_cell.base_dout = SOOL_Periph_GPIO_PinConfig_Initialize_Int(dout_port, dout_pin, GPIO_Mode_IPU, EXTI_Trigger_Rising_Falling);

	/* Create an Alternative Function pin configuration;
	 * at the moment this does not need to be stored internally,
	 * pure configuration is enough. */
	SOOL_PinConfig_AltFunction sck = SOOL_Periph_GPIO_PinConfig_Initialize_AltFunction(sck_port, sck_pin, GPIO_Mode_AF_PP);

	/* Create an instance of SOOL_TimerBasic */
	// set prescaler to count microseconds
	uint16_t prescaler_us = (uint16_t)(SystemCoreClock / 1000000ul);

	// period of 18 us (for some reason 8-16 us periods don't work - no pulses are generated)
	// whereas for values below 6 the SOFT does not keep up with the HARDWARE and interrupt
	// routine takes too long and thus too many pulses are generated
	uint16_t period = 18; // in microseconds
	volatile SOOL_TimerBasic tim_basic = SOOL_Periph_TIM_TimerBasic_Init(TIMx, prescaler_us, period, DISABLE); // ENABLE);

	/* Create an instance of SOOL_TimerOnePulse */
	// OutputCompare
	SOOL_TimerOutputCompare tim_oc = SOOL_Periph_TIM_TimerOutputCompare_Init(tim_basic, channel, TIM_OCMode_PWM1,
												9, // NOTE: pulse must be in the range: [0 < PULSE < PERIOD]
												ENABLE, TIM_OCIdleState_Reset, TIM_OCPolarity_Low, TIM_OutputState_Enable);
	// OnePulse
	/* NOTE:
	 * at TIMx->CNT == (delay_time) the pulse will be generated;
	 * instant edge is DISABLED to force all pulses have the same length (otherwise
	 * the first one can be degenerated);
	 * Repetition Counter (RCR) value of the TIMx timer is zeroed at that moment (does not matter);
	 * for more info see the TimeOnePulse documentation
	 */
	load_cell.base_tim_sck = SOOL_Periph_TIM_TimerOnePulse_Init(tim_oc, 8, DISABLE);
	tim_basic._setup.TIMx->RCR = 0;

//	// PD_SCK high (blocks data reception)
//	SOOL_Periph_GPIO_SetBits(sck_port, sck_pin);

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

//	// Pull PD_SCK low - necessary for data retrieval
//	SOOL_Periph_GPIO_ResetBits(sck_port, sck_pin);

	// methods
	load_cell.Tare = SOOL_HX711_Tare;
	load_cell.GetData = SOOL_HX711_GetData;
	load_cell.IsDataReady = SOOL_HX711_IsDataReady;

	load_cell._ExtiInterruptHandler  = SOOL_HX711_ExtiInterruptHandler;
	load_cell._TimerInterruptHandler = SOOL_HX711_TimerInterruptHandler;

	// state
	load_cell._state.data_bits_left = -1;
	load_cell._state.data_last = 0;
	load_cell._state.data_temp = 0;
	load_cell._state.flag_data_ready = 0;
	load_cell._state.flag_read_started = 0;

	return (load_cell);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t SOOL_HX711_IsDataReady(volatile SOOL_HX711 *hx_ptr) {
	uint8_t temp = hx_ptr->_state.flag_data_ready;
	hx_ptr->_state.flag_data_ready = 0;
	return (hx_ptr->_state.flag_data_ready);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static int32_t SOOL_HX711_GetData(volatile SOOL_HX711 *hx_ptr) {
	return (hx_ptr->_state.data_last);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t SOOL_HX711_Tare(volatile SOOL_HX711 *hx_ptr, uint8_t samples) {

	SOOL_Array_Int32 measurements = SOOL_Memory_Array_Int32_Init(samples);
	int64_t sum = 0;

	for ( uint8_t i = 0; i < samples; i++ ) {
		while ( !SOOL_HX711_IsDataReady(hx_ptr) );
		measurements.Add(&measurements, SOOL_HX711_GetData(hx_ptr));
		sum += SOOL_HX711_GetData(hx_ptr);
	}

	hx_ptr->_state.offset = sum / samples;
	return (1);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t SOOL_HX711_TimerInterruptHandler(volatile SOOL_HX711 *hx_ptr) {

	if ( !hx_ptr->_state.flag_read_started ) {
		return (0);
	}

	/* Restart counter (in OP Mode Update the event stops counter) */
	hx_ptr->base_tim_sck.base.Start(&hx_ptr->base_tim_sck.base);

	/* To prevent unnecessary pulse generation */
	uint8_t finished = 0;
	--hx_ptr->_state.data_bits_left;

	// do not save the data bits that are exhibited during the next measurement gain selection
	if ( hx_ptr->_state.data_bits_left < hx_ptr->_state.gain && hx_ptr->_state.data_bits_left != 0 ) {
		return (3);
	} else if ( hx_ptr->_state.data_bits_left == 0 ) {
		hx_ptr->base_tim_sck.base.DisableChannel(&hx_ptr->base_tim_sck);
		finished = 1;
	}

	/* Read */
	if ( !finished ) {

		hx_ptr->_state.data_temp |= SOOL_HX711_ReadBit(hx_ptr);
		hx_ptr->_state.data_temp <<= 1;

	} else {

		/* Process whole `word` */
		hx_ptr->_state.data_temp ^= 0x800000;

		// convert raw value to appropriate units
		hx_ptr->_state.data_last = (hx_ptr->_state.data_temp - hx_ptr->_state.offset) / hx_ptr->_state.inc_per_unit;

		// update state
		hx_ptr->_state.flag_data_ready = 1;
		hx_ptr->_state.flag_read_started = 0;

		return (1);

	}

	return (2);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t SOOL_HX711_ExtiInterruptHandler(volatile SOOL_HX711 *hx_ptr) {

	/* DOUT falling edge detection */
	if ( !SOOL_HX711_ReadBit(hx_ptr) && !hx_ptr->_state.flag_read_started ) {

		// reset internal state
		hx_ptr->_state.flag_read_started = 1;
		hx_ptr->_state.data_bits_left = 24 + hx_ptr->_state.gain + 1;

		// stop the timer to reset counter
		hx_ptr->base_tim_sck.base.Stop(&hx_ptr->base_tim_sck);

		// reset timer
		hx_ptr->base_tim_sck.base.base.SetCounter(&hx_ptr->base_tim_sck.base, 0);

		// OnePulse timer - generate a sequence of pulses
		hx_ptr->base_tim_sck.base.base._setup.TIMx->RCR = 24 + hx_ptr->_state.gain + 1;
		hx_ptr->base_tim_sck.base.EnableChannel(&hx_ptr->base_tim_sck.base);
		hx_ptr->base_tim_sck.GeneratePulse(&hx_ptr->base_tim_sck);

		// reset buffer
		hx_ptr->_state.data_temp = 0;

		return (1);

	}

	return (0);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t SOOL_HX711_ReadBit(volatile SOOL_HX711 *hx_ptr) {
	return (SOOL_Periph_GPIO_ReadInputDataBit(hx_ptr->base_dout._gpio.port, hx_ptr->base_dout._gpio.pin));
}

