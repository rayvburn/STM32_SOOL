/*
 * TimerInputCapture.h
 *
 *  Created on: 28.06.2019
 *      Author: user
 */

#ifndef INC_SOOL_PERIPHERALS_TIM_TIMERINPUTCAPTURE_H_
#define INC_SOOL_PERIPHERALS_TIM_TIMERINPUTCAPTURE_H_

#include "TimerBasic.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/* Forward declaration */
struct _SOOL_TimerInputCaptureStruct;
typedef struct _SOOL_TimerInputCaptureStruct SOOL_TimerInputCapture;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

struct _SOOL_TimerInputCaptureStateStruct {
	uint8_t  transition_detected;
	uint16_t transition_counter_val;
};

struct _SOOL_TimerInputCaptureSetupStruct {
	uint16_t TIM_IT_CCx; 		// channel compare ID
	uint16_t TIM_FLAG_CCxOF;	// overcapture
	uint16_t TIM_Channel_X;		// CCRx register content
};

/**
 * Composition of SOOL_TimerBasic and SOOL_TimerInputCapture class' contents
 * Unluckily there is no way to access base `class` members directly
 */
struct _SOOL_TimerInputCaptureStruct {

	// base section --------------------------------------------------------
	SOOL_TimerBasic tim_basic;

	// derived section -----------------------------------------------------
	struct _SOOL_TimerInputCaptureStateStruct _state;
	struct _SOOL_TimerInputCaptureSetupStruct _setup;

	void (*Start)(volatile SOOL_TimerInputCapture*);
	void (*Stop)(volatile SOOL_TimerInputCapture*);
	uint16_t (*GetSavedCounterVal)(const volatile SOOL_TimerInputCapture*);
	uint8_t (*_InterruptHandler)(volatile SOOL_TimerInputCapture*);

};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

extern volatile SOOL_TimerInputCapture SOOL_Periph_TIM_TimerInputCapture_Init(TIM_TypeDef* TIMx, uint16_t prescaler, uint16_t period);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#endif /* INC_SOOL_PERIPHERALS_TIM_TIMERINPUTCAPTURE_H_ */
