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
static uint8_t 	SOOL_HX711_TimerStuckInterruptHandler(volatile SOOL_HX711 *hx_ptr);
static uint8_t 	SOOL_HX711_ExtiInterruptHandler(volatile SOOL_HX711 *hx_ptr);

static uint8_t	SOOL_HX711_IsDataReady(volatile SOOL_HX711 *hx_ptr);
static int32_t 	SOOL_HX711_GetData(volatile SOOL_HX711 *hx_ptr);
static uint8_t	SOOL_HX711_Tare(volatile SOOL_HX711 *hx_ptr, uint8_t samples);
static void 	SOOL_HX711_PowerSwitch(volatile SOOL_HX711 *hx_ptr, FunctionalState state);

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

	/*
	 * RM0008 Revision 19, p. 306,
	 * 14.3.3 Repetition counter:
	 * the update event (UEV) is generated with respect to the counter
	 * overflows/underflows. It is actually generated only when the repetition
	 * counter has reached zero.
	 *
	 * So, the sensor controller cannot depend on Update interrupts (counter stops after Update event
	 * in OnePulse mode, but rather on CaptureCompare events
	 */

	/* Create an instance of SOOL_TimerBasic */
	// set prescaler to count microseconds
	uint16_t prescaler_us = (uint16_t)(SystemCoreClock / 1000000ul);

	// period of 18 us (for some reason 8-16 us periods don't work - no pulses are generated)
	// whereas for values below 6 the SOFT does not keep up with the HARDWARE and interrupt
	// routine takes too long and thus too many pulses are generated
	uint16_t period = 18; // in microseconds
	volatile SOOL_TimerBasic tim_basic = SOOL_Periph_TIM_TimerBasic_Init(TIMx, prescaler_us, period, DISABLE);

	/* Create an instance of SOOL_TimerOnePulse (with a repetition counter) */
	// OutputCompare
	SOOL_TimerOutputCompare tim_oc = SOOL_Periph_TIM_TimerOutputCompare_Init(tim_basic, channel, TIM_OCMode_PWM2,
												9, // NOTE: pulse must be in the range: [0 < PULSE < PERIOD]
												ENABLE,// DISABLE, //
												TIM_OCIdleState_Reset, TIM_OCPolarity_Low, TIM_OutputState_Enable);
	// OnePulse
	/* NOTE:
	 * at TIMx->CNT == (delay_time) the pulse will be generated;
	 * instant edge is DISABLED to force all pulses have the same length (otherwise
	 * the first one can be degenerated);
	 * Repetition Counter (RCR) value of the TIMx timer is zeroed at that moment (does not matter);
	 * for more info see the TimeOnePulse documentation
	 */
	load_cell.base_tim_sck = SOOL_Periph_TIM_TimerOnePulse_Init(tim_oc, 8, DISABLE, ENABLE);
	tim_basic._setup.TIMx->RCR = 0;

	// very very unlikely that the sensor offset will be 0 for any application
	if ( offset != 0 ) {
		load_cell._state.offset = offset;
		load_cell._state.flag_offset_calculated = 1;
	} else {
		load_cell._state.flag_offset_calculated = 0;
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

	// methods
	load_cell.Tare = SOOL_HX711_Tare;
	load_cell.GetData = SOOL_HX711_GetData;
	load_cell.IsDataReady = SOOL_HX711_IsDataReady;
	load_cell.PowerSwitch = SOOL_HX711_PowerSwitch;

	load_cell._ExtiInterruptHandler  = SOOL_HX711_ExtiInterruptHandler;
	load_cell._TimerInterruptHandler = SOOL_HX711_TimerInterruptHandler;
	load_cell._TimerStuckInterruptHandler = SOOL_HX711_TimerStuckInterruptHandler;

	// state
	load_cell._state.data_bits_left = -1;
	load_cell._state.data_last = 0;
	load_cell._state.data_temp = 0;
	load_cell._state.flag_data_ready = 0;
	load_cell._state.flag_read_started = 0;

	// disable sensor
	//SOOL_HX711_PowerSwitch(&load_cell, DISABLE);

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

	/* Evaluate if the channel is enabled */
	uint8_t redisable = 0;
	if ( hx_ptr->_state.flag_power_off ) {
		redisable = 1;
		SOOL_HX711_PowerSwitch(hx_ptr, ENABLE);
	}

	// reset `flag_offset_calculated` to prevent subtraction and division
	// operations in timer interrupt handler
	hx_ptr->_state.flag_offset_calculated = 0;

	// create a storage for readings
	SOOL_Array_Int32 measurements = SOOL_Memory_Array_Int32_Init(samples);
	int64_t sum = 0;
	int32_t value = 0;

	// repeat read as many times as `samples` number states
	for ( uint8_t i = 0; i < samples; i++ ) {
		while ( !SOOL_HX711_IsDataReady(hx_ptr) );
		value = SOOL_HX711_GetData(hx_ptr);
		measurements.Add(&measurements, value);
		sum += value;
	}

	// calculate offset
	hx_ptr->_state.offset = sum / samples;

	// restore previous state of the channel
	if ( redisable ) {
		SOOL_HX711_PowerSwitch(hx_ptr, DISABLE);
	}

	// set offset calculated flag
	hx_ptr->_state.flag_offset_calculated = 1;

	return (1);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void SOOL_HX711_PowerSwitch(volatile SOOL_HX711 *hx_ptr, FunctionalState state) {

	hx_ptr->_state.flag_read_started = 0;

	// stop the counter for a moment
	hx_ptr->base_tim_sck.base.Stop(&hx_ptr->base_tim_sck.base);
	switch (state) {

		case(ENABLE):
			//hx_ptr->base_tim_sck.base.EnableChannel(&hx_ptr->base_tim_sck.base);
			//TODO: experimental - clear potential pending interrupt flag
//			hx_ptr->base_tim_sck.base.

			hx_ptr->base_tim_sck.base.EnableChannel(&hx_ptr->base_tim_sck.base);
			SOOL_Periph_TIMCompare_SetInterruptMask(hx_ptr->base_tim_sck.base.base._setup.TIMx, hx_ptr->base_tim_sck.base._setup.TIM_Channel_x, ENABLE);
			hx_ptr->base_dout.EnableEXTI(&hx_ptr->base_dout);
			hx_ptr->_state.flag_power_off = 0;
			break;

		case(DISABLE):
			/* Disabling the channel is not enough to prevent interrupt generation - RM0008 states:
			 * "(OC mode) Generates an interrupt if the corresponding interrupt mask is set (CCXIE bit in the
			 * TIMx_DIER register)".
			 * So the CCxIE should be reset (TIMx_DIER register).
			 */
			SOOL_Periph_TIMCompare_SetInterruptMask(hx_ptr->base_tim_sck.base.base._setup.TIMx, hx_ptr->base_tim_sck.base._setup.TIM_Channel_x, DISABLE);
			hx_ptr->base_tim_sck.base.DisableChannel(&hx_ptr->base_tim_sck.base);
			hx_ptr->base_dout.DisableEXTI(&hx_ptr->base_dout);
//			SOOL_Periph_TIMCompare_ForceOutput(hx_ptr->base_tim_sck.base.base._setup.TIMx, hx_ptr->base_tim_sck.base._setup.TIM_Channel_x, 1);
			hx_ptr->_state.flag_power_off = 1;
			break;

	}

	// restart the counter
	hx_ptr->base_tim_sck.base.Start(&hx_ptr->base_tim_sck.base);

	// manually clear the interrupt flag (will be possibly pending before `enable` call)
	hx_ptr->base_tim_sck.base.base._setup.TIMx->SR = (uint16_t)~hx_ptr->base_tim_sck.base._setup.TIM_IT_CCx;

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t SOOL_HX711_TimerInterruptHandler(volatile SOOL_HX711 *hx_ptr) {

	/* Evaluate if reading is in progress */
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

		/* Evaluate whether the offset has already been calculated */
		if ( hx_ptr->_state.flag_offset_calculated ) {
			// convert raw value to appropriate units
			hx_ptr->_state.data_last = (hx_ptr->_state.data_temp - hx_ptr->_state.offset) / hx_ptr->_state.inc_per_unit;
		} else {
			hx_ptr->_state.data_last = hx_ptr->_state.data_temp;
		}

		// update state
		hx_ptr->_state.flag_data_ready = 1;
		hx_ptr->_state.flag_read_started = 0;

		return (1);

	}

	return (2);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t SOOL_HX711_TimerStuckInterruptHandler(volatile SOOL_HX711 *hx_ptr) {

	/* FIXME: a strange behaviour is present - after PowerSwitch call with DISABLE argument
	 * the OC channel (SCK) is disabled and CCxIF flag of the SR register cannot be cleared.
	 * However, the interrupt flag is pending and blocks the MCU.
	 * An extra condition (below) is evaluated to prevent the situation described above. */

	// check if CC channel is disabled and the interrupt flag is set (STRANGE)
	if ( !SOOL_Periph_TIMCompare_IsCaptureCompareChannelEnabled(hx_ptr->base_tim_sck.base.base._setup.TIMx, hx_ptr->base_tim_sck.base._setup.TIM_Channel_x)
		 && TIM_GetITStatus(hx_ptr->base_tim_sck.base.base._setup.TIMx, hx_ptr->base_tim_sck.base._setup.TIM_IT_CCx) != RESET )
	{
		//hx_ptr->base_tim_sck.base.base._setup.TIMx->SR = (uint16_t)~hx_ptr->base_tim_sck.base._setup.TIM_IT_CCx;
		return (1);
	}
	return (0);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t SOOL_HX711_ExtiInterruptHandler(volatile SOOL_HX711 *hx_ptr) {

	/* DOUT falling edge detection */
	if ( !SOOL_HX711_ReadBit(hx_ptr) && !hx_ptr->_state.flag_read_started // && !hx_ptr->_state.flag_power_off
			) {

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
		hx_ptr->base_tim_sck.GeneratePulse(&hx_ptr->base_tim_sck); // sets counter, among others

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

