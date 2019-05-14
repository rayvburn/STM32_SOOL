/*
 * IrReceiver.h
 *
 *  Created on: 25.04.2019
 *      Author: user
 */

#ifndef SENSORS_IRRECEIVER_H_
#define SENSORS_IRRECEIVER_H_

#include <sool/Peripherals/GPIO/PinConfig_Int.h>
#include "stm32f10x.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/// \brief TSOP3123/1736 IR receiver controller
/// detects simple 1 bit signal

/// \brief status `register` which changes as interrupt routing fires up
struct _SOOL_IrReceiverState {
	uint8_t 	received_flag;	// received signal (active low)
	uint8_t		last_state_int; // last pin state during interrupt routine processing
	uint32_t 	last_edge_time;	// non-defined if it will be milliseconds or 0.1 seconds
};

// - - - - - - - - - - - - - - - - -

// forward declare a structure and typedef it to further make use of it inside a structure
struct _SOOL_IrReceiverStruct;
typedef struct _SOOL_IrReceiverStruct SOOL_IrReceiver;

// IR Receiver is interrupt-driven - use previously defined structure for this task
struct _SOOL_IrReceiverStruct {

	SOOL_PinConfig_Int 				_setup;
	struct _SOOL_IrReceiverState 	_state;

	uint8_t 	(*GetReceptionFlag)(volatile SOOL_IrReceiver*); // interrupt-driven
	uint8_t 	(*GetCurrentState)(const volatile SOOL_IrReceiver*);
	uint32_t 	(*GetLastEdgeTime)(const volatile SOOL_IrReceiver*);
	uint8_t 	(*IsStateStable)(const volatile SOOL_IrReceiver*, const uint32_t); /// \param[in] object pointer, time gap required; current time requested from SysTick
	void 		(*SetNvicState)(SOOL_PinConfig_Int*, const FunctionalState);
	void 		(*SetExtiState)(SOOL_PinConfig_Int*, const FunctionalState);
	uint8_t 	(*_InterruptHandler)(volatile SOOL_IrReceiver*); // routine fired in a proper ISR (firstly it must check if interrupt has been triggered on sensor's EXTI line)

};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/** NOTE: uses SysTick configured as SOOL_SysTick_DefaultConfig() */
volatile SOOL_IrReceiver SOOL_Sensor_IrReceiver_Init(SOOL_PinConfig_Int setup);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#endif /* SENSORS_IRRECEIVER_H_ */
