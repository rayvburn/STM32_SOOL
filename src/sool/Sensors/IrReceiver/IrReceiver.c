/*
 * IrReceiver.c
 *
 *  Created on: 25.04.2019
 *      Author: user
 */

//#include "IrReceiver.h"
#include <sool/Peripherals/TIM/SystickTimer.h>
#include "sool/Sensors/IrReceiver/IrReceiver.h"
#include "stm32f10x_exti.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t 	IrReceiver_GetReceptionFlag	(volatile SOOL_IrReceiver *ir_ptr);
static uint8_t 	IrReceiver_GetCurrentState	(const volatile SOOL_IrReceiver *ir_ptr);
static uint32_t IrReceiver_GetLastEdgeTime	(const volatile SOOL_IrReceiver *ir_ptr);
static uint8_t 	IrReceiver_IsStateStable	(const volatile SOOL_IrReceiver *ir_ptr, const uint32_t req_gap);
static uint8_t 	IrReceiver_InterruptHandler	(volatile SOOL_IrReceiver *ir_ptr);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

volatile SOOL_IrReceiver SOOL_Sensor_IrReceiver_Init(SOOL_PinConfig_Int setup) {

	volatile SOOL_IrReceiver obj;

	// setup
//	obj._setup = setup; // infinite loop?
	/*
	obj.base.exti.line = setup.exti.line;
	obj.base.exti.setup = setup.exti.setup;
	obj.base.gpio.pin = setup.gpio.pin;
	obj.base.gpio.port = setup.gpio.port;
	obj.base.nvic.irqn = setup.nvic.irqn;
	obj.base.nvic.setup = setup.nvic.setup;
	obj.base.exti.pin_src = setup.exti.pin_src;
	obj.base.exti.port_src = setup.exti.port_src;
	*/
	obj.base = setup;

	// state init
	obj._state.last_edge_time = 0;
	obj._state.last_state_int = 1; // the sensor is active low
	obj._state.received_flag = 0;

//	// interrupt-switching functions
//	obj.SetExtiState = SOOL_Periph_GPIO_PinConfig_ExtiSwitch;
//	obj.SetNvicState = SOOL_Periph_GPIO_PinConfig_NvicSwitch;

	// other functions
	obj.GetCurrentState = IrReceiver_GetCurrentState;
	obj.GetLastEdgeTime = IrReceiver_GetLastEdgeTime;
	obj.GetReceptionFlag = IrReceiver_GetReceptionFlag;
	obj._InterruptHandler = IrReceiver_InterruptHandler;
	obj.IsStateStable = IrReceiver_IsStateStable;

	return (obj);

}

// =============================================================================================

static uint8_t IrReceiver_GetReceptionFlag(volatile SOOL_IrReceiver *ir_ptr) {
	uint8_t temp = ir_ptr->_state.received_flag;
	ir_ptr->_state.received_flag = 0;
	return (temp);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t IrReceiver_GetCurrentState(const volatile SOOL_IrReceiver *ir_ptr) {
	return (GPIO_ReadInputDataBit(ir_ptr->base._gpio.port, ir_ptr->base._gpio.pin));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint32_t IrReceiver_GetLastEdgeTime(const volatile SOOL_IrReceiver *ir_ptr) {
	return (ir_ptr->_state.last_edge_time);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t IrReceiver_IsStateStable(const volatile SOOL_IrReceiver *ir_ptr, const uint32_t req_gap) {

	if ( SOOL_Periph_TIM_SysTick_GetMillis() - ir_ptr->_state.last_edge_time > req_gap) {
		// a given time has elapsed
		return (1);
	}
	return (0);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t IrReceiver_InterruptHandler(volatile SOOL_IrReceiver *ir_ptr) {

//	if ( EXTI_GetITStatus(ir_ptr->base._exti.setup.EXTI_Line) == RESET ) {
//		// interrupt request on different EXTI Line
//		return (0);
//	}

//	// clear flag
//	EXTI_ClearITPendingBit( ir_ptr->base._exti.setup.EXTI_Line );

	// process interrupt handler
	ir_ptr->_state.last_edge_time = SOOL_Periph_TIM_SysTick_GetMillis();
	ir_ptr->_state.last_state_int = GPIO_ReadInputDataBit(ir_ptr->base._gpio.port, ir_ptr->base._gpio.pin);

	// set received flag only when input in low state (sensor is active low)
	if ( ir_ptr->_state.last_state_int == 0 ) {
		ir_ptr->_state.received_flag = 1;
	}

	// indicate that handler finished work
	return (1);

}

