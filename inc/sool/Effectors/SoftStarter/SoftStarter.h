/*
 * SoftStarter.h
 *
 *  Created on: 17.12.2019
 *      Author: user
 */

#ifndef INC_SOOL_EFFECTORS_SOFTSTARTER_SOFTSTARTER_H_
#define INC_SOOL_EFFECTORS_SOFTSTARTER_SOFTSTARTER_H_

#include <stdint.h>

struct _SOOL_SoftStarterStruct;
typedef struct _SOOL_SoftStarterStruct SOOL_SoftStarter;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

struct _SOOL_SoftStarterConfigStruct {
	uint16_t pulse_start;
	uint16_t increments;
};

struct _SOOL_SoftStarterSetupStruct {
	int16_t  pulse_change;
	uint32_t time_change_gap;
};

struct _SOOL_SoftStarterStateStruct {
	uint16_t pulse_last;
	uint16_t changes_left; // how many times the pulse will be incremented/decremented
	uint32_t time_last_pulse_change;
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

struct _SOOL_SoftStarterStruct {

	struct _SOOL_SoftStarterSetupStruct	 _setup;
	struct _SOOL_SoftStarterStateStruct  _state;
	struct _SOOL_SoftStarterConfigStruct _config;

	/**
	 * @brief Reconfigures internal parameters of the ramp
	 * @param ss_ptr
	 * @param pulse_start
	 * @param pulse_end
	 * @param duration
	 * @return 0 if operation was NOT successful
	 */
	uint8_t (*Reconfigure)(SOOL_SoftStarter* ss_ptr, uint16_t pulse_start, uint16_t pulse_end, uint32_t duration);

	/**
	 * @brief Starts the soft starting procedure. Saves the timestamp (milliseconds) internally.
	 * @param ss_ptr
	 */
	void (*Start)(SOOL_SoftStarter* ss_ptr);

	/**
	 * @brief Processes the possible change of the pulse value (according to the timestamp)
	 * @param ss_ptr
	 * @return 1 if pulse was updated
	 */
	uint8_t (*Process)(SOOL_SoftStarter* ss_ptr);

	/**
	 * @brief Evaluates whether the soft start procedure has finished
	 * @param ss_ptr
	 * @return
	 * @note The internal state flag `finished` is not cleared after this call (stays until the next `Start` call).
	 */
	uint8_t (*IsFinished)(const SOOL_SoftStarter* ss_ptr);

	/**
	 * @brief Retrieves the last pulse value
	 * @param ss_ptr
	 * @return Returns the newly calculated `pulse` value (or the last valid
	 * one if the method is called after `Process` returns 0)
	 */
	uint16_t (*Get)(SOOL_SoftStarter* ss_ptr);

};

// --------------------------------------------------------------

/**
 * @brief Provides linear change of the pulse value (timer-specific) along the time given
 * by `duration` parameter. The ramp can be of a rising or falling tendency (adjustable
 * via pulse_start > pulse_end or pulse_start < pulse_end).
 * @param pulse_start
 * @param pulse_end
 * @param duration: how long the soft starting procedure should take [in milliseconds]
 * @return
 * @note This class requires SysTick timer to be configured by default, call
 * 		 @ref SOOL_Periph_TIM_SysTick_DefaultConfig at the startup of the MCU.
 * @note If there is an instance in the application which blocks the MCU operation
 * for more than 1 millisecond then SoftStart procedure will not work properly
 * (time resolution of the `pulse` increment is 1 ms).
 */
extern SOOL_SoftStarter SOOL_Effector_SoftStarter_Initialize(uint16_t pulse_start, uint16_t pulse_end, uint32_t duration);

// --------------------------------------------------------------

#endif /* INC_SOOL_EFFECTORS_SOFTSTARTER_SOFTSTARTER_H_ */
