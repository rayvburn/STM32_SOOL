/*
 * IrReceiver.c
 *
 *  Created on: 25.04.2019
 *      Author: user
 */

//#include "IrReceiver.h"
#include "Sensors/IrReceiver/IrReceiver.h"

#include "stm32f10x_exti.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// private function prototypes
static void IrReceiver_SetLocalPointer(IrReceiverID id);
static void IrReceiver_EnableAPBClock(GPIO_TypeDef* port);
static void IrReceiver_SetEXTILineEXTIPinSourceIRQn(uint16_t pin, uint32_t *exti_ln, uint8_t *pin_src, uint8_t *irqn);
static void IrReceiver_SetEXTIPortSource(const GPIO_TypeDef* port, uint8_t *port_src);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static IrReceiverData* irreceiver;			// to avoid memory allocation inside interrupt routines
static IrReceiverData  irreceiver_right;
static IrReceiverData  irreceiver_left;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void IrReceiver_InitialConfig() {

	/*
	 * Prevents from improper EXTI_Line values in defined-to-be structs
	 * Need to be fired each before first Button_Config()
	 */

	/*
	 * 0x00 is not correct value in terms of IS_EXTI_LINE(), see
	 * @defgroup EXTI_Exported_Constants
	 * @defgroup EXTI_Lines
	 */

	irreceiver_right.setup.exti_line 		= 0x00;
	irreceiver_left.setup.exti_line 		= 0x00;

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void IrReceiver_Config(IrReceiverID id, GPIO_TypeDef* port, uint16_t pin) {

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

	IrReceiver_SetEXTIPortSource(port, &port_src);
	IrReceiver_SetEXTILineEXTIPinSourceIRQn(pin, &exti_ln, &pin_src, &irqn);

	IrReceiver_EnableAPBClock(port);
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
	IrReceiver_SetLocalPointer(id);
	irreceiver->setup.gpio_port = port;
	irreceiver->setup.gpio_pin = pin;
	irreceiver->setup.exti_line = exti_ln;
	irreceiver->setup.port_source = port_src;
	irreceiver->setup.pin_source = pin_src;
	irreceiver->setup.irq_channel = irqn;

	// inittypedef struct fields
	irreceiver->setup.exti_setup = exti;
	irreceiver->setup.nvic_setup = nvic;

	// irreceiver push flag to be read in main loop
	irreceiver->state.signal_flag = 0;

	EXTI_Init(&exti);
	GPIO_EXTILineConfig(port_src, pin_src);
	NVIC_Init(&nvic);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void IrReceiver_SwitchInterruptNVIC(IrReceiverID id, FunctionalState state) {

	IrReceiver_SetLocalPointer(id);
	irreceiver->setup.nvic_setup.NVIC_IRQChannelCmd = state;
	NVIC_Init(&(irreceiver->setup.nvic_setup));

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void IrReceiver_SwitchInterruptEXTI(IrReceiverID id, FunctionalState state) {

	IrReceiver_SetLocalPointer(id);
	irreceiver->setup.exti_setup.EXTI_LineCmd = state;
	EXTI_Init(&(irreceiver->setup.exti_setup));

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

uint8_t IrReceiver_GetPushedFlag(IrReceiverID id) {

	IrReceiver_SetLocalPointer(id);
	uint8_t temp = irreceiver->state.signal_flag;
	irreceiver->state.signal_flag = 0;
	return temp;

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

uint8_t IrReceiver_GetState(IrReceiverID id) {

	IrReceiver_SetLocalPointer(id);
	return GPIO_ReadInputDataBit(irreceiver->setup.gpio_port, irreceiver->setup.gpio_pin);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// private function
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static void IrReceiver_SetLocalPointer(IrReceiverID id) {

	switch (id) {
	case(IR_RECEIVER_LEFT):
		irreceiver = &irreceiver_left;
		break;
	case(IR_RECEIVER_RIGHT):
		irreceiver = &irreceiver_right;
		break;
	default:
		irreceiver = 0;	// for compiler only
		break;
	}

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// private function
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static void IrReceiver_EnableAPBClock(GPIO_TypeDef* port) {

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
static void IrReceiver_SetEXTILineEXTIPinSourceIRQn(uint16_t pin, uint32_t *exti_ln,
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
static void IrReceiver_SetEXTIPortSource(const GPIO_TypeDef* port, uint8_t *port_src) {

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

	if (EXTI_GetITStatus(irreceiver_left.setup.exti_line)) {

		EXTI_ClearITPendingBit(irreceiver_left.setup.exti_line);

		if ( GPIO_ReadInputDataBit(irreceiver_right.setup.gpio_port, irreceiver_right.setup.gpio_pin) == 0 ) {
			irreceiver_right.state.signal_flag = 1;
		} else {

		}

	}

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void EXTI9_5_IRQHandler() {

	if (EXTI_GetITStatus(irreceiver_right.setup.exti_line) ) {

		// logic or in case of not set one of above values
		EXTI_ClearITPendingBit(irreceiver_left.setup.exti_line);

		if ( GPIO_ReadInputDataBit(irreceiver_right.setup.gpio_port, irreceiver_left.setup.gpio_pin) == 0 ) {
			irreceiver_left.state.signal_flag = 1;
		}

	}

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
