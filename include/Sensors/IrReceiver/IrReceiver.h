/*
 * IrReceiver.h
 *
 *  Created on: 25.04.2019
 *      Author: user
 */

#ifndef IRRECEIVER_H_
#define IRRECEIVER_H_

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#include "Sensors/Button/Button.h"
#include "stm32f10x.h"
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/// \brief So-called objects, reference to Buttons takes place via these enums
typedef enum {
	IR_RECEIVER_LEFT = 0,
	IR_RECEIVER_RIGHT
} IrReceiverID;

// - - - - - - - - - - - - - - - - -

/* no need to defined SetupStruct one again as fields
 * would be the same */
typedef struct ButtonSetup IrReceiverSetup;

// - - - - - - - - - - - - - - - - -

struct IrReceiverState {
	uint8_t 			signal_flag;
};

// - - - - - - - - - - - - - - - - -

typedef struct {
	IrReceiverSetup setup;
	struct IrReceiverState state;
} IrReceiverData;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

extern void		IrReceiver_InitialConfig();
extern void 	IrReceiver_Config(IrReceiverID id, GPIO_TypeDef* port, uint16_t pin);
extern void 	IrReceiver_SwitchInterruptNVIC(IrReceiverID id, FunctionalState state);
extern void 	IrReceiver_SwitchInterruptEXTI(IrReceiverID id, FunctionalState state);
extern uint8_t 	IrReceiver_GetPushedFlag(IrReceiverID id);
extern uint8_t  IrReceiver_GetState(IrReceiverID id);

extern void 	EXTI4_IRQHandler();
extern void 	EXTI9_5_IRQHandler();

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#endif /* IRRECEIVER_H_ */
