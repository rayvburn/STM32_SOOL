/*
 * StepperMotor.c
 *
 *  Created on: 04.10.2018
 *      Author: user
 */

#include "StepperTB6600.h"
#include "USART.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static TIM_TimeBaseInitTypeDef 	tim_base_tb6600;
static TIM_OCInitTypeDef 		tim_oc_tb6600;
static uint16_t 				pwm_period = 1000;
static uint16_t 				pwm_pulse_width = 0;
volatile static Stepper_State	pos_state = {0, CCW, 0, 0, 1};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void StepperTB6600_Init	() {

	// start of rcc clocks (GPIO ports and timer) || adjust if port migration occurs! ||
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	// init gpio
	GPIO_InitTypeDef gpio;
	GPIO_StructInit(&gpio);

	gpio.GPIO_Mode = GPIO_Mode_Out_PP;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;

	gpio.GPIO_Pin = STEPPER_TB6600_DIR_PIN;		// Direction
	GPIO_Init(STEPPER_TB6600_DIR_PORT, &gpio);
	gpio.GPIO_Pin = STEPPER_TB6600_EN_PIN;		// Enable
	GPIO_Init(STEPPER_TB6600_EN_PORT, &gpio);

	gpio.GPIO_Mode = GPIO_Mode_AF_PP;
	gpio.GPIO_Pin = STEPPER_TB6600_PUL_PIN;		// Pulse
	GPIO_Init(STEPPER_TB6600_PUL_PORT, &gpio);

	// init timer and its interrupts
	tim_base_tb6600.TIM_CounterMode = TIM_CounterMode_Up;
	tim_base_tb6600.TIM_Prescaler = 72 - 1;
	tim_base_tb6600.TIM_Period = pwm_period - 1;
	TIM_TimeBaseInit(TIM2, &tim_base_tb6600);
	TIM_ITConfig(TIM2, TIM_IT_Update | TIM_IT_CC2, ENABLE);
	TIM_Cmd(TIM2, ENABLE);

	// init pwm channel
	StepperTB6600_UpdatePulseWidth();
	TIM_OCStructInit(&tim_oc_tb6600);
	tim_oc_tb6600.TIM_OCMode = TIM_OCMode_PWM2;
	tim_oc_tb6600.TIM_Pulse  = pwm_pulse_width;
	tim_oc_tb6600.TIM_OutputState = TIM_OutputState_Enable;
	tim_oc_tb6600.TIM_OCPolarity  = TIM_OCPolarity_High;
	TIM_OC2Init(TIM2, &tim_oc_tb6600);	// OC2 because output pin is PA1

	// init timer's nvic
	NVIC_InitTypeDef nvic;
	nvic.NVIC_IRQChannel = TIM2_IRQn;
	nvic.NVIC_IRQChannelPreemptionPriority = 0;
	nvic.NVIC_IRQChannelSubPriority = 0;
	nvic.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvic);

	StepperTB6600_SetEn(STOP);

	pos_state.pos_reached = 1;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void StepperTB6600_SetSpeed	(uint16_t speed) {

	if ( !(speed == 0) ) {

		TIM_Cmd(TIM2, DISABLE);							// stop clock

		pwm_period = speed;								// load new period value
		tim_base_tb6600.TIM_Period = pwm_period - 1;	// set new pwm frequency
		TIM_TimeBaseInit(TIM2, &tim_base_tb6600);		// load data

		StepperTB6600_UpdatePulseWidth();				// update duty cycle to generate square wave
		tim_oc_tb6600.TIM_Pulse = pwm_pulse_width;		// load duty cycle value to struct
		TIM_OC2Init(TIM2, &tim_oc_tb6600);				// update OC

		TIM_Cmd(TIM2, ENABLE);							// start clock

	} else {
		StepperTB6600_SetEn(STOP);
	}

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void StepperTB6600_UpdatePulseWidth() {
	// duty is always 50% in stepper driver (square wave) - there's no point adjusting the value in another way
	pwm_pulse_width = pwm_period/2;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void StepperTB6600_SetDir (StepperDriverDir dir) {

	StepperTB6600_SetEn(STOP);

	switch (dir) {

	case(CW):
		//pos_state->dir = CW;
		pos_state.dir = CW;
		GPIO_SetBits(STEPPER_TB6600_DIR_PORT, STEPPER_TB6600_DIR_PIN);
		break;

	case(CCW):
		//pos_state->dir = CCW;
		pos_state.dir = CCW;
		GPIO_ResetBits(STEPPER_TB6600_DIR_PORT, STEPPER_TB6600_DIR_PIN);
		break;

	}

	StepperTB6600_SetEn(ROTATE);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void StepperTB6600_SetEn (StepperDriverEn state) {

	// inverted logic here
	switch (state) {

	case(ROTATE):
		TIM_Cmd(TIM2, ENABLE);		// start timer
		GPIO_SetBits(STEPPER_TB6600_EN_PORT, STEPPER_TB6600_EN_PIN);

		break;

	case(STOP):
		TIM_Cmd(TIM2, DISABLE);		// stop timer
		TIM_SetCounter(TIM2, 0); 	// set timer's counter to 0
		GPIO_ResetBits(STEPPER_TB6600_EN_PORT, STEPPER_TB6600_EN_PIN);
		break;

	}

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

uint8_t StepperTB6600_SetLimitPos(int64_t pos_limit) {

	if ( pos_limit != pos_state.pos ) {
		if ( pos_limit > pos_state.pos ) {
			StepperTB6600_SetDir(CCW);
			USART_SendString(DEBUGGING, "CCW ");
		} else {
			StepperTB6600_SetDir(CW);
			USART_SendString(DEBUGGING, " CW ");
		}
		pos_state.pos_limit = pos_limit;
		pos_state.pos_reached = 0;
		pos_state.limit_set = 1;
		StepperTB6600_SetSpeed(3*STEPPER_INITIAL_SPEED_PULSE); // very slow movement
		StepperTB6600_SetEn(ROTATE);
		return 1;
	} else {
		return 0;
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

uint8_t StepperTB6600_IsLimitSet() {
	return pos_state.limit_set;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

int64_t StepperTB6600_GetCurrentPos() {
	return pos_state.pos;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

int64_t StepperTB6600_GetLimitPos() {
	return pos_state.pos_limit;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

uint8_t StepperTB6600_PosReached() {
	uint8_t temp = pos_state.pos_reached;
	pos_state.pos_reached = 0;
	return temp;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

uint16_t StepperTB6600_GetRPM() {

	// Mikroma FA-34
	const uint8_t STEPS_PER_REV = 200;
	const uint8_t MICROSTEPS_PER_FULL_STEP = 8;

	/* for cpu_freq  = 72 MHz and prescaler = 72 we get timer base of 1 MHz
	 * which gives 1 000 000 pulses / sec (~frequency)
	 * considering some timer ~period, number of pulses per sec will be ~frequency / ~period
	 */

	// no floating point operations needed at now
	uint32_t pulses_per_sec = (uint32_t)( 1000000 / pwm_period );
	uint16_t rpm = (uint16_t)( 60 * pulses_per_sec / (STEPS_PER_REV * MICROSTEPS_PER_FULL_STEP) );
	return rpm;

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void StepperTB6600_ResetMotorPosition() {

	TIM_Cmd(TIM2, DISABLE);		// stop timer
	TIM_SetCounter(TIM2, 0); 	// set timer's counter to 0
	pos_state.limit_set = 0;
	pos_state.pos = 0;
	pos_state.pos_limit = 0;
	pos_state.pos_reached = 1;

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void TIM2_IRQHandler() {

	if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET) {
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);

	} else if (TIM_GetITStatus(TIM2, TIM_IT_CC2) == SET) {
		TIM_ClearITPendingBit(TIM2, TIM_IT_CC2);

		if ( pos_state.dir == CCW ) {
			pos_state.pos++;
		} else {
			pos_state.pos--;
		}

		if ( pos_state.limit_set ) { // check if position limit exists

			// check if pos limit was not reached
			if ( pos_state.pos == pos_state.pos_limit ) {
				pos_state.pos_reached = 1;
				pos_state.limit_set = 0;
				StepperTB6600_SetEn(STOP);
			}

		}

	}

}

