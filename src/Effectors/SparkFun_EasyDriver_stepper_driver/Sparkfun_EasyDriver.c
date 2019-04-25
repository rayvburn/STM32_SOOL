/*
 * Sparkfun_EasyDriver.c
 *
 *  Created on: 08.10.2018
 *      Author: user
 */

#include "Sparkfun_EasyDriver.h"
#include "USART.h" // debug
#include "stm32f10x_rcc.h"
#include "stm32f10x_tim.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static EasyDriver_Config *cfg_ptr;
volatile static EasyDriver_State  *state_ptr;
static TIM_OCInitTypeDef *oc_ptr;

static EasyDriver_Config config_lifter;
static EasyDriver_Config config_pawl;

// interrupts use below
volatile static EasyDriver_State state_lifter;
volatile static EasyDriver_State state_pawl;

static TIM_OCInitTypeDef oc_lifter;
static TIM_OCInitTypeDef oc_pawl;

static TIM_TimeBaseInitTypeDef tim_base; // common (1 timer)

static uint16_t pwm_period = 1000;
static uint16_t pwm_pulse_width = 0;

// = = = = = = =  private prototypes = = = = = = = = = = =

static void EasyDriver_SetLocalPointers(EasyDriver_ID id);
static void EasyDriver_StatePosUpdate(EasyDriver_ID id, volatile EasyDriver_State *state_, EasyDriver_Config *config_);

// = = = = = = = = = = = = = = = = = = = = = = = = = = = =

void EasyDriver_Init(	uint16_t en_pin, 	GPIO_TypeDef *en_port,
						uint16_t step_pin, 	GPIO_TypeDef *step_port,
						uint16_t dir_pin, 	GPIO_TypeDef *dir_port,
						TIM_TypeDef* tim,	IRQn_Type tim_irq,
						EasyDriver_MountType mount, EasyDriver_ID id ) {

	// set pointer to appropriate struct

	EasyDriver_SetLocalPointers(id);
	/*
	switch ( id ) {
	case (LIFTER):
		cfg_ptr = &config_lifter;
		oc_ptr = &oc_lifter;
		break;
	case(PAWL):
		cfg_ptr = &config_pawl;
		oc_ptr = &oc_pawl;
		break;
	default:
		return;
		break;
	}
	*/

	// copy values to struct
	cfg_ptr->gpio_enable_pin = en_pin;
	cfg_ptr->gpio_enable_port = en_port;
	cfg_ptr->gpio_step_pin = step_pin;
	cfg_ptr->gpio_step_port = step_port;
	cfg_ptr->gpio_dir_pin = dir_pin;
	cfg_ptr->gpio_dir_port = dir_port;
	cfg_ptr->tim_type = tim;
	cfg_ptr->timer_irqn = tim_irq;
	cfg_ptr->mount = mount;

	state_ptr->pos_limit_reached = 1; 	// to allow instant movement
	state_ptr->current_position = 1;	// has to be set SEPARATELY in SetInitialPosition()

	// start of rcc clocks (GPIO ports and timer) || adjust if port migration occurs! ||
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

	// init gpio
	GPIO_InitTypeDef gpio;
	GPIO_StructInit(&gpio);

	// dir
	gpio.GPIO_Mode = GPIO_Mode_Out_PP;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	gpio.GPIO_Pin = cfg_ptr->gpio_dir_pin;
	GPIO_Init(cfg_ptr->gpio_dir_port, &gpio);

	// enable
	gpio.GPIO_Pin = cfg_ptr->gpio_enable_pin;
	GPIO_Init(cfg_ptr->gpio_enable_port, &gpio);

	//step
	gpio.GPIO_Mode = GPIO_Mode_AF_PP;
	gpio.GPIO_Pin = cfg_ptr->gpio_step_pin;
	GPIO_Init(cfg_ptr->gpio_step_port, &gpio);


	// idle state
	EasyDriver_SetEn(STOP, TIMER_TOGGLE, id);
	EasyDriver_SetDir(UP, id);


	// init timer and its interrupts
	tim_base.TIM_CounterMode = TIM_CounterMode_Up;
	tim_base.TIM_Prescaler = 64 - 1; // 72 - 1;
	tim_base.TIM_Period = pwm_period - 1;
	TIM_TimeBaseInit(cfg_ptr->tim_type, &tim_base);
	TIM_ITConfig(cfg_ptr->tim_type, TIM_IT_Update | TIM_IT_CC1 | TIM_IT_CC2, ENABLE); // CCx NEEDS TO BE ADJUSTED ACCORDING TO PULSE PIN PLACEMENT
	TIM_Cmd(cfg_ptr->tim_type, ENABLE);

	// init pwm channel
	EasyDriver_UpdatePulseWidth();
	TIM_OCStructInit(oc_ptr);
	oc_ptr->TIM_OCMode = TIM_OCMode_PWM2;
	oc_ptr->TIM_Pulse  = pwm_pulse_width;
	oc_ptr->TIM_OutputState = TIM_OutputState_Enable;
	oc_ptr->TIM_OCPolarity  = TIM_OCPolarity_High;

	switch ( id ) {

	// TIM_OCxInit(..., ...) MUST be adjusted according to wiring (timer channel OC)!
	case (LIFTER):
		TIM_OC1Init(cfg_ptr->tim_type, oc_ptr);	// OC1 because output pin is PA6 (T3C1)
		break;
	case(PAWL):
		TIM_OC2Init(cfg_ptr->tim_type, oc_ptr);	// OC2 because output pin is PA7 (T3C2)
		break;
	}

	// init timer's nvic
	NVIC_InitTypeDef nvic;
	nvic.NVIC_IRQChannel = TIM3_IRQn; // cfg_ptr->timer_irqn;
	nvic.NVIC_IRQChannelPreemptionPriority = 0;
	nvic.NVIC_IRQChannelSubPriority = 0;
	nvic.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvic);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void EasyDriver_SetMotorParameters(uint16_t steps_per_rev, uint32_t mm_move_per_rev, uint8_t microsteps, EasyDriver_ID id) {

	EasyDriver_SetLocalPointers(id);
	/*
	switch ( id ) {
	case (LIFTER):
		state_ptr = &state_lifter;
		break;
	case(PAWL):
		state_ptr = &state_pawl;
		break;
	default:
		return;
		break;
	}
	*/

	state_ptr->steps_per_rev = steps_per_rev;
	state_ptr->mm_move_per_rev = mm_move_per_rev;
	state_ptr->microstepping = microsteps;

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void EasyDriver_SetInitialPosition(uint8_t limit_switch_state, EasyDriver_ID id) {

	/* limit switch state description:
	 * 	- (1 -> not pressed, motor is not in the limit position now)
	 *	- (0 ->     pressed, motor is     in the limit position now)
	 */

	EasyDriver_SetLocalPointers(id);

	switch (limit_switch_state) {
	case(1):
		state_ptr->current_position = POSITION_FORWARD;
		break;
	case(0):
		state_ptr->current_position = POSITION_LIMITED;
		break;
	}

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

uint8_t EasyDriver_GetMotorPosition(EasyDriver_ID id) {

	EasyDriver_SetLocalPointers(id);
	return (uint8_t)state_ptr->current_position;

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void EasyDriver_MoveLinear(uint32_t len_mm, uint16_t speed, EasyDriver_LinearDirection dir,
						   uint8_t limit_switch_state, EasyDriver_ID id) {

	EasyDriver_SetLocalPointers(id);
	/*
	switch ( id ) {
	case (LIFTER):
		state_ptr = &state_lifter;
		break;
	case(PAWL):
		state_ptr = &state_pawl;
		break;
	default:
		return;
		break;
	}
	*/


	/* limit switch state description:
	 * 	- (1 -> not pressed, motor is NOT in the limit position now)
	 *	- (0 ->     pressed, motor is     in the limit position now)
	 */

	/* BELOW IS QUITE SPECIFIC FOR THE EXPERIMENT THIS LIBRARY WAS MADE FOR
	 * there are some movement constraints applied */

	// check if: motor is in limit position, limit switch is pushed and it tries
	// to move in the switches direction
	uint8_t forbidden_movement_flag = 0;

	switch (cfg_ptr->mount) {
	case(MOUNT_BOTTOM_UP):

			if ( limit_switch_state == 0 && dir == UP ) {
				forbidden_movement_flag = 1;
			} else if ( state_ptr->current_position == POSITION_FORWARD && dir == DOWN) {
				forbidden_movement_flag = 1;
			}
			break;

	case(MOUNT_BOTTOM_DOWN):

			if ( limit_switch_state == 0 && dir == DOWN ) {
				forbidden_movement_flag = 1;
			} else if ( state_ptr->current_position == POSITION_FORWARD && dir == UP) {
				forbidden_movement_flag = 1;
			}
			break;

	}

	if (forbidden_movement_flag == 1) {
		//USART_SendString(DEBUGGING, "==========PERMITTED MOVEMENT========\n");
		return;
	}

	/* END OF THE EXPERIMENT SPECIFIC SECTION */

	EasyDriver_SetSpeed(speed, id);

	// float used here for better accuracy
	float steps_to_take_f = (float)(state_ptr->steps_per_rev) * (float)(state_ptr->microstepping) * (float)(len_mm / state_ptr->mm_move_per_rev);
	uint32_t steps_to_take = (uint32_t)(steps_to_take_f);
	EasyDriver_MakeSteps(steps_to_take, dir, id);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void EasyDriver_SetSpeed (uint16_t speed, EasyDriver_ID id ) {

	EasyDriver_SetLocalPointers(id);
	/*
	switch ( id ) {
	case (LIFTER):
		cfg_ptr = &config_lifter;
		oc_ptr = &oc_lifter;
		break;
	case(PAWL):
		cfg_ptr = &config_pawl;
		oc_ptr = &oc_pawl;
		break;
	default:
		return;
		break;
	}
	*/

	if ( !(speed == 0) ) {

		TIM_Cmd(cfg_ptr->tim_type, DISABLE);				// stop clock

		pwm_period = speed;									// load new period value
		tim_base.TIM_Period = pwm_period - 1;				// set new pwm frequency
		TIM_TimeBaseInit(cfg_ptr->tim_type, &tim_base);		// load data

		EasyDriver_UpdatePulseWidth();						// update duty cycle to generate square wave
		oc_ptr->TIM_Pulse = pwm_pulse_width;				// load duty cycle value to struct

		switch ( id ) {
		// TIM_OCxInit(..., ...) MUST be adjusted according to wiring (timer channel OC)!
		case (LIFTER):
			TIM_OC1Init(cfg_ptr->tim_type, oc_ptr);
			break;
		case(PAWL):
			TIM_OC2Init(cfg_ptr->tim_type, oc_ptr);
			break;
		}
		TIM_Cmd(cfg_ptr->tim_type, ENABLE);							// start clock

	} else {
		EasyDriver_SetEn(STOP, TIMER_LEAVE, id);
	}

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void EasyDriver_UpdatePulseWidth() {
	pwm_pulse_width = pwm_period/2;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void EasyDriver_SetDir	(EasyDriver_LinearDirection dir, EasyDriver_ID id ) {

	EasyDriver_SetLocalPointers(id);
	/*
	switch ( id ) {
	case (LIFTER):
		cfg_ptr = &config_lifter;
		state_ptr = &state_lifter;
		break;
	case(PAWL):
		cfg_ptr = &config_pawl;
		state_ptr = &state_pawl;
		break;
	default:
		return;
		break;
	}
	*/

	switch (dir) {

	case(UP):

		if ( cfg_ptr->mount == MOUNT_BOTTOM_UP ) {
			GPIO_SetBits(cfg_ptr->gpio_dir_port, cfg_ptr->gpio_dir_pin);
		} else if ( cfg_ptr->mount == MOUNT_BOTTOM_DOWN ) {
			GPIO_ResetBits(cfg_ptr->gpio_dir_port, cfg_ptr->gpio_dir_pin);
		}
		state_ptr->dir = UP;
		break;

	case(DOWN):

		if ( cfg_ptr->mount == MOUNT_BOTTOM_UP ) {
			GPIO_ResetBits(cfg_ptr->gpio_dir_port, cfg_ptr->gpio_dir_pin);
		} else if ( cfg_ptr->mount == MOUNT_BOTTOM_DOWN ) {
			GPIO_SetBits(cfg_ptr->gpio_dir_port, cfg_ptr->gpio_dir_pin);
		}
		state_ptr->dir = DOWN;
		break;

	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void EasyDriver_SetEn (StepperDriverEn state, EasyDriver_TimerState timer_state, EasyDriver_ID id) {

	/*
	 * timer_state must be set according to assumption if another motor sharing the same timer
	 * is possibly running; if there are few motors using the same timer then timer_state should
	 * be set to TIMER_LEAVE; otherwise, when no motors are expected to run in this moment -
	 * set TIMER_TOGGLE - it will turn on OR turn off and reset the timer's counter
	 */

	EasyDriver_SetLocalPointers(id);
	/*
	switch ( id ) {
	case (LIFTER):
		cfg_ptr = &config_lifter;
		state_ptr = &state_lifter;
		break;
	case(PAWL):
		cfg_ptr = &config_pawl;
		state_ptr = &state_pawl;
		break;
	default:
		return;
		break;
	}
	*/

	switch (state) {

	case(ROTATE):
		( timer_state == TIMER_TOGGLE ) ? (TIM_Cmd(cfg_ptr->tim_type, ENABLE)) : (0); // start the timer or leave it
		GPIO_ResetBits(cfg_ptr->gpio_enable_port, cfg_ptr->gpio_enable_pin);
		state_ptr->currently_moving = 1;
		break;

	case(STOP):
		if ( timer_state == TIMER_TOGGLE ) {
			TIM_Cmd(cfg_ptr->tim_type, DISABLE);	// start the timer
			TIM_SetCounter(cfg_ptr->tim_type, 0); 	// set timer's counter to 0
		}
		GPIO_SetBits(cfg_ptr->gpio_enable_port, cfg_ptr->gpio_enable_pin);
		state_ptr->currently_moving = 0;
		break;

	}

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void EasyDriver_MakeSteps(uint32_t steps, EasyDriver_LinearDirection dir, EasyDriver_ID id ) {

	EasyDriver_SetLocalPointers(id);
	/*
	switch ( id ) {
		case (LIFTER):
			cfg_ptr = &config_lifter;
			state_ptr = &state_lifter;
			break;
		case(PAWL):
			cfg_ptr = &config_pawl;
			state_ptr = &state_pawl;
			break;
		default:
			break;
	}
	*/

	// stop the motor and timer
	EasyDriver_SetEn(STOP, TIMER_TOGGLE, id);
	EasyDriver_SetDir(dir, id);	// not checking what is set now

	if ( dir == UP ) {

		state_ptr->pos_limit = state_ptr->pos + (int64_t)(steps);

	} else if ( dir == DOWN ) {

		state_ptr->pos_limit = state_ptr->pos - (int64_t)(steps);

	}

	state_ptr->pos_limit_set = 1;
	state_ptr->pos_limit_reached = 0;

	// start motor
	EasyDriver_SetEn(ROTATE, TIMER_TOGGLE, id);

	// start timer
	TIM_Cmd(cfg_ptr->tim_type, ENABLE);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

uint8_t EasyDriver_IsLinearMovementFinished(EasyDriver_ID id) {

	EasyDriver_SetLocalPointers(id);
	return state_ptr->pos_limit_reached;

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void EasyDriver_WaitForEvent(EasyDriver_ID id, uint8_t event_flag) {

	if ( event_flag ) {

		EasyDriver_SetLocalPointers(id);
		TIM_Cmd(cfg_ptr->tim_type, DISABLE);

		// stop only when going in the POSITION LIMITED direction - limit switch only on 1 side
		if ( state_ptr->current_position != POSITION_LIMITED ) {

			EasyDriver_SetEn(STOP, TIMER_LEAVE, id);

			//USART_SendString(DEBUGGING, "WAIT FOR EVENT() --- FORWARD state STOP !!!!!!!!!!!!!!!!!");
			state_ptr->pos_limit_set = 0;
			state_ptr->pos_limit_reached = 1;
			state_ptr->current_position = POSITION_LIMITED;

		//} else {

		//	USART_SendString(DEBUGGING, "WAIT FOR EVENT() --- POS LIMITED state");

		}

		TIM_Cmd(cfg_ptr->tim_type, ENABLE);

	}

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// ~private
int64_t EasyDriver_GetPosition(EasyDriver_ID id) {

	switch ( id ) {
		case (LIFTER):
			state_ptr = &state_lifter;
			break;
		case(PAWL):
			state_ptr = &state_pawl;
			break;
		default:
			return 0;
			break;
	}
	return (state_ptr->pos);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void TIM3_IRQHandler() {


	if (TIM_GetITStatus(TIM3, TIM_IT_Update) == SET) {

		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
		/*
		 * main (Update) frequency debugging
		 *
		if ( GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_2) == SET ) {
			GPIO_ResetBits(GPIOC, GPIO_Pin_2);
		} else {
			GPIO_SetBits(GPIOC, GPIO_Pin_2);
		}
		*/

	} else if (TIM_GetITStatus(TIM3, TIM_IT_CC1) == SET) {

		TIM_ClearITPendingBit(TIM3, TIM_IT_CC1);
		EasyDriver_StatePosUpdate(LIFTER, &state_lifter, &config_lifter);
		/*
		 * CC1 frequency debugging
		 *
		if ( GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_3) == SET ) {
			GPIO_ResetBits(GPIOC, GPIO_Pin_3);
		} else {
			GPIO_SetBits(GPIOC, GPIO_Pin_3);
		}
		*/

	} else if (TIM_GetITStatus(TIM3, TIM_IT_CC2) == SET) {

		TIM_ClearITPendingBit(TIM3, TIM_IT_CC2);
		EasyDriver_StatePosUpdate(PAWL, &state_pawl, &config_pawl);

	}

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// ~~private
static void EasyDriver_StatePosUpdate(EasyDriver_ID id, volatile EasyDriver_State *state_, EasyDriver_Config *config_) {

	if ( state_->currently_moving ) {

		if ( state_->dir == UP ) {
			state_->pos++;
		} else {
			state_->pos--;
		}

		if ( state_->pos_limit_set ) { 	// check if position limit exists

			/*
			 * FOR BIG, not INCREMENTAL, changes in pos
			 *
			if ( state_lifter.dir == UP ) {

				if ( state_lifter.pos >= state_lifter.pos_limit ) {
					TIM_Cmd(config_lifter.tim_type, DISABLE);
					EasyDriver_SetEn(STOP, LIFTER);
					state_lifter.pos_limit_set = 0;	// reset pos limit
					state_lifter.pos_limit_reached = 1;
				}

			} else {

				if ( state_lifter.pos <= state_lifter.pos_limit ) {
					TIM_Cmd(config_lifter.tim_type, DISABLE);
					EasyDriver_SetEn(STOP, LIFTER);
					state_lifter.pos_limit_set = 0;	// reset pos limit
					state_lifter.pos_limit_reached = 1;
				}

			}
			*/

			// check if pos limit was reached
			if ( state_->pos == state_->pos_limit ) {
				EasyDriver_SetEn(STOP, TIMER_LEAVE, id);
				state_->pos_limit_set = 0;				// reset pos limit
				state_->pos_limit_reached = 1;
				state_ptr->current_position = POSITION_FORWARD; // movement will finish only when in forward direction (in opposite there is a bigger value of distance pushed to function than it is feasible)
				USART_SendIntValue(DEBUGGING, id);
				USART_SendString(DEBUGGING, "  REACHED\n");
			}

		}

	}

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// ~private
static void EasyDriver_SetLocalPointers(EasyDriver_ID id) {

	switch ( id ) {
	case (LIFTER):
		cfg_ptr = &config_lifter;
		state_ptr = &state_lifter;
		oc_ptr = &oc_lifter;
		break;
	case(PAWL):
		cfg_ptr = &config_pawl;
		state_ptr = &state_pawl;
		oc_ptr = &oc_pawl;
		break;
	default:
		return;
		break;
	}

}
