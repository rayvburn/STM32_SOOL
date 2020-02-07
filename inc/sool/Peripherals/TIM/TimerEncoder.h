/*
 * TimerEncoder.h
 *
 *  Created on: 30.01.2020
 *      Author: user
 */

#ifndef INC_SOOL_PERIPHERALS_TIM_TIMERENCODER_H_
#define INC_SOOL_PERIPHERALS_TIM_TIMERENCODER_H_

#include <sool/Peripherals/TIM/TimerBasic.h>
#include <sool/Peripherals/TIM/TimerInputCapture.h>

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/* Forward declaration */
struct _SOOL_TimerEncoderStruct;
typedef struct _SOOL_TimerEncoderStruct SOOL_TimerEncoder;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/** Provides basic periodical events handling feature */
struct _SOOL_TimerEncoderStruct {

	SOOL_TimerBasic							base;
	uint16_t (*GetCount)(volatile SOOL_TimerEncoder* tim_ptr);

};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/**
  * @note Documentation taken from ST's @ref TIM_EncoderInterfaceConfig function
  * @note "Encoder interface mode acts simply as an external clock with direction selection. This
  *   means that the counter just counts continuously between 0 and the auto-reload value in the
  *   TIMx_ARR register (0 to ARR or ARR down to 0 depending on the direction). So user must
  *   configure TIMx_ARR before starting. in the same way, the capture, compare, prescaler,
  *   repetition counter, trigger output features continue to work as normal. Encoder mode and
  *   External clock mode 2 are not compatible and must not be selected together."
  *   RM0008 Revision 19, p. 329
  * @note This `class` uses InputCapture mode in a simple way thus does not have @ref SOOL_TimerInputCapture
  *   listed as a `base class`
  * @brief  Configures the TIMx Encoder Interface.
  * @param  TIMx: where x can be  1, 2, 3, 4, 5 or 8 to select the TIM peripheral.
  * @param  TIM_EncoderMode: specifies the TIMx Encoder Mode.
  *   This parameter can be one of the following values:
  *     @arg TIM_EncoderMode_TI1: Counter counts on TI1FP1 edge depending on TI2FP2 level.
  *     @arg TIM_EncoderMode_TI2: Counter counts on TI2FP2 edge depending on TI1FP1 level.
  *     @arg TIM_EncoderMode_TI12: Counter counts on both TI1FP1 and TI2FP2 edges depending
  *                                on the level of the other input.
  * @param  TIM_IC1Polarity: specifies the IC1 Polarity
  *   This parameter can be one of the following values:
  *     @arg TIM_ICPolarity_Falling: IC Falling edge.
  *     @arg TIM_ICPolarity_Rising: IC Rising edge.
  * @param  TIM_IC2Polarity: specifies the IC2 Polarity
  *   This parameter can be one of the following values:
  *     @arg TIM_ICPolarity_Falling: IC Falling edge.
  *     @arg TIM_ICPolarity_Rising: IC Rising edge.
  * @param enable_int_update: whether to enable INTERRUPT on counter overflow (ARR = 0xFFFF);
  *   as an interrupt handler use the `base._InterruptHandler`
  * @retval SOOL_TimerEncoder instance
  */
extern volatile SOOL_TimerEncoder SOOL_Periph_TIM_TimerEncoder_Init(TIM_TypeDef* TIMx, uint16_t TIM_EncoderMode,
        uint16_t TIM_IC1Polarity, uint16_t TIM_IC2Polarity, FunctionalState enable_int_update);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

extern void SOOL_Periph_TIM_TimerEncoder_Startup(volatile SOOL_TimerEncoder* encoder_ptr);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


#endif /* INC_SOOL_PERIPHERALS_TIM_TIMERENCODER_H_ */
