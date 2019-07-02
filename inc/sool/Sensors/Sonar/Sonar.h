/*
 * Sonar.h
 *
 *  Created on: 24.06.2019
 *      Author: user
 */

#ifndef INC_SOOL_SENSORS_SONAR_SONAR_H_
#define INC_SOOL_SENSORS_SONAR_SONAR_H_

#include <sool/Peripherals/TIM/TimerBasic.h>
#include <sool/Peripherals/TIM/TimerOnePulse.h>
#include <sool/Peripherals/TIM/TimerInputCapture.h>

#include <sool/Peripherals/GPIO/PinConfig_Int.h>	// Echo
#include "sool/Effectors/PinSwitch/PinSwitch.h"		// Trigger


/// \brief HC-SR04 sonar range finder controller
/// requires a timer for triggering emission of ultrasonic waves
/// (high state of at least 10 us) and counting waves' time of flight
/// back to sensor;
/// echo pin needs to be interrupt driven for accurate time measurement,
/// trigger pin may be just a PinSwitch `class` object
///
/// SHARED TIMER INSTANCE - circular buffer



/// \brief ???????? status `register` which changes as interrupt routing fires up
struct _SOOL_SonarState {

	uint8_t 	started;
	uint8_t		finished;
	uint8_t 	timeout_occurred;
	uint16_t 	counter_val;
	uint16_t	distance_cm;;

};

//typedef enum {
//	SOOL_SONAR_RISING_EDGE = 0u,
//	SOOL_SONAR_FALLING_EDGE,
//	SOOL_SONAR_TIMEOUT
//} SOOL_Sonar_IsrStatus;

// - - - - - - - - - - - - - - - - -

// forward declare a structure and typedef it to further make use of it inside a structure
struct _SOOL_SonarStruct;
typedef struct _SOOL_SonarStruct SOOL_Sonar;

struct _SOOL_SonarStruct {

	struct _SOOL_SonarState	_state;

	// --------- base classes section ------------
	SOOL_PinConfig_AltFunction	base_echo;
	SOOL_PinConfig_AltFunction	base_trigger;
	SOOL_TimerOnePulse 			base_tim_out;
	SOOL_TimerInputCapture		base_tim_in;

	// --------- derived class section -----------
	uint8_t 	(*StartMeasurement)(volatile SOOL_Sonar*);
	uint8_t 	(*IsStarted)(const volatile SOOL_Sonar*);
	uint8_t 	(*IsFinished)(const volatile SOOL_Sonar*);
	uint8_t 	(*DidTimeout)(const volatile SOOL_Sonar*);
	uint16_t	(*GetDistanceCm)(const volatile SOOL_Sonar*);

//	uint8_t 	(*_EXTI_InterruptHandler)(volatile SOOL_Sonar*); 		// routine fired in a proper ISR (firstly it must check if interrupt has been triggered on sensor's EXTI line)
//	uint8_t 	(*_TIM_IC_InterruptHandler)(volatile SOOL_Sonar*);
//	uint8_t 	(*_TIM_Update_InterruptHandler)(volatile SOOL_Sonar*);

//	void 		(*_CalculateDistance)(volatile SOOL_Sonar*);
	uint8_t 	(*_PulseEnd_InterruptHandler)(volatile SOOL_Sonar*);
	uint8_t 	(*_EchoEdge_InterruptHandler)(volatile SOOL_Sonar*);
	uint8_t 	(*_Timeout_InterruptHandler)(volatile SOOL_Sonar*);

};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

//const uint16_t SOOL_Sonar_DISTANCE_UNKNOWN = 9999;

// TODO
// extern
// extern
//volatile SOOL_Sonar SOOL_Sensor_Sonar_InitTimer(const SOOL_PinConfig_NoInt trig_pin, const SOOL_PinConfig_Int echo_pin, TIM_TypeDef* timer);
//volatile SOOL_Sonar SOOL_Sensor_Sonar_Init(const SOOL_PinConfig_NoInt trig_pin, const SOOL_PinConfig_Int echo_pin);

#endif /* INC_SOOL_SENSORS_SONAR_SONAR_H_ */
