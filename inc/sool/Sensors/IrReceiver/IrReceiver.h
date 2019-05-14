/*
 * IrReceiver.h
 *
 *  Created on: 25.04.2019
 *      Author: user
 */

#ifndef SENSORS_IRRECEIVER_H_
#define SENSORS_IRRECEIVER_H_

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#include <sool/Common/PinConfig.h>

#include "stm32f10x.h"
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/// \brief TSOP3123/1736 IR receiver controller
/// detects simple 1 bit signal

/// \brief status `register` which changes as interrupt routing fires up
typedef struct {
	uint8_t 	received_flag;	// received signal (active low)
	uint8_t		last_state_int; // last pin state during interrupt routine processing
	uint32_t 	last_edge_time;	// non-defined if it will be milliseconds or 0.1 seconds
} IrReceiverState;

// - - - - - - - - - - - - - - - - -

// forward declare a structure and typedef it to further make use of it inside a structure
struct IrReceiverStruct;
typedef struct IrReceiverStruct IrReceiver;

// IR Receiver is interrupt-driven - use previously defined structure for this task
struct IrReceiverStruct {
	SOOL_PinConfigInt 	_setup;
	IrReceiverState 	_state;
	uint8_t 	(*GetReceptionFlag)(volatile IrReceiver*); // interrupt-driven
	uint8_t 	(*GetCurrentState)(const volatile IrReceiver*);
	uint32_t 	(*GetLastEdgeTime)(const volatile IrReceiver*);
	uint8_t 	(*IsStateStable)(const volatile IrReceiver*, const uint32_t); /// \param[in] object pointer, time gap required; current time requested from SysTick
	void 		(*SetNvicState)(SOOL_PinConfigInt*, const FunctionalState state);
	void 		(*SetExtiState)(SOOL_PinConfigInt*, const FunctionalState state);
	uint8_t 	(*_InterruptHandler)(volatile IrReceiver*); // routine fired in a proper ISR (firstly it must check if interrupt has been triggered on sensor's EXTI line)
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

volatile IrReceiver SOOL_Sensors_IrReceiver_Init(SOOL_PinConfigInt setup);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#endif /* SENSORS_IRRECEIVER_H_ */
