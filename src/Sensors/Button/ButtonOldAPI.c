/*
 * Button.c
 *
 *  Created on: 20.10.2018
 *      Author: user
 */

//#include "Button.h"
#include <Sensors/Button/ButtonOldAPI.h>
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// private function prototypes
static void Button_SetLocalPointer(ButtonID id);
static void Button_EnableAPBClock(GPIO_TypeDef* port);
static void Button_SetEXTILineEXTIPinSourceIRQn(uint16_t pin, uint32_t *exti_ln, uint8_t *pin_src, uint8_t *irqn);
static void Button_SetEXTIPortSource(const GPIO_TypeDef* port, uint8_t *port_src);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static ButtonData* button;			// to avoid memory allocation inside interrupt routines
static ButtonData  button_start;
static ButtonData  button_arm_lift;
static ButtonData  button_arm_lower;
static ButtonData  button_arm_limit_switch;
static ButtonData  button_pawl_pusher;
static ButtonData  button_pawl_collapser;
static ButtonData  button_pawl_limit_switch;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void Button_InitialConfig() {

	/*
	 * Prevents from improper EXTI_Line values in defined-to-be structs
	 * Need to be fired each before first Button_Config()
	 */

	/*
	 * 0x00 is not correct value in terms of IS_EXTI_LINE(), see
	 * @defgroup EXTI_Exported_Constants
	 * @defgroup EXTI_Lines
	 */

	button_start.setup.exti_line 			= 0x00;
	button_arm_lift.setup.exti_line 		= 0x00;
	button_arm_lower.setup.exti_line 		= 0x00;
	button_arm_limit_switch.setup.exti_line = 0x00;
	button_pawl_pusher.setup.exti_line 		= 0x00;
	button_pawl_collapser.setup.exti_line	= 0x00;

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void Button_Config(ButtonID id, GPIO_TypeDef* port, uint16_t pin) {

	/*
	 * 	1) 	Be careful when 2 buttons are connected to the same exti line,
	 * 		config functions invoking order is important!
	 */

	/*
	 * 	2) 	Manually adjust appropriate IRQHandler or a program will be stuck
	 * 		in not cleared INTERRUPT flag!
	 */

	uint32_t exti_ln;
	uint8_t pin_src, port_src, irqn;

	Button_SetEXTIPortSource(port, &port_src);
	Button_SetEXTILineEXTIPinSourceIRQn(pin, &exti_ln, &pin_src, &irqn);

	Button_EnableAPBClock(port);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

	GPIO_InitTypeDef gpio;
	GPIO_StructInit(&gpio);
	gpio.GPIO_Pin = pin;
	gpio.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(port, &gpio);

	EXTI_InitTypeDef exti;
	EXTI_StructInit(&exti);
	exti.EXTI_Line = exti_ln;
	exti.EXTI_Mode = EXTI_Mode_Interrupt;
	exti.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	exti.EXTI_LineCmd = ENABLE;
	// exti init at the end of the function
	// exti line config at the end of the function

	NVIC_InitTypeDef nvic;
	nvic.NVIC_IRQChannel = irqn;
	nvic.NVIC_IRQChannelPreemptionPriority = 0x00;
	nvic.NVIC_IRQChannelSubPriority = 0x00;
	nvic.NVIC_IRQChannelCmd = ENABLE;
	// nvic init at the end of the function

	// gpio-specific struct fields
	Button_SetLocalPointer(id);
	button->setup.gpio_port = port;
	button->setup.gpio_pin = pin;
	button->setup.exti_line = exti_ln;
	button->setup.port_source = port_src;
	button->setup.pin_source = pin_src;
	button->setup.irq_channel = irqn;

	// inittypedef struct fields
	button->setup.exti_setup = exti;
	button->setup.nvic_setup = nvic;

	// button push flag to be read in main loop
	button->state.pushed_flag = 0;

	EXTI_Init(&exti);
	GPIO_EXTILineConfig(port_src, pin_src);
	NVIC_Init(&nvic);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void Button_SwitchInterruptNVIC(ButtonID id, FunctionalState state) {

	Button_SetLocalPointer(id);
	button->setup.nvic_setup.NVIC_IRQChannelCmd = state;
	NVIC_Init(&(button->setup.nvic_setup));

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void Button_SwitchInterruptEXTI(ButtonID id, FunctionalState state) {

	Button_SetLocalPointer(id);
	button->setup.exti_setup.EXTI_LineCmd = state;
	EXTI_Init(&(button->setup.exti_setup));

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

uint8_t Button_GetPushedFlag(ButtonID id) {

	Button_SetLocalPointer(id);
	uint8_t temp = button->state.pushed_flag;
	button->state.pushed_flag = 0;
	return temp;

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

uint8_t Button_GetState(ButtonID id) {

	Button_SetLocalPointer(id);
	return GPIO_ReadInputDataBit(button->setup.gpio_port, button->setup.gpio_pin);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// private function
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static void Button_SetLocalPointer(ButtonID id) {

	switch (id) {
	case(STARTER):
		button = &button_start;
		break;
	case(ARM_LIFTER):
		button = &button_arm_lift;
		break;
	case(ARM_LOWERER):
		button = &button_arm_lower;
		break;
	case(ARM_LIMIT_SWITCH):
		button = &button_arm_limit_switch;
		break;
	case(PAWL_PUSHER):
		button = &button_pawl_pusher;
		break;
	case(PAWL_COLLAPSER):
		button = &button_pawl_collapser;
		break;
	case(PAWL_LIMIT_SWITCH):
		button = &button_pawl_limit_switch;
		break;
	default:
		button = 0;	// for compiler only
		break;
	}

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// private function
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static void Button_EnableAPBClock(GPIO_TypeDef* port) {

	if ( port == GPIOA ) {
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	} else if ( port == GPIOB ) {
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	} else if ( port == GPIOC ) {
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	} else if ( port == GPIOD ) {
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
	}

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// private function
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static void Button_SetEXTILineEXTIPinSourceIRQn(uint16_t pin, uint32_t *exti_ln,
										 	  	uint8_t *pin_src, uint8_t *irqn) {

	switch(pin) {

	case(GPIO_Pin_0):
		*exti_ln = EXTI_Line0;
		*pin_src = GPIO_PinSource0;
		*irqn = EXTI0_IRQn;
		break;
	case(GPIO_Pin_1):
		*exti_ln = EXTI_Line1;
		*pin_src = GPIO_PinSource1;
		*irqn = EXTI1_IRQn;
		break;
	case(GPIO_Pin_2):
		*exti_ln = EXTI_Line2;
		*pin_src = GPIO_PinSource2;
		*irqn = EXTI2_IRQn;
		break;
	case(GPIO_Pin_3):
		*exti_ln = EXTI_Line3;
		*pin_src = GPIO_PinSource3;
		*irqn = EXTI3_IRQn;
		break;
	case(GPIO_Pin_4):
		*exti_ln = EXTI_Line4;
		*pin_src = GPIO_PinSource4;
		*irqn = EXTI4_IRQn;
		break;
	case(GPIO_Pin_5):
		*exti_ln = EXTI_Line5;
		*pin_src = GPIO_PinSource5;
		*irqn = EXTI9_5_IRQn;
		break;
	case(GPIO_Pin_6):
		*exti_ln = EXTI_Line6;
		*pin_src = GPIO_PinSource6;
		*irqn = EXTI9_5_IRQn;
		break;
	case(GPIO_Pin_7):
		*exti_ln = EXTI_Line7;
		*pin_src = GPIO_PinSource7;
		*irqn = EXTI9_5_IRQn;
		break;
	case(GPIO_Pin_8):
		*exti_ln = EXTI_Line8;
		*pin_src = GPIO_PinSource8;
		*irqn = EXTI9_5_IRQn;
		break;
	case(GPIO_Pin_9):
		*exti_ln = EXTI_Line9;
		*pin_src = GPIO_PinSource9;
		*irqn = EXTI9_5_IRQn;
		break;
	case(GPIO_Pin_10):
		*exti_ln = EXTI_Line10;
		*pin_src = GPIO_PinSource10;
		*irqn = EXTI15_10_IRQn;
		break;
	case(GPIO_Pin_11):
		*exti_ln = EXTI_Line11;
		*pin_src = GPIO_PinSource11;
		*irqn = EXTI15_10_IRQn;
		break;
	case(GPIO_Pin_12):
		*exti_ln = EXTI_Line12;
		*pin_src = GPIO_PinSource12;
		*irqn = EXTI15_10_IRQn;
		break;
	case(GPIO_Pin_13):
		*exti_ln = EXTI_Line13;
		*pin_src = GPIO_PinSource13;
		*irqn = EXTI15_10_IRQn;
		break;
	case(GPIO_Pin_14):
		*exti_ln = EXTI_Line14;
		*pin_src = GPIO_PinSource14;
		*irqn = EXTI15_10_IRQn;
		break;
	case(GPIO_Pin_15):
		*exti_ln = EXTI_Line15;
		*pin_src = GPIO_PinSource15;
		*irqn = EXTI15_10_IRQn;
		break;

	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// private function
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static void Button_SetEXTIPortSource(const GPIO_TypeDef* port, uint8_t *port_src) {

	if ( port == GPIOA ) {
		*port_src = GPIO_PortSourceGPIOA;
	} else if ( port == GPIOB ) {
		*port_src = GPIO_PortSourceGPIOB;
	} else if ( port == GPIOC ) {
		*port_src = GPIO_PortSourceGPIOC;
	} else if ( port == GPIOD ) {
		*port_src = GPIO_PortSourceGPIOD;
	}

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/*
 * THIS IS THE IRQHANDLER TEMPLATE, IT NEEDS TO BE ADJUSTED ACCORDING TO CURRENT BUTTONS SETUP
 *
void EXTIx_IRQHandler() {
	if (EXTI_GetITStatus(EXTI_LineY)) {
		EXTI_ClearITPendingBit(EXTI_LineY);
		if ( GPIO_ReadInputDataBit(GPIOz, GPIO_Pin_Y) == 0 ) {
			// falling edge routine
		} else {
			// rising edge routine
		}
	}
}
 *
 */

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void EXTI4_IRQHandler() {

	if (EXTI_GetITStatus(button_start.setup.exti_line)) {

		EXTI_ClearITPendingBit(button_start.setup.exti_line);

		if ( GPIO_ReadInputDataBit(button_start.setup.gpio_port, button_start.setup.gpio_pin) == 0 ) {
			button_start.state.pushed_flag = 1;
		} else {

		}

	}

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void EXTI9_5_IRQHandler() {

	if (EXTI_GetITStatus(button_arm_lift.setup.exti_line) ) {

		// logic or in case of not set one of above values
		EXTI_ClearITPendingBit(button_arm_lift.setup.exti_line);

		if ( GPIO_ReadInputDataBit(button_arm_lift.setup.gpio_port, button_arm_lift.setup.gpio_pin) == 0 ) {
			button_arm_lift.state.pushed_flag = 1;
		}

	} else if ( EXTI_GetITStatus(button_arm_lower.setup.exti_line) ) {

		EXTI_ClearITPendingBit(button_arm_lower.setup.exti_line);
		if ( GPIO_ReadInputDataBit(button_arm_lower.setup.gpio_port, button_arm_lower.setup.gpio_pin) == 0 ) {
			button_arm_lower.state.pushed_flag = 1;
		}

	} else if ( EXTI_GetITStatus(button_arm_limit_switch.setup.exti_line) ) {

		EXTI_ClearITPendingBit(button_arm_limit_switch.setup.exti_line);
		if ( GPIO_ReadInputDataBit(button_arm_limit_switch.setup.gpio_port, button_arm_limit_switch.setup.gpio_pin) == 0 ) {
			button_arm_limit_switch.state.pushed_flag = 1;
		}

	} else if ( EXTI_GetITStatus(button_pawl_pusher.setup.exti_line) ) {

		EXTI_ClearITPendingBit(button_pawl_pusher.setup.exti_line);
		if ( GPIO_ReadInputDataBit(button_pawl_pusher.setup.gpio_port, button_pawl_pusher.setup.gpio_pin) == 0 ) {
			button_pawl_pusher.state.pushed_flag = 1;
		}

	}

}

// - - - - - - - - - - - - - -

void EXTI15_10_IRQHandler() {

	if ( EXTI_GetITStatus(button_pawl_limit_switch.setup.exti_line) ) {

		EXTI_ClearITPendingBit(button_pawl_limit_switch.setup.exti_line);
		if ( GPIO_ReadInputDataBit(button_pawl_limit_switch.setup.gpio_port, button_pawl_limit_switch.setup.gpio_pin) == 0 ) {
			button_pawl_limit_switch.state.pushed_flag = 1;
		}

	} else if ( EXTI_GetITStatus(button_pawl_collapser.setup.exti_line) ) {

		EXTI_ClearITPendingBit(button_pawl_collapser.setup.exti_line);
		if ( GPIO_ReadInputDataBit(button_pawl_collapser.setup.gpio_port, button_pawl_collapser.setup.gpio_pin) == 0 ) {
			button_pawl_collapser.state.pushed_flag = 1;
		}

	}

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


/*
static void Button_HandleInterruptFallingEdge(ButtonData* button_) {



}
*/
