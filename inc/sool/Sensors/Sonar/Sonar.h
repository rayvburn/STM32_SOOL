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

	// --------- base classes section ------------ // in fact it is rather a composition but let it be
	SOOL_PinConfig_AltFunction	base_echo;
	SOOL_PinConfig_AltFunction	base_trigger;
	SOOL_TimerOnePulse 			base_tim_out; // OnePulse Mode
	SOOL_TimerInputCapture		base_tim_in;

	// --------- derived class section -----------
	struct _SOOL_SonarState	_state;

	/**
	 * @brief Starts measurement, requires NVIC to be enabled
	 * @note EnableNVIC of the TimerBasic (TimerOnePulse->TimerOutputCompare->TimerBasic)
	 * needs to be called before first measurement but after placing sonar
	 * instances in the IRQ handlers. Call should look like (for instance called `sonar_l`):
	 * sonar_l.base_tim_in.base.EnableNVIC(&sonar_l.base_tim_in.base);
	 * @note This function differ depending on chosen initializer (whether full or echo-only)
	 * @param
	 * @return
	 */
	uint8_t 	(*StartMeasurement)(volatile SOOL_Sonar*);
	uint8_t 	(*IsStarted)(const volatile SOOL_Sonar*);
	uint8_t 	(*IsFinished)(const volatile SOOL_Sonar*);
	uint8_t 	(*DidTimeout)(const volatile SOOL_Sonar*);
	uint16_t	(*GetDistanceCm)(const volatile SOOL_Sonar*);

	SOOL_PinConfig_AltFunction (*GetEchoPinConfig)(const volatile SOOL_Sonar*);
	SOOL_PinConfig_AltFunction (*GetTriggerPinConfig)(const volatile SOOL_Sonar*);
	SOOL_TimerOnePulse (*GetTimerOP)(const volatile SOOL_Sonar*);

	/**
	 * @brief _PulseEnd_EventHandler - when handling interrupt for an instance of Sonar which
	 * shares timer peripheral with another sensor, call _PulseEnd_EventHandler of the `child`
	 * instance first
	 * @param
	 * @return
	 */
	uint8_t 	(*_PulseEnd_EventHandler)(volatile SOOL_Sonar*);
	uint8_t 	(*_EchoEdge_EventHandler)(volatile SOOL_Sonar*);

	/**
	 * @brief _Timeout_EventHandler - when handling interrupt for an instance of Sonar which
	 * shares timer peripheral with another sensor, call _Timeout_EventHandler of the `child`
	 * instance in the same routine as the `parent's` one
	 * @param
	 * @return
	 */
	uint8_t 	(*_Timeout_EventHandler)(volatile SOOL_Sonar*);

};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

extern volatile SOOL_Sonar SOOL_Sensor_Sonar_Init(GPIO_TypeDef* trig_port, uint16_t trig_pin,
		uint16_t trig_tim_channel, GPIO_TypeDef* echo_port, uint16_t echo_pin,
		uint16_t echo_tim_channel, TIM_TypeDef* TIMx, uint16_t range_max);

extern volatile SOOL_Sonar SOOL_Sensor_Sonar_InitEcho(GPIO_TypeDef* echo_port, uint16_t echo_pin,
		uint16_t echo_tim_channel, SOOL_PinConfig_AltFunction trig_cfg,
		volatile SOOL_TimerOnePulse timer_pulse);

// FIXME: not tested, this will likely require first sensor's measurement to finish until possibility
// of a start of the second one will arise
extern volatile SOOL_Sonar SOOL_Sensor_Sonar_InitTrigEcho(GPIO_TypeDef* trig_port, uint16_t trig_pin,
		uint16_t trig_tim_channel, GPIO_TypeDef* echo_port, uint16_t echo_pin,
		uint16_t echo_tim_channel, uint16_t range_max, volatile SOOL_TimerBasic timer_base);

/**
 * @brief Single sonar startup routine, consists of NVIC enabling and `StartMeasurement` method call.
 * @param sonar_ptr
 * @note If 2 sonars share single timer instance, only 1 NVIC enabling and 2 `StartMeasurement`
 * calls are required
 * @note Remember to copy the object into proper IRQHandlers
 */
extern void SOOL_Sensor_Sonar_Startup(volatile SOOL_Sonar* sonar_ptr);

/* Example of use (2 sonars at once) can be found at:
 * https://gitlab.com/frb-pow/002tubewaterflowmcu/blob/63200cd02eac11177d323c57a406d01d8ad62d96/src/main.c#L60 */

#endif /* INC_SOOL_SENSORS_SONAR_SONAR_H_ */
