/*
 * Sparkfun_EasyDriver.h
 *
 *  Created on: 08.10.2018
 *      Author: user
 */

#ifndef SPARKFUN_EASYDRIVER_H_
#define SPARKFUN_EASYDRIVER_H_

#include "sool/Effectors/TB6600_stepper_driver/StepperTB6600.h"	// contains dir and enable enums
#include "stm32f10x.h"

/* 2 EasyDrivers used in this project
 * both use the same timer but different
 * output compare pins */

/* limit switches of motors are located on the outer side of the enclosure */

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

typedef enum {
	LIFTER = 0,	// currently assigned to TIM3_OC1 pin
	PAWL		// currently assigned to TIM3_OC2 pin
} EasyDriver_ID;

// - - - - - - -

typedef enum {
	DOWN = 0,
	UP
} EasyDriver_LinearDirection;

// - - - - - - -

typedef enum {
	MOUNT_BOTTOM_UP = 0,
	MOUNT_BOTTOM_DOWN
} EasyDriver_MountType;

// - - - - - - -

typedef enum {
	/*
	 * FORWARD position is detected on the base of reaching the target,
	 * Position to which the motor should go in the limit switch direction is always set
	 * with an excess, so it won't reach it (mechanically permitted) before the limit switch signal.
	 * On the other side, the FORWARD position is set in mm to fulfill experiment requirements.
	 */
	POSITION_LIMITED = 0,
	POSITION_FORWARD
} EasyDriver_MotorPosition;

// - - - - - - -

typedef enum {
	TIMER_LEAVE,
	TIMER_TOGGLE
} EasyDriver_TimerState;

// - - - - - - -

typedef struct {

	uint16_t 				gpio_enable_pin;
	GPIO_TypeDef * 			gpio_enable_port;
	uint16_t 				gpio_step_pin;
	GPIO_TypeDef * 			gpio_step_port;
	uint16_t 				gpio_dir_pin;
	GPIO_TypeDef * 			gpio_dir_port;
	TIM_TypeDef*			tim_type;
	IRQn_Type 				timer_irqn;			// adjust IRQHandler function manually
	EasyDriver_MountType	mount;

} EasyDriver_Config;

// - - - - - - -

typedef struct {
	EasyDriver_MotorPosition	current_position;
	uint8_t						currently_moving;
	int64_t						pos;
	EasyDriver_LinearDirection 	dir;
	int64_t 					pos_limit; // position at which motor should stop
	uint8_t 					pos_limit_set;
	uint8_t						pos_limit_reached;
	uint32_t	 				mm_move_per_rev;
	uint16_t					steps_per_rev;
	uint8_t						microstepping;
} EasyDriver_State;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

extern void 	EasyDriver_Init(uint16_t en_pin, 	GPIO_TypeDef *en_port,
								uint16_t step_pin, 	GPIO_TypeDef *step_port,
								uint16_t dir_pin, 	GPIO_TypeDef *dir_port,
								TIM_TypeDef* tim,	IRQn_Type tim_irq,
								EasyDriver_MountType mount, EasyDriver_ID id );
extern void 	EasyDriver_SetMotorParameters(uint16_t steps_per_rev, uint32_t mm_move_per_rev, uint8_t microsteps, EasyDriver_ID id);
extern void		EasyDriver_SetInitialPosition(uint8_t limit_switch_state, EasyDriver_ID id);
extern uint8_t	EasyDriver_GetMotorPosition(EasyDriver_ID id);
extern void 	EasyDriver_MoveLinear(uint32_t len_mm, uint16_t speed, EasyDriver_LinearDirection dir, uint8_t limit_switch_state, EasyDriver_ID id);
extern void 	EasyDriver_WaitForEvent(EasyDriver_ID id, uint8_t event_flag);	// immediately ends a movement and resets according internal flags when the flag is true
extern uint8_t 	EasyDriver_IsLinearMovementFinished(EasyDriver_ID id);
extern void 	EasyDriver_SetSpeed	(uint16_t speed, EasyDriver_ID id ); 	// speed expressed as us pulse
extern void 	EasyDriver_UpdatePulseWidth();
extern void 	EasyDriver_SetDir(EasyDriver_LinearDirection dir, EasyDriver_ID id );
extern void 	EasyDriver_SetEn(StepperDriverEn state, EasyDriver_TimerState timer_state, EasyDriver_ID id );
extern void 	EasyDriver_MakeSteps(uint32_t steps, EasyDriver_LinearDirection dir, EasyDriver_ID id );
extern void		TIM3_IRQHandler(void); // adjust IRQHandler name if needed

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#endif /* SPARKFUN_EASYDRIVER_H_ */
