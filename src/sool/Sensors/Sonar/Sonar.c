/*
 * Sonar.c
 *
 *  Created on: 24.06.2019
 *      Author: user
 */

#include <sool/Sensors/Sonar/Sonar.h>
#include <sool/Peripherals/TIM/TimerInputCapture.h>

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t Sonar_StartMeasurement(volatile SOOL_Sonar *sonar_ptr);
//static void Sonar_StartContinuousMeasurements(volatile SOOL_Sonar *sonar_ptr);
static uint8_t Sonar_IsStarted(const volatile SOOL_Sonar *sonar_ptr);
static uint8_t Sonar_IsFinished(const volatile SOOL_Sonar *sonar_ptr);
static uint8_t Sonar_DidTimeout(const volatile SOOL_Sonar *sonar_ptr);
static uint16_t Sonar_GetDistanceCm(const volatile SOOL_Sonar *sonar_ptr);

static void Sonar_ReinitTimer();
static void Sonar_FreeTimer();

static uint8_t Sonar_EXTI_InterruptHandler(volatile SOOL_Sonar *sonar_ptr);
static uint8_t Sonar_TIM_InterruptHandler(volatile SOOL_Sonar *sonar_ptr);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// helper functions
static volatile SOOL_Sonar Sonar_InitializeClass(uint16_t trig_pin, GPIO_TypeDef* trig_port, uint16_t echo_pin, GPIO_TypeDef* echo_port);
static uint16_t Sonar_ConvertPulseTimeToCm(uint16_t timer_counter);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// helper - single timer for all sonars
static typedef struct {
	uint16_t		TIMEOUT_PRESCALER;
	uint16_t		TIMEOUT_PERIOD;
	uint16_t 		TIM_Channel_X;
} Sonar_TimerSetup;

static Sonar_TimerSetup timer_setup;
static volatile SOOL_TimerInputCapture timer;
static volatile timer_busy_flag = 0;

// ======================================================================

volatile SOOL_Sonar SOOL_Sensor_Sonar_InitFull(uint16_t trig_pin, GPIO_TypeDef* trig_port,
		uint16_t echo_pin, GPIO_TypeDef* echo_port, TIM_TypeDef* TIMx, uint16_t tim_channel,
		uint16_t range_max) {

	/* Create a sonar instance */
	volatile SOOL_Sonar sonar = Sonar_InitializeClass(trig_pin, trig_port, echo_pin, echo_port);

	/* Set prescaler to count microseconds */
	uint16_t prescaler_us = (uint16_t)(SystemCoreClock / 1000000ul);

	/* It takes 29 us for the sound to travel 2 centimeters - use range_max parameter
	 * to calculate timeout for timer (cancelling started measurement due to long wait
	 * for return signal */
	uint16_t period_tout_dist = (uint16_t)(range_max * 29);

	/* Save timer-related data */
	timer_setup.TIMEOUT_PRESCALER = prescaler_us;
	timer_setup.TIMEOUT_PERIOD = period_tout_dist;
	timer_setup.TIM_Channel_X = tim_channel;

	/* Create input-event driven timer */
	timer = SOOL_Periph_TIM_TimerInputCapture_Init(TIMx, prescaler_us, period_tout_dist, tim_channel, TIM_ICPolarity_BothEdge);

	return (sonar);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

volatile SOOL_Sonar SOOL_Sensor_Sonar_Init(uint16_t trig_pin, GPIO_TypeDef* trig_port,
		uint16_t echo_pin, GPIO_TypeDef* echo_port) {

	volatile SOOL_Sonar sonar = Sonar_InitializeClass(trig_pin, trig_port, echo_pin, echo_port);
	return (sonar);

}

// ======================================================================

static uint8_t Sonar_StartMeasurement(volatile SOOL_Sonar *sonar_ptr) {

	if ( !timer_busy_flag ) {
		sonar_ptr->_setup.trigger.SetHigh(&sonar_ptr->_setup.trigger);
		sonar_ptr->_state.distance_cm = 0;
		sonar_ptr->_state.finished = 0;
		sonar_ptr->_state.started = 1;
		sonar_ptr->_state.timeout_occurred = 0;
		return (1);
	}
	return (0);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

//static void Sonar_StartContinuousMeasurements(volatile SOOL_Sonar *sonar_ptr) {
//
//}

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

static void Sonar_ReinitTimer() {
	timer = SOOL_Periph_TIM_TimerInputCapture_Init(timer.tim_basic._TIMx, timer_setup.TIMEOUT_PRESCALER,
						timer_setup.TIMEOUT_PERIOD, timer_setup.TIM_Channel_X, TIM_ICPolarity_BothEdge);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void Sonar_FreeTimer() {
	TIM_DeInit(timer.tim_basic._TIMx);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t Sonar_EXTI_InterruptHandler(volatile SOOL_Sonar *sonar_ptr) {

	if ( EXTI_GetITStatus(sonar_ptr->_setup.echo.exti.line) == RESET ) {
		// interrupt request on different EXTI Line
		return (0);
	}

	// check state of the echo pin
	if ( GPIO_ReadInputDataBit(sonar_ptr->_setup.echo.gpio.port, sonar_ptr->_setup.echo.gpio.pin) == 1 ) {

		// ECHO state is high - let's start timer
		if ( sonar_ptr->_state.started && !sonar_ptr->_state.finished ) {
			timer.Start(&timer);
		}

	} else {

		// ECHO state is low
		if ( sonar_ptr->_state.started && !sonar_ptr->_state.finished ) {
			timer.Stop(&timer);

			// NOTE - this needs to be processed a little later, it will probably trigger interrupt first
			// (before Timer InputCapture due to filetring procedure)
			sonar_ptr->_state.finished = 1;
			//sonar_ptr->_state.distance_cm
		}

	}


	// clear flag
	EXTI_ClearITPendingBit(sonar_ptr->_setup.echo.exti.line);

	// check if interrupt triggered after timeout
	if ( !sonar_ptr->_state.timeout_occurred ) {

		// sonar's read finished before timeout occurred


	}

	// indicate that handler finished work
	return (1);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t Sonar_TIM_InterruptHandler(volatile SOOL_Sonar *sonar_ptr) {

	// timer interrupt indicates that timeout has occurred - find sonar which has `started` flag set
	if ( sonar_ptr->_state.started ) {

		sonar_ptr->_state.timeout_occurred = 1;
		sonar_ptr->_state.started = 0;
		sonar_ptr->_state.finished = 1;
		sonar_ptr->_state.distance_cm = SOOL_Sonar_DISTANCE_UNKNOWN;
		return (1);

	}
	return (0);

}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - helper functions section  - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static volatile SOOL_Sonar Sonar_InitializeClass(uint16_t trig_pin, GPIO_TypeDef* trig_port,
		uint16_t echo_pin, GPIO_TypeDef* echo_port) {

	volatile SOOL_Sonar sonar;

	// helper
	SOOL_PinConfig_NoInt trig_cfg = SOOL_Periph_GPIO_PinConfig_Initialize_NoInt(trig_port, trig_pin, GPIO_Mode_Out_PP);

	sonar._setup.trigger = SOOL_Effector_PinSwitch_Init(trig_cfg);
	sonar._setup.echo = SOOL_Periph_GPIO_PinConfig_Initialize_Int(echo_port, echo_pin, EXTI_Trigger_Rising_Falling);

	sonar.DidTimeout = Sonar_DidTimeout;
	sonar.GetDistanceCm = Sonar_GetDistanceCm;
	sonar.IsFinished = Sonar_IsFinished;
	sonar.IsStarted = Sonar_IsStarted;

	// interrupt-switching functions
	sonar.SetExtiState = SOOL_Periph_GPIO_PinConfig_ExtiSwitch;
	sonar.SetNvicState = SOOL_Periph_GPIO_PinConfig_NvicSwitch;

	//sonar.StartContinuousMeasurements = Sonar_StartContinuousMeasurements;
	sonar.StartMeasurement = Sonar_StartMeasurement;

	sonar._EXTI_InterruptHandler = Sonar_EXTI_InterruptHandler;
	sonar._TIM_InterruptHandler = Sonar_TIM_InterruptHandler;

	sonar.ReinitTimer = Sonar_ReinitTimer;
	sonar.FreeTimer = Sonar_FreeTimer;

	// initial state
	sonar._state.distance_cm = 0;
	sonar._state.finished = 0;
	sonar._state.started = 0;
	sonar._state.timeout_occurred = 0;

	return (sonar);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint16_t Sonar_ConvertPulseTimeToCm(uint16_t timer_counter) {
	/* Timer is configured to count microseconds, see speed of sound for calculations */
	return (timer_counter / 58u);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
