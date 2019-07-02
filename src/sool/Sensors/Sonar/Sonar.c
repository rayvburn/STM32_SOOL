/*
 * Sonar.c
 *
 *  Created on: 24.06.2019
 *      Author: user
 */

#include <sool/Sensors/Sonar/Sonar.h>
#include <sool/Peripherals/TIM/TimerInputCapture.h>

// GPIO initialization
#include <sool/Peripherals/GPIO/PinConfig_NoInt.h>
#include <sool/Peripherals/GPIO/GPIO_common.h>

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t Sonar_StartMeasurement(volatile SOOL_Sonar *sonar_ptr);
static uint8_t Sonar_IsStarted(const volatile SOOL_Sonar *sonar_ptr);
static uint8_t Sonar_IsFinished(const volatile SOOL_Sonar *sonar_ptr);
static uint8_t Sonar_DidTimeout(const volatile SOOL_Sonar *sonar_ptr);
static uint16_t Sonar_GetDistanceCm(const volatile SOOL_Sonar *sonar_ptr);

static uint8_t Sonar_PulseEnd_InterruptHandler(volatile SOOL_Sonar *sonar_ptr);
static uint8_t Sonar_EchoEdge_InterruptHandler(volatile SOOL_Sonar *sonar_ptr);
static uint8_t Sonar_Timeout_InterruptHandler(volatile SOOL_Sonar *sonar_ptr);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// helper functions
static volatile SOOL_Sonar Sonar_InitializeClassHW(
		uint16_t trig_pin, 			GPIO_TypeDef* trig_port, 	uint8_t init_pin_trig,
		uint16_t trig_tim_channel, 	uint8_t init_tim_oc,
		uint16_t echo_pin, 			GPIO_TypeDef* echo_port, 	uint16_t echo_tim_channel,
		TIM_TypeDef* TIMx, 			uint8_t init_tim_base, 		uint16_t range_max,
		SOOL_PinConfig_AltFunction trig_pin_cfg_fcn,
		volatile SOOL_TimerBasic timer_base_fcn,
		volatile SOOL_TimerOnePulse timer_op_fcn);
static uint16_t Sonar_ConvertPulseTimeToCm(uint16_t timer_counter_us);
static uint16_t Sonar_CalculateTimeDiff(uint16_t start, uint16_t end);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/**
 * @brief Fully hardware measurement
 *
 * Trigger pin and port must be connected to a channel of timer (for example T3C1, T4C3)
 * Echo pin and port must be connected to a channel of timer
 *
 * @param trig_pin
 * @param trig_port
 * @param echo_pin
 * @param echo_port
 * @param TIMx
 * @param tim_channel
 * @param range_max
 * @return
 */
volatile SOOL_Sonar SOOL_Sensor_Sonar_Init(uint16_t trig_pin, GPIO_TypeDef* trig_port,
		uint16_t trig_tim_channel, uint16_t echo_pin, GPIO_TypeDef* echo_port,
		uint16_t echo_tim_channel, TIM_TypeDef* TIMx, uint16_t range_max)
{

	/* Create a sonar instance from scratch*/

	// empty instances just for completeness
	SOOL_PinConfig_AltFunction blank_alt_fcn;
	volatile SOOL_TimerBasic blank_timer_base;
	volatile SOOL_TimerOnePulse blank_timer_pulse;

	volatile SOOL_Sonar sonar = Sonar_InitializeClassHW(trig_pin, trig_port, 1, trig_tim_channel, 1,
			echo_pin, echo_port, echo_tim_channel, TIMx, 1, range_max, blank_alt_fcn, blank_timer_base,
			blank_timer_pulse);

	return (sonar);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/**
 * Partial initializer of Sonar, assumes that few sensors use the same trigger signal;
 * range_max parameter must be equal to other sensors sharing trigger pin (and timer)
 * @param echo_pin
 * @param echo_port
 * @param echo_tim_channel
 * @param range_max
 * @param trig_cfg
 * @param timer_base
 * @param timer_pulse
 * @return
 */
volatile SOOL_Sonar SOOL_Sensor_Sonar_InitEcho(uint16_t echo_pin, GPIO_TypeDef* echo_port,
		uint16_t echo_tim_channel, uint16_t range_max, SOOL_PinConfig_AltFunction trig_cfg,
		volatile SOOL_TimerBasic timer_base, volatile SOOL_TimerOnePulse timer_pulse)
{

	/* Create a sonar instance */
	volatile SOOL_Sonar sonar = Sonar_InitializeClassHW(0, 0, 0, 0, 0, echo_pin, echo_port,
			echo_tim_channel, 0, 0, range_max, trig_cfg, timer_base, timer_pulse);

	return (sonar);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/**
 * Partial initializer of Sonar, assumes that few sensors use the same timer peripheral
 * range_max parameter must be equal to other sensors sharing timer
 * @param trig_pin
 * @param trig_port
 * @param trig_tim_channel
 * @param echo_pin
 * @param echo_port
 * @param echo_tim_channel
 * @param range_max
 * @param timer_base
 * @return
 */
volatile SOOL_Sonar SOOL_Sensor_Sonar_InitTrigEcho(uint16_t trig_pin, GPIO_TypeDef* trig_port,
		uint16_t trig_tim_channel, uint16_t echo_pin, GPIO_TypeDef* echo_port,
		uint16_t echo_tim_channel, uint16_t range_max, volatile SOOL_TimerBasic timer_base)
{

	/* Create a sonar instance */

	// empty instances just for completeness
	SOOL_PinConfig_AltFunction blank_alt_fcn;
	volatile SOOL_TimerOnePulse blank_timer_pulse;

	volatile SOOL_Sonar sonar = Sonar_InitializeClassHW(trig_pin, trig_port, 1, trig_tim_channel, 1,
			echo_pin, echo_port, echo_tim_channel, 0, 0, range_max, blank_alt_fcn, timer_base,
			blank_timer_pulse);

	return (sonar);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t Sonar_StartMeasurement(volatile SOOL_Sonar *sonar_ptr) {

//	if ( !timer_busy_flag ) {
//		sonar_ptr->base_trigger.SetHigh(&sonar_ptr->base_trigger);
//		sonar_ptr->_state.distance_cm = 0;
//		sonar_ptr->_state.finished = 0;
//		sonar_ptr->_state.started = 1;
//		sonar_ptr->_state.timeout_occurred = 0;
//		return (1);
//	}
//	return (0);

	/* Generate pulse */
	sonar_ptr->base_tim_out.GeneratePulse(&sonar_ptr->base_tim_out);

	/* Update internal state */
	sonar_ptr->_state.counter_val = 0;
	sonar_ptr->_state.distance_cm = 0;
	sonar_ptr->_state.finished = 0;
	sonar_ptr->_state.started = 1;
	sonar_ptr->_state.timeout_occurred = 0;

	return (1);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t Sonar_IsStarted(const volatile SOOL_Sonar *sonar_ptr) {
	return (sonar_ptr->_state.started);
}
static uint8_t Sonar_IsFinished(const volatile SOOL_Sonar *sonar_ptr) {
	return (sonar_ptr->_state.finished);
}
static uint8_t Sonar_DidTimeout(const volatile SOOL_Sonar *sonar_ptr) {
	return (sonar_ptr->_state.timeout_occurred);
}
static uint16_t Sonar_GetDistanceCm(const volatile SOOL_Sonar *sonar_ptr) {
	return (sonar_ptr->_state.distance_cm);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t Sonar_PulseEnd_InterruptHandler(volatile SOOL_Sonar *sonar_ptr) {

	/* Disable OnePulse Mode of the timer */
	sonar_ptr->base_tim_out.DisableOPMode(&sonar_ptr->base_tim_out);

	/* Set InputCapture polarity to detect rising edge of the signal on echo pin */
	sonar_ptr->base_tim_in.SetPolarity(&sonar_ptr->base_tim_in, TIM_OCPolarity_High);

	return (1);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t Sonar_EchoEdge_InterruptHandler(volatile SOOL_Sonar *sonar_ptr) {

	/* Check if interrupt triggered after timeout */
	if ( sonar_ptr->_state.timeout_occurred ) {
		return (0);
	}

	/* Check state of the echo pin */
	if ( SOOL_Periph_GPIO_ReadInputDataBit(sonar_ptr->base_echo.base.gpio.port, sonar_ptr->base_echo.base.gpio.pin) == 1 ) {

		// ECHO state is high
		if ( sonar_ptr->_state.started && !sonar_ptr->_state.finished ) {

			// save counter value after rising edge
			sonar_ptr->_state.counter_val = sonar_ptr->base_tim_in.GetSavedCounterVal(&sonar_ptr->base_tim_in);

			// set InputCapture polarity to detect falling edge of the signal on echo pin */
			sonar_ptr->base_tim_in.SetPolarity(&sonar_ptr->base_tim_in, TIM_OCPolarity_Low);

		}

	} else {

		// ECHO state is low
		if ( sonar_ptr->_state.started && !sonar_ptr->_state.finished ) {

			// update distance based on previous and current counter values
			uint16_t time_diff = Sonar_CalculateTimeDiff(sonar_ptr->_state.counter_val,
														 sonar_ptr->base_tim_in.GetSavedCounterVal(&sonar_ptr->base_tim_in));
			sonar_ptr->_state.distance_cm = Sonar_ConvertPulseTimeToCm(time_diff);

			// update internal state
			sonar_ptr->_state.finished = 1;

		}

	}

	/* indicate that handler finished job */
	return (1);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t Sonar_Timeout_InterruptHandler(volatile SOOL_Sonar *sonar_ptr) {

	/* No need to clear timer flags, TIM ISR will do it */

	// timer interrupt indicates that timeout has occurred - find sonar which has `started` flag set
	if ( sonar_ptr->_state.started && !sonar_ptr->_state.finished) {

		sonar_ptr->_state.timeout_occurred = 1;
		sonar_ptr->_state.started = 0;
		sonar_ptr->_state.finished = 1;
		sonar_ptr->_state.distance_cm = 0;
		return (1);

	}
	return (0);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - helper functions section  - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// HW - calculations done fully by hardware
// TODO: there may be also an EXTI-based version
static volatile SOOL_Sonar Sonar_InitializeClassHW(
		uint16_t trig_pin, 			GPIO_TypeDef* trig_port, 	uint8_t init_pin_trig,
		uint16_t trig_tim_channel, 	uint8_t init_tim_oc,
		uint16_t echo_pin, 			GPIO_TypeDef* echo_port, 	uint16_t echo_tim_channel,
		TIM_TypeDef* TIMx, 			uint8_t init_tim_base, 		uint16_t range_max,
		SOOL_PinConfig_AltFunction trig_pin_cfg_fcn,
		volatile SOOL_TimerBasic timer_base_fcn,
		volatile SOOL_TimerOnePulse timer_op_fcn)
{

	/* Class instance */
	volatile SOOL_Sonar sonar;

//	/* Initialize timer only when it is not running already.
//	 * Check current conditions. */
//	uint8_t init_time_base, init_timer_oc = 1; 	// whether to initialize timer modules
//
//	if ( SOOL_Periph_TIMCompare_IsCaptureCompareChannelEnabled(TIMx, echo_tim_channel) ) {
//		// echo signals cannot be shared
//		return (sonar);
//	}
//	if ( SOOL_Periph_TIMCompare_IsCaptureCompareChannelEnabled(TIMx, trig_tim_channel) ) {
//		// there may be a shared trigger signal for multiple sensors
//		init_time_base = 0;
//		init_timer_oc = 0;
//	}
//	if ( init_time_base ) {
//
//		// check other channels, make sure time base needs initialization
//		if ( SOOL_Periph_TIMCompare_IsCaptureCompareChannelEnabled(TIMx, TIM_Channel_1) ||
//			 SOOL_Periph_TIMCompare_IsCaptureCompareChannelEnabled(TIMx, TIM_Channel_2) ||
//			 SOOL_Periph_TIMCompare_IsCaptureCompareChannelEnabled(TIMx, TIM_Channel_3) ||
//			 SOOL_Periph_TIMCompare_IsCaptureCompareChannelEnabled(TIMx, TIM_Channel_4) )
//		{
//			init_time_base = 0;
//		}
//
//	}
//	DEPRECATED because there is a need to pass pointer to TimerBasic between objects
//

	/* Trigger pin configuration */
	if ( init_pin_trig ) {
		SOOL_PinConfig_AltFunction trig = SOOL_Periph_GPIO_PinConfig_Initialize_AltFunction(trig_port, trig_pin, GPIO_Mode_AF_PP);
		trig_pin_cfg_fcn = trig;
	}

	/* Echo pin configuration */
	SOOL_PinConfig_AltFunction echo = SOOL_Periph_GPIO_PinConfig_Initialize_AltFunction(echo_port, echo_pin, GPIO_Mode_IN_FLOATING);

	/* TimerBasic configuration */
	if ( init_tim_base ) {

		/* Helper calculations related to timers configuration */
		// Set prescaler to count microseconds
		uint16_t prescaler_us = (uint16_t)(SystemCoreClock / 1000000ul);

		// It takes 29 us for the sound to travel 2 centimeters - use range_max parameter
		// to calculate timeout for timer (abandon started measurement due to long wait
		// for return signal)
		uint16_t period_tout_dist = (uint16_t)(range_max * 29);

		/* TimerBasic initialization */
		volatile SOOL_TimerBasic timer_basic = SOOL_Periph_TIM_TimerBasic_Init(TIMx, prescaler_us,
									period_tout_dist, ENABLE);

		timer_base_fcn = timer_basic;

	}

	/* TimerOutputCompare and TimerOnePulse configuration */
	if ( init_tim_oc ) {

		/* TimerOutputCompare initialization */
		volatile SOOL_TimerOutputCompare timer_oc = SOOL_Periph_TIM_TimerOutputCompare_Init(timer_base_fcn,
									trig_tim_channel, TIM_OCMode_PWM2, 15, ENABLE,
									TIM_OCIdleState_Reset, TIM_OCPolarity_High, TIM_OutputState_Enable);

		/* TimerOnePulse initialization */
		volatile SOOL_TimerOnePulse timer_op = SOOL_Periph_TIM_TimerOnePulse_Init(timer_oc, 0, ENABLE);
		timer_op_fcn = timer_op;

	}

	/* TimerInputCapture initialization */
	volatile SOOL_TimerInputCapture timer_input = SOOL_Periph_TIM_TimerInputCapture_Init(timer_base_fcn,
									echo_tim_channel, TIM_ICPolarity_Rising, ENABLE);

	/* Save internally base classes */
	sonar.base_trigger = trig_pin_cfg_fcn;
	sonar.base_echo = echo;
	sonar.base_tim_out = timer_op_fcn;
	sonar.base_tim_in = timer_input;

	/* Class methods */
	sonar.DidTimeout = Sonar_DidTimeout;
	sonar.GetDistanceCm = Sonar_GetDistanceCm;
	sonar.IsFinished = Sonar_IsFinished;
	sonar.IsStarted = Sonar_IsStarted;

	sonar.StartMeasurement = Sonar_StartMeasurement;
	sonar._EchoEdge_InterruptHandler = Sonar_EchoEdge_InterruptHandler;
	sonar._PulseEnd_InterruptHandler = Sonar_PulseEnd_InterruptHandler;
	sonar._Timeout_InterruptHandler = Sonar_Timeout_InterruptHandler;

	/* Initial state settings */
	sonar._state.distance_cm = 0;
	sonar._state.finished = 0;
	sonar._state.started = 0;
	sonar._state.timeout_occurred = 0;
	sonar._state.counter_val = 0;

	return (sonar);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint16_t Sonar_ConvertPulseTimeToCm(uint16_t timer_counter_us) {
	/* Timer is configured to count microseconds, see speed of sound for calculations */
	return (timer_counter_us / 58u);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

uint16_t Sonar_CalculateTimeDiff(uint16_t start, uint16_t end) {

	/* Copy the counter value on the rising edge to a temporary variable */
	uint16_t temp = start;
	uint16_t diff = 0;

	/* Check whether overflow occurred.
	 * Both edges always occur within one timer period (timeout). */
	if ( end < temp ) {
		// need to consider counter max value, uint16_t max here
		diff = (0xFFFF - temp) + end;
	} else {
		// normal calculations
		diff = end - temp;
	}

	return (diff);

}

