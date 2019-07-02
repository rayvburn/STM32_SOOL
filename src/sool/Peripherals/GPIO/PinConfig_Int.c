/*
 * PinConfig_Int.c
 *
 *  Created on: 15.05.2019
 *      Author: user
 */

#include <sool/Peripherals/GPIO/PinConfig_Int.h>
#include "sool/Peripherals/NVIC/NVIC.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void PinConfig_EnableEXTI(volatile SOOL_PinConfig_Int *pin_cfg_ptr);
static void PinConfig_DisableEXTI(volatile SOOL_PinConfig_Int *pin_cfg_ptr);

static void PinConfig_EnableNVIC(volatile SOOL_PinConfig_Int *pin_cfg_ptr);
static void PinConfig_DisableNVIC(volatile SOOL_PinConfig_Int *pin_cfg_ptr);

static uint8_t PinConfig_InterruptHandler(volatile SOOL_PinConfig_Int *pin_cfg_ptr);

// helper
static void PinConfig_SetEXTILineEXTIPinSourceIRQn(uint16_t pin, uint32_t *exti_ln, uint8_t *pin_src, uint8_t *irqn);
static void PinConfig_SetEXTIPortSource(const GPIO_TypeDef* port, uint8_t *port_src);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/**
 * Enables EXTI by default
 * @param gpio_port
 * @param gpio_pin
 * @param gpio_mode
 * @param exti_trigger
 * @return
 */
SOOL_PinConfig_Int SOOL_Periph_GPIO_PinConfig_Initialize_Int(GPIO_TypeDef* gpio_port,
			uint16_t gpio_pin, GPIOMode_TypeDef gpio_mode, EXTITrigger_TypeDef exti_trigger) {

	/* object to be filled with given values
	 * and peripherals on which it depends
	 * will be started */
	SOOL_PinConfig_Int config;

	/*
	 * Prevents from improper EXTI_Line values in defined-to-be structs
	 * Need to be fired each before first Button_Config()
	 */

	/*
	 * 0x00 is not correct value in terms of IS_EXTI_LINE(), see
	 * @defgroup EXTI_Exported_Constants
	 * @defgroup EXTI_Lines
	 */
//	config._exti.line = 0x00;

	uint32_t exti_ln;
	uint8_t pin_src, port_src, irqn;

	/* Select proper values */
	PinConfig_SetEXTIPortSource(gpio_port, &port_src);
	PinConfig_SetEXTILineEXTIPinSourceIRQn(gpio_pin, &exti_ln, &pin_src, &irqn);

	/* Enable port clock */
	SOOL_Periph_GPIO_PinConfig_EnableAPBClock(gpio_port);

	/* Enable alternative function clock (EXTI) */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

	/* Configure GPIO */
	GPIO_InitTypeDef gpio;
	GPIO_StructInit(&gpio);
	gpio.GPIO_Pin = gpio_pin;
	gpio.GPIO_Mode = gpio_mode;
	GPIO_Init(gpio_port, &gpio);

	/* EXTI */
	EXTI_InitTypeDef exti;
	EXTI_StructInit(&exti);
	exti.EXTI_Line = exti_ln;
	exti.EXTI_Mode = EXTI_Mode_Interrupt;
	exti.EXTI_Trigger = exti_trigger;
	exti.EXTI_LineCmd = ENABLE;
	// exti init at the end of the function
	// exti line config at the end of the function

	NVIC_InitTypeDef nvic;
	nvic.NVIC_IRQChannel = irqn;
	nvic.NVIC_IRQChannelPreemptionPriority = 0x00;
	nvic.NVIC_IRQChannelSubPriority = 0x00;
	nvic.NVIC_IRQChannelCmd = ENABLE;
	// nvic init at the end of the function

	/* Copy data to internal structures */
	config._gpio.port = gpio_port;
	config._gpio.pin = gpio_pin;
//	config._exti.line = exti_ln;
//	config._exti.port_src = port_src;
//	config._exti.pin_src = pin_src;
	config._nvic.irqn = irqn;

	/* EXTIInitTypedef structure copy - more convenient utilization via EXTI_Init */
	config._exti.setup = exti;

	/* Copy member functions */
	config.DisableEXTI = PinConfig_DisableEXTI;
	config.DisableNVIC = PinConfig_DisableNVIC;
	config.EnableEXTI = PinConfig_EnableEXTI;
	config.EnableNVIC = PinConfig_EnableNVIC;
	config._InterruptHandler = PinConfig_InterruptHandler;

	/* Configure external interrupts on a given pin */
	EXTI_Init(&exti);
	GPIO_EXTILineConfig(port_src, pin_src);
	NVIC_Init(&nvic);

	return (config);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void PinConfig_EnableEXTI(volatile SOOL_PinConfig_Int *pin_cfg_ptr) {
	pin_cfg_ptr->_exti.setup.EXTI_LineCmd = ENABLE;
	EXTI_Init(&(pin_cfg_ptr->_exti.setup));
}
static void PinConfig_DisableEXTI(volatile SOOL_PinConfig_Int *pin_cfg_ptr) {
	pin_cfg_ptr->_exti.setup.EXTI_LineCmd = DISABLE;
	EXTI_Init(&(pin_cfg_ptr->_exti.setup));
}

static void PinConfig_EnableNVIC(volatile SOOL_PinConfig_Int *pin_cfg_ptr) {
	SOOL_Periph_NVIC_Enable(pin_cfg_ptr->_nvic.irqn);
}
static void PinConfig_DisableNVIC(volatile SOOL_PinConfig_Int *pin_cfg_ptr) {
	SOOL_Periph_NVIC_Disable(pin_cfg_ptr->_nvic.irqn);
}

static uint8_t PinConfig_InterruptHandler(volatile SOOL_PinConfig_Int *pin_cfg_ptr) {

	/* Check if the line which generated interrupt match the current one */
	if ( EXTI_GetITStatus(pin_cfg_ptr->_exti.setup.EXTI_Line) == RESET ) {
		// interrupt request on different EXTI Line
		return (0);
	}

	/* Clear interrupt flag */
	EXTI_ClearITPendingBit(pin_cfg_ptr->_exti.setup.EXTI_Line);

	/* Indicate that ISR has been triggered */
	return (1);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// private function
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static void PinConfig_SetEXTILineEXTIPinSourceIRQn(uint16_t pin, uint32_t *exti_ln,
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
static void PinConfig_SetEXTIPortSource(const GPIO_TypeDef* port, uint8_t *port_src) {

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
