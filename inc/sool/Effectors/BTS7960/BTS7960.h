/*
 * BTS7960.h
 *
 *  Created on: 17.09.2019
 *      Author: user
 */

#ifndef INC_SOOL_EFFECTORS_BTS7960_BTS7960_H_
#define INC_SOOL_EFFECTORS_BTS7960_BTS7960_H_

struct _SOOL_BTS7960_SetupStruct {

};

typedef enum {
	BTS7960_FORWARD = 0u,
	BTS7960_REVERSE
} SOOL_BTS7960_Direction;

/* Forward declaration */
typedef struct _SOOL_BTS7960_Struct SOOL_BTS7960;

struct _SOOL_BTS7960_Struct {

	// ----- `base classes` section ------------
	volatile SOOL_TimerBasic 			base_tim;
	volatile SOOL_TimerOutputCompare 	base_pwm_fwd;
	volatile SOOL_TimerOutputCompare 	base_pwm_rev;
	SOOL_PinSwitch 						base_en_fwd;
	SOOL_PinSwitch 						base_en_rev;
	volatile SOOL_Button 				base_fault_fwd;
	volatile SOOL_Button 				base_fault_rev;

	// -----------------

	struct _SOOL_BTS7960_SetupStruct	_setup;

	uint8_t (*SetDirection)(volatile SOOL_BTS7960* driver_ptr, SOOL_BTS7960_Direction dir);
	uint8_t (*SetSpeed)(volatile SOOL_BTS7960* driver_ptr, SOOL_BTS7960_Direction dir, uint16_t speed);
	uint16_t (*GetSpeed)(volatile SOOL_BTS7960* driver_ptr, SOOL_BTS7960_Direction dir);
	uint8_t (*GetOvercurrentStatus)(volatile SOOL_BTS7960* driver_ptr, SOOL_BTS7960_Direction dir);

};

// NOTE: maximum switching frequency for the BTS7960 driver is 25 kHz

/// @brief Wrapper class which manages timer instances, pin switchers
/// and interrupt-driven inputs
/// @param timer_base
/// @param pwm_fwd
/// @param pwm_rev
/// @param en_fwd
/// @param en_rev
/// @param fault_fwd
/// @param fault_rev
/// @return
extern volatile SOOL_BTS7960 SOOL_Effector_BTS7960_Init(volatile SOOL_TimerBasic timer_base,
													    volatile SOOL_TimerOutputCompare pwm_fwd,
														volatile SOOL_TimerOutputCompare pwm_rev,
														SOOL_PinSwitch en_fwd,
														SOOL_PinSwitch en_rev,
														volatile SOOL_Button fault_fwd,
														volatile SOOL_Button fault_rev);

//volatile SOOL_TimerBasic timer_pwm = SOOL_Periph_TIM_TimerBasic_Init(TIM1, 36, 100, DISABLE); // 20 kHz (up to 25 kHz operation)
//	volatile SOOL_TimerOutputCompare motor_up_speed = SOOL_Periph_TIM_TimerOutputCompare_Init(timer_pwm, TIM_Channel_1, TIM_OCMode_PWM1, 90, DISABLE, TIM_OCIdleState_Set, TIM_OCPolarity_High, TIM_OutputState_Enable); 	// PWM 90%
//	volatile SOOL_TimerOutputCompare motor_down_speed = SOOL_Periph_TIM_TimerOutputCompare_Init(timer_pwm, TIM_Channel_2, TIM_OCMode_PWM1, 90, DISABLE, TIM_OCIdleState_Set, TIM_OCPolarity_High, TIM_OutputState_Enable); 	// PWM 90%
//	SOOL_PinSwitch motor_up_switch = SOOL_Effector_PinSwitch_Init(SOOL_Periph_GPIO_PinConfig_Initialize_NoInt(GPIOA, GPIO_Pin_8, GPIO_Mode_Out_PP));
//	SOOL_PinSwitch motor_down_switch = SOOL_Effector_PinSwitch_Init(SOOL_Periph_GPIO_PinConfig_Initialize_NoInt(GPIOA, GPIO_Pin_8, GPIO_Mode_Out_PP));
//	volatile SOOL_Button motor_up_fault = SOOL_Sensor_Button_Init(SOOL_Periph_GPIO_PinConfig_Initialize_Int(GPIOA, GPIO_Pin_9, GPIO_Mode_IN_FLOATING, EXTI_Trigger_Rising_Falling));
//	volatile SOOL_Button motor_down_fault

#endif /* INC_SOOL_EFFECTORS_BTS7960_BTS7960_H_ */
