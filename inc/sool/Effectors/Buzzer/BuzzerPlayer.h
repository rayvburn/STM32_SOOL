/*
 * BuzzerPlayer.h
 *
 *  Created on: 22.08.2019
 *      Author: user
 */

#ifndef INC_SOOL_EFFECTORS_BUZZER_BUZZERPLAYER_H_
#define INC_SOOL_EFFECTORS_BUZZER_BUZZERPLAYER_H_

#include "sool/Effectors/PinSwitch/PinSwitch.h"

struct _SOOL_Buzzer_SetupStruct {
	uint32_t 	start_time;
	uint8_t		mode;			// corresponds to the currently executed method
	uint8_t 	status; 		// whether running or not
};

typedef enum {
	SOOL_BUZZER_MODE_SINGLE = 0,
	SOOL_BUZZER_MODE_DOUBLE,
	SOOL_BUZZER_MODE_WARNING
} SOOL_Buzzer_Mode;

/* Forward declaration */
typedef struct _SOOL_Buzzer_Struct SOOL_Buzzer;

struct _SOOL_Buzzer_Struct {

	SOOL_PinSwitch 					base;

	struct _SOOL_Buzzer_SetupStruct _setup;

	uint8_t (*SetMode)(SOOL_Buzzer *buzz_ptr, SOOL_Buzzer_Mode mode, uint32_t millis);
	uint8_t (*Play)(SOOL_Buzzer *buzz_ptr, uint32_t millis);

};

extern SOOL_Buzzer SOOL_Effector_Buzzer_Init(SOOL_PinConfig_NoInt setup);

#endif /* INC_SOOL_EFFECTORS_BUZZER_BUZZERPLAYER_H_ */
