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
	TIM_ICInitTypeDef config;
	uint16_t TIM_IT_CCx; 		// channel compare ID
	uint16_t TIM_FLAG_CCxOF;	// overcapture
	uint16_t TIM_Channel_x;		// acquisition of the CCRx register content
	uint8_t NVIC_IRQ_channel;
};

/**
 * Composition of SOOL_TimerBasic and SOOL_TimerInputCapture class' contents
 * Unluckily there is no way to access base `class` members directly.
 * IMPORTANT: in case of derived classes which are interrupt-related
 * one must remember to call base's interrupt handlers too
 */
struct _SOOL_TimerInputCaptureStruct {

	// base section --------------------------------------------------------
	SOOL_TimerBasic base;

	// derived section -----------------------------------------------------
	struct _SOOL_TimerInputCaptureStateStruct _state;
	struct _SOOL_TimerInputCaptureSetupStruct _setup;

	void (*Start)(volatile SOOL_TimerInputCapture*);
	void (*Stop)(volatile SOOL_TimerInputCapture*);

	void (*ReinitIC)(volatile SOOL_TimerInputCapture*);
	void (*DisableIC)(volatile SOOL_TimerInputCapture*);	// Disable InputCapture

//	/**
//	 * EnableNVIC can be called after timer's interrupt handler was placed in IRQHandler
//	 * function and when there is a certainty that all objects driven by timer interrupts
//	 * were already put in proper IRQHandlers too
//	 * @param Pointer to SOOL_TimerOutputCompare instance
//	 */
//	void (*EnableNVIC)(volatile SOOL_TimerInputCapture*);
//	void (*DisableNVIC)(volatile SOOL_TimerInputCapture*);

	/**
	 *
	 * @param SOOL_TimerInputCapture*
	 * @param TIM_ICPolarity
	 */
	void (*SetPolarity)(volatile SOOL_TimerInputCapture*, uint16_t TIM_ICPolarity);

	uint16_t (*GetSavedCounterVal)(const volatile SOOL_TimerInputCapture*);
	uint8_t (*_InterruptHandler)(volatile SOOL_TimerInputCapture*);

};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

extern volatile SOOL_TimerInputCapture SOOL_Periph_TIM_TimerInputCapture_Init(volatile SOOL_TimerBasic tim_basic,
		uint16_t channel, uint16_t ic_polarity, FunctionalState enable_int_cc);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#endif /* INC_SOOL_PERIPHERALS_TIM_TIMERINPUTCAPTURE_H_ */
