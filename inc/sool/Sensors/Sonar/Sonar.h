/*
 * Sonar.h
 *
 *  Created on: 24.06.2019
 *      Author: user
 */

#ifndef INC_SOOL_SENSORS_SONAR_SONAR_H_
#define INC_SOOL_SENSORS_SONAR_SONAR_H_

#include <sool/Peripherals/GPIO/PinConfig_Int.h>
#include <sool/Peripherals/GPIO/PinConfig_NoInt.h>
#include "stm32f10x_tim.h"

//#include "stm32f10x.h"

/// \brief HC-SR04 sonar range finder controller
/// requires a timer for triggering emission of ultrasonic waves
/// (high state of at least 10 us) and counting waves' time of flight
/// back to sensor;
/// echo pin needs to be interrupt driven for accurate time measurement,
/// trigger pin may be just a PinSwitch `class` object
///
/// SHARED TIMER INSTANCE - circular buffer

// - - - - - - - - - - - - -
// cases to consider - - - -
// - - - - - - - - - - - - -
// timeout
// conversion to cm
// leave TIMER

// - - - - - - - - - - - - -
// methods - - - - - - - - -
// - - - - - - - - - - - - -
// startMeasurement
// isStarted
// isFinished
// getDistanceRaw
// getDistanceCm
// EXTI_InterruptHandler
// TIM_InterruptHandler


/// \brief ???????? status `register` which changes as interrupt routing fires up
struct _SOOL_SonarState {
	uint8_t 	started;
	uint8_t		finished;
	uint8_t 	timeout_occurred;
	//uint32_t 	distance_raw;
	uint16_t	distance_cm;
	//float		distance_cm_float;
};

struct _SOOL_SonarSetup {
	SOOL_PinConfig_Int		echo;
	//SOOL_PinConfig_NoInt 	trig;
	SOOL_PinSwitch			trigger;
};

// - - - - - - - - - - - - - - - - -

// forward declare a structure and typedef it to further make use of it inside a structure
struct _SOOL_SonarStruct;
typedef struct _SOOL_SonarStruct SOOL_Sonar;

struct _SOOL_SonarStruct {

	struct _SOOL_SonarSetup _setup;
	struct _SOOL_SonarState	_state;

	uint8_t 	(*StartMeasurement)(volatile SOOL_Sonar*);
	//void 		(*StartContinuousMeasurements)(volatile SOOL_Sonar*);	// DEPRECATED
	uint8_t 	(*IsStarted)(const volatile SOOL_Sonar*);
	uint8_t 	(*IsFinished)(const volatile SOOL_Sonar*);
	uint8_t 	(*DidTimeout)(const volatile SOOL_Sonar*);
	// uint32_t	(*GetDistanceRaw)(const volatile SOOL_Sonar*);			// DEPRECATED
	uint16_t	(*GetDistanceCm)(const volatile SOOL_Sonar*);
	//float		(*GetDistanceCmFloat)(const volatile SOOL_Sonar*);		// DEPRECATED

	void 		(*SetNvicState)(SOOL_PinConfig_Int*, const FunctionalState);
	void 		(*SetExtiState)(SOOL_PinConfig_Int*, const FunctionalState);

	void 		(*ReinitTimer)(void);
	void 		(*FreeTimer)(void);

	uint8_t 	(*_EXTI_InterruptHandler)(volatile SOOL_Sonar*); 		// routine fired in a proper ISR (firstly it must check if interrupt has been triggered on sensor's EXTI line)
	uint8_t 	(*_TIM_InterruptHandler)(volatile SOOL_Sonar*);

};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

const uint16_t SOOL_Sonar_DISTANCE_UNKNOWN = 9999;

// TODO
// extern
// extern
//volatile SOOL_Sonar SOOL_Sensor_Sonar_InitTimer(const SOOL_PinConfig_NoInt trig_pin, const SOOL_PinConfig_Int echo_pin, TIM_TypeDef* timer);
//volatile SOOL_Sonar SOOL_Sensor_Sonar_Init(const SOOL_PinConfig_NoInt trig_pin, const SOOL_PinConfig_Int echo_pin);

#endif /* INC_SOOL_SENSORS_SONAR_SONAR_H_ */
