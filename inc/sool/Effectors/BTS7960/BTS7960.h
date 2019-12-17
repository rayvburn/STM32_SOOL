/*
 * BTS7960.h
 *
 *  Created on: 17.09.2019
 *      Author: user
 */

#ifndef INC_SOOL_EFFECTORS_BTS7960_BTS7960_H_
#define INC_SOOL_EFFECTORS_BTS7960_BTS7960_H_

#include "sool/Peripherals/TIM/TimerBasic.h"
#include "sool/Peripherals/TIM/TimerOutputCompare.h"
#include "sool/Effectors/PinSwitch/PinSwitch.h"
#include "sool/Sensors/Button/Button.h"

typedef enum {
	BTS7960_FORWARD = 0u,
	BTS7960_REVERSE
} SOOL_BTS7960_Direction;

/* Forward declaration */
typedef struct _SOOL_BTS7960_Struct SOOL_BTS7960;

struct _SOOL_BTS7960_Struct {

	// ----- `base classes` section ------------
	SOOL_TimerBasic 			base_tim;
	SOOL_TimerOutputCompare 	base_pwm_fwd;
	SOOL_TimerOutputCompare 	base_pwm_rev;
	SOOL_PinSwitch 				base_en_fwd;
	SOOL_PinSwitch 				base_en_rev;
	SOOL_Button 				base_fault_fwd;
	SOOL_Button 				base_fault_rev;

	// -----------------

	uint8_t (*Stop)(volatile SOOL_BTS7960* driver_ptr);

	/**
	 * @brief Acts as a starter in the selected direction
	 * @param driver_ptr
	 * @param dir
	 * @return
	 */
	uint8_t (*SetDirection)(volatile SOOL_BTS7960* driver_ptr, SOOL_BTS7960_Direction dir);
	uint8_t (*SetSpeed)(volatile SOOL_BTS7960* driver_ptr, SOOL_BTS7960_Direction dir, uint16_t speed);
	uint16_t (*GetSpeed)(volatile SOOL_BTS7960* driver_ptr, SOOL_BTS7960_Direction dir);
	uint8_t (*GetOvercurrentStatus)(volatile SOOL_BTS7960* driver_ptr, SOOL_BTS7960_Direction dir);

};

/// @brief Wrapper class which manages timer instances, pin switchers
/// and interrupt-driven inputs
/// @param timer_base
/// @param pwm_fwd
/// @param pwm_rev
/// @param en_fwd
/// @param en_rev
/// @param fault_fwd
/// @param fault_rev
/// @return SOOL_BTS7960 instance
/// @note Maximum switching frequency for the BTS7960 driver is 25 kHz.
/// @note TimerBase instance should have interrupts DISABLED to prevent
/// extra processing time usage. With no interrupts enabled
/// one doesn't have to worry about Timer-related IRQHandlers
/// configuration.
extern volatile SOOL_BTS7960 SOOL_Effector_BTS7960_Init(volatile SOOL_TimerBasic timer_base,
													    volatile SOOL_TimerOutputCompare pwm_fwd,
														volatile SOOL_TimerOutputCompare pwm_rev,
														SOOL_PinSwitch en_fwd,
														SOOL_PinSwitch en_rev,
														volatile SOOL_Button fault_fwd,
														volatile SOOL_Button fault_rev);

/**
 * @brief Startup routine
 * @param driver_ptr
 */
extern void SOOL_Effector_BTS7960_Startup(volatile SOOL_BTS7960* driver_ptr);

// ---------------------------------------------------------------------------

/** @ref Datasheet: https://www.handsontec.com/dataspecs/module/BTS7960%20Motor%20Driver.pdf */

#endif /* INC_SOOL_EFFECTORS_BTS7960_BTS7960_H_ */
