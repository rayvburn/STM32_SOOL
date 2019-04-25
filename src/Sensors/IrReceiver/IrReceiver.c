/*
 * IrReceiver.c
 *
 *  Created on: 25.04.2019
 *      Author: user
 */

//#include "IrReceiver.h"
#include "include/Sensors/IrReceiver/IrReceiver.h"
//#include "include/Peripherals/TIM/Systick_Timer.h"

#include "stm32f10x_exti.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t 	IrReceiver_GetReceptionFlag	(IrReceiver *ir_ptr);
static uint8_t 	IrReceiver_GetCurrentState	(const IrReceiver *ir_ptr);
static uint32_t IrReceiver_GetLastEdgeTime	(const IrReceiver *ir_ptr);
static uint8_t 	IrReceiver_IsStateStable	(const IrReceiver *ir_ptr, const uint32_t req_gap);
static uint8_t 	IrReceiver_InterruptHandler	(IrReceiver *ir_ptr);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

IrReceiver SOOL_IrReceiver_Init(SOOL_PinConfigInt setup) {

	IrReceiver obj;

	// setup
	obj.setup = setup;

	// state init
	obj.state.last_edge_time = 0;
	obj.state.last_state_int = 1; // the sensor is active low
	obj.state.received_flag = 0;

	// interrupt-switching functions
	obj.SetExtiState = SOOL_PinConfig_ExtiSwitch;
	obj.SetNvicState = SOOL_PinConfig_NvicSwitch;

	// other functions
	obj.GetCurrentState = IrReceiver_GetCurrentState;
	obj.GetLastEdgeTime = IrReceiver_GetLastEdgeTime;
	obj.GetReceptionFlag = IrReceiver_GetReceptionFlag;
	obj.InterruptHandler = IrReceiver_InterruptHandler;
	obj.IsStateStable = IrReceiver_IsStateStable;

	return (obj);

}

// =============================================================================================

static uint8_t IrReceiver_GetReceptionFlag(IrReceiver *ir_ptr) {
	uint8_t temp = ir_ptr->state.received_flag;
	ir_ptr->state.received_flag = 0;
	return (temp);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t IrReceiver_GetCurrentState(const IrReceiver *ir_ptr) {
	return (GPIO_ReadInputDataBit(ir_ptr->setup.gpio_port, ir_ptr->setup.gpio_pin));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint32_t IrReceiver_GetLastEdgeTime(const IrReceiver *ir_ptr) {
	return (ir_ptr->state.last_edge_time);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t IrReceiver_IsStateStable(const IrReceiver *ir_ptr, const uint32_t req_gap) {
//	if ( SysTick_GetTithingsOfSec() - ir_ptr->state.last_edge_time > req_gap) {
	if ( 2 > 0 ) {
		// a given time has elapsed
		return (1);
	}
	return (0);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t IrReceiver_InterruptHandler(IrReceiver *ir_ptr) {

	if ( EXTI_GetITStatus(ir_ptr->setup.exti_line) == RESET ) {
		// interrupt request on different EXTI Line
		return (0);
	}

	// clear flag
	EXTI_ClearITPendingBit( ir_ptr->setup.exti_line );

	// process interrupt handler
//	ir_ptr->state.last_edge_time = SysTick_GetTithingsOfSec();
	ir_ptr->state.last_state_int = GPIO_ReadInputDataBit(ir_ptr->setup.gpio_port, ir_ptr->setup.gpio_pin);

	// set received flag only when input in low state (sensor is active low)
	if ( ir_ptr->state.last_state_int == 0 ) {
		ir_ptr->state.received_flag = 1;
	}

	// indicate that handler finished work
	return (1);

}

