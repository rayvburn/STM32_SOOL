/*
 * HX711.c
 *
 *  Created on: 08.11.2019
 *      Author: user
 */

#include "sool/Sensors/HX711/HX711.h"
#include "sool/Common/Delay.h"
#include "sool/Memory/Array/ArrayInt32.h"
#include "sool/Peripherals/GPIO/GPIO_common.h" // SOOL_Periph_GPIO_SetBits

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t	SOOL_HX711_TimerInterruptHandler(volatile SOOL_HX711 *hx_ptr);
static uint8_t 	SOOL_HX711_ExtiInterruptHandler(volatile SOOL_HX711 *hx_ptr);

static uint8_t	SOOL_HX711_IsDataReady(volatile SOOL_HX711 *hx_ptr);
static uint8_t 	SOOL_HX711_GetData(volatile SOOL_HX711 *hx_ptr);

// helper
static uint8_t SOOL_HX711_ReadBit(volatile SOOL_HX711 *hx_ptr) ;

// temp
static void SOOL_HX711_SetHelperPinTIM(FunctionalState state);
static void SOOL_HX711_SetHelperPinEXTI(FunctionalState state);

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

	/* Create an Alternative Functinon pin configuration;
	 * at the moment this does not need to be stored internally,
	 * pure configuration is enough. */
	SOOL_PinConfig_AltFunction sck = SOOL_Periph_GPIO_PinConfig_Initialize_AltFunction(sck_port, sck_pin, GPIO_Mode_AF_PP);

	////// helper
	///
	SOOL_Periph_GPIO_PinConfig_Initialize_NoInt(GPIOA, GPIO_Pin_9, GPIO_Mode_Out_PP);  // EXTI
	SOOL_Periph_GPIO_PinConfig_Initialize_NoInt(GPIOA, GPIO_Pin_12, GPIO_Mode_Out_PP); // TIM
	///
	///

	/* Create an instance of SOOL_TimerBasic */
	// set prescaler to count microseconds
//	uint16_t prescaler_us = (uint16_t)(SystemCoreClock / 2000000ul);
	uint16_t prescaler_us = (uint16_t)(SystemCoreClock / 1000000ul);
//	uint16_t period = 20;
	uint16_t period = 6; // 2 us
	volatile SOOL_TimerBasic tim_basic = SOOL_Periph_TIM_TimerBasic_Init(TIMx, prescaler_us, period, DISABLE); // ENABLE);

	/* Create an instance of SOOL_TimerOutputCompare */
//	load_cell.base_tim_sck = SOOL_Periph_TIM_TimerOutputCompare_Init(tim_basic, channel, TIM_OCMode_PWM2, 1,
//								DISABLE, TIM_OCIdleState_Set, TIM_OCPolarity_Low, TIM_OutputState_Enable);
	load_cell.base_tim_sck = SOOL_Periph_TIM_TimerOutputCompare_Init(tim_basic, channel, TIM_OCMode_Toggle, // TIM_OCMode_Toggle,
								//10, // NOTE: pulse must be in the range: [0 < PULSE < PERIOD]
								3,
								ENABLE, TIM_OCIdleState_Reset, TIM_OCPolarity_Low, TIM_OutputState_Enable);

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

static uint8_t SOOL_HX711_GetData(volatile SOOL_HX711 *hx_ptr) {
	return (hx_ptr->_state.data_last);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t SOOL_HX711_TimerInterruptHandler(volatile SOOL_HX711 *hx_ptr) {

	if ( !hx_ptr->_state.flag_read_started ) {
		return (0);
	}

	SOOL_HX711_SetHelperPinTIM(ENABLE);
	/* To prevent unnecessary pulse generation */
	uint8_t finished = 0;
	if ( --hx_ptr->_state.data_bits_left == 0 ) {
		hx_ptr->base_tim_sck.base.DisableNVIC(&hx_ptr->base_tim_sck.base);
		hx_ptr->base_tim_sck.DisableChannel(&hx_ptr->base_tim_sck);
		hx_ptr->base_tim_sck.base.EnableNVIC(&hx_ptr->base_tim_sck.base);
		finished = 1;
	}

	/* Read */
	hx_ptr->_state.data_temp |= SOOL_HX711_ReadBit(hx_ptr);
	hx_ptr->_state.data_temp <<= 1;

	/* Process whole `word` */
	if ( finished ) {

		hx_ptr->_state.data_temp ^= 0x800000;

//		hx_ptr->base_tim_sck.Stop(&hx_ptr->base_tim_sck);

		// convert raw value to appropriate units
		hx_ptr->_state.data_last = (hx_ptr->_state.data_temp + hx_ptr->_state.offset) / hx_ptr->_state.inc_per_unit;

		// update state
		hx_ptr->_state.flag_data_ready = 1;
		hx_ptr->_state.flag_read_started = 0;
		SOOL_HX711_SetHelperPinEXTI(DISABLE);
		SOOL_HX711_SetHelperPinTIM(DISABLE);
		return (1);

	}

	SOOL_HX711_SetHelperPinTIM(DISABLE);
	return (2);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t SOOL_HX711_ExtiInterruptHandler(volatile SOOL_HX711 *hx_ptr) {

	/* DOUT falling edge detection */
	if ( !SOOL_HX711_ReadBit(hx_ptr) && !hx_ptr->_state.flag_read_started ) {

		// reset internal state
		hx_ptr->_state.flag_read_started = 1;
		hx_ptr->_state.data_bits_left = 24 + hx_ptr->_state.gain; //  - 1; TOGGLE CC

		// stop the timer to reset counter
		hx_ptr->base_tim_sck.Stop(&hx_ptr->base_tim_sck);

		// reset timer
		hx_ptr->base_tim_sck.base.SetCounter(&hx_ptr->base_tim_sck.base, 0);

		SOOL_HX711_SetHelperPinEXTI(ENABLE);

		// enable OutputCompare channel of the timer
		hx_ptr->base_tim_sck.EnableChannel(&hx_ptr->base_tim_sck);

		// start the counter
		hx_ptr->base_tim_sck.Start(&hx_ptr->base_tim_sck);

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

// - - - -

static void SOOL_HX711_SetHelperPinTIM(FunctionalState state) {

	if ( state == ENABLE ) {
		SOOL_Periph_GPIO_SetBits(GPIOA, GPIO_Pin_12);
	} else {
		SOOL_Periph_GPIO_ResetBits(GPIOA, GPIO_Pin_12);
	}

}

static void SOOL_HX711_SetHelperPinEXTI(FunctionalState state) {

	if ( state == ENABLE ) {
		SOOL_Periph_GPIO_SetBits(GPIOA, GPIO_Pin_9);
	} else {
		SOOL_Periph_GPIO_ResetBits(GPIOA, GPIO_Pin_9);
	}

}
