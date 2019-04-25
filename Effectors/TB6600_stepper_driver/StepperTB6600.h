/*
 * StepperMotor.h
 *
 *  Created on: 04.10.2018
 *      Author: user
 */

#ifndef STEPPERTB6600_H_
#define STEPPERTB6600_H_

// - - - - - - - - - - - - -

/* Controller for HY-DIV268N-5A
 * TB6600+ */

#include "../CMSIS/device/stm32f10x.h"
#include "Globals.h"

// - - - - - - - - - - - - -

typedef enum {
	CW = 0u,
	CCW
} StepperDriverDir;

// - - - - - - - - - - - - -

typedef enum {
	STOP = 0u,
	ROTATE
} StepperDriverEn;

// - - - - - - - - - - - - -

typedef struct {
	int64_t				pos;
	StepperDriverDir 	dir;
	uint8_t				limit_set;
	int64_t 			pos_limit;
	uint8_t				pos_reached;
} Stepper_State;

// - - - - - - - - - - - - -

extern void StepperTB6600_Init		();
extern void StepperTB6600_SetSpeed	(uint16_t speed); 		  // speed expressed as us pulse
extern void StepperTB6600_UpdatePulseWidth();
extern void StepperTB6600_SetDir	(StepperDriverDir dir);
extern void StepperTB6600_SetEn		(StepperDriverEn state);
extern uint8_t StepperTB6600_SetLimitPos(int64_t limit);
extern int64_t StepperTB6600_GetCurrentPos();
extern int64_t StepperTB6600_GetLimitPos();
extern uint8_t StepperTB6600_IsLimitSet();
extern uint8_t StepperTB6600_PosReached();
extern uint16_t StepperTB6600_GetRPM();
extern void     StepperTB6600_ResetMotorPosition();
extern void	TIM2_IRQHandler(void);	// adjust IRQHandler name if needed

#endif /* STEPPERTB6600_H_ */
