/*
 * BuzzerPlayer.h
 *
 *  Created on: 22.08.2019
 *      Author: user
 */

#ifndef INC_SOOL_EFFECTORS_BUZZER_BUZZERPLAYER_H_
#define INC_SOOL_EFFECTORS_BUZZER_BUZZERPLAYER_H_

#include "sool/Effectors/PinSwitch/PinSwitch.h"
#include "sool/Workflow/ActionTimer.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

struct _SOOL_Buzzer_SetupStruct {
	uint8_t		mode;			// corresponds to the currently executed method
	uint8_t 	status; 		// whether running or not
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

typedef enum {
	SOOL_BUZZER_MODE_IDLE = 0,
	SOOL_BUZZER_MODE_SINGLE,
	SOOL_BUZZER_MODE_DOUBLE,
	SOOL_BUZZER_MODE_WARNING
} SOOL_Buzzer_Mode;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/* Forward declaration */
typedef struct _SOOL_Buzzer_Struct SOOL_Buzzer;

struct _SOOL_Buzzer_Struct {

	SOOL_PinSwitch 					base;
	SOOL_ActionTimer				base_tim;

	struct _SOOL_Buzzer_SetupStruct _setup;

	/// @brief Sets the buzzer `melody`. Start time stamp is set which is further utilized
	/// by `Play` method.
	/// @param buzz_ptr
	/// @param mode
	/// @return
	uint8_t (*SetMode)(SOOL_Buzzer *buzz_ptr, SOOL_Buzzer_Mode mode);

	/// @brief Play should follow the `SetMode` call.
	/// @param buzz_ptr
	/// @return 1 if currently playing or 0 if finished
	uint8_t (*Play)(SOOL_Buzzer *buzz_ptr);

};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/**
 * @brief A `class` providing a few melodies to be played via a standard buzzer.
 * @note It requires SysTick timer to be configured as default @ref SOOL_Periph_TIM_SysTick_DefaultConfig
 * @param setup
 * @return
 */
extern SOOL_Buzzer SOOL_Effector_Buzzer_Init(SOOL_PinConfig_NoInt setup);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/* Test code:

	SOOL_Common_ClockConfigDefault(); 		// 72 MHz with external oscillator
	SOOL_Periph_TIM_SysTick_DefaultConfig();// 1000 ticks per second

	// Buzzer
	SOOL_Buzzer buzzer = SOOL_Effector_Buzzer_Init(SOOL_Periph_GPIO_PinConfig_Initialize_NoInt(GPIOB, GPIO_Pin_10, GPIO_Mode_Out_PP));

	buzzer.SetMode(&buzzer, SOOL_BUZZER_MODE_WARNING);

	while (1) {
		buzzer.Play(&buzzer);
	}

 */
#endif /* INC_SOOL_EFFECTORS_BUZZER_BUZZERPLAYER_H_ */
