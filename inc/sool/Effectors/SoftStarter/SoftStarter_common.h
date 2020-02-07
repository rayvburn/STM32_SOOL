/*
 * SoftStarter_common.h
 *
 *  Created on: 06.02.2020
 *      Author: user
 */

#ifndef INC_SOOL_EFFECTORS_SOFTSTARTER_SOFTSTARTER_COMMON_H_
#define INC_SOOL_EFFECTORS_SOFTSTARTER_SOFTSTARTER_COMMON_H_

#include <stdint.h>

// --------------------------------------------------------------------------------------------------

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
	uint16_t changes_left; // how many more times the pulse will be incremented/decremented
	uint32_t time_last_pulse_change;
};

// --------------------------------------------------------------------------------------------------

/**
 * @brief Reconfigures internal parameters of the ramp
 * @param config_ptr
 * @param setup_ptr
 * @param state_ptr
 * @param pulse_start
 * @param pulse_end
 * @param duration: number of iterated value counts; for down-counting (i.e. negative `duration`)
 *   set `duration` as absolute value and update (via @ref SOOL_Effector_SoftStarter_Process)
 *   with decreasing numbers as `stamp`
 * @return
 */
extern uint8_t SOOL_Effector_SoftStarter_Reconfigure(struct _SOOL_SoftStarterConfigStruct* config_ptr,
		struct _SOOL_SoftStarterSetupStruct* setup_ptr,
		struct _SOOL_SoftStarterStateStruct* state_ptr,
		uint16_t pulse_start, uint16_t pulse_end, uint32_t duration);


/**
 * @brief Starts the soft starting procedure. Saves the `stamp` value.
 * @param config_ptr
 * @param setup_ptr
 * @param state_ptr
 * @param stamp
 */
extern void SOOL_Effector_SoftStarter_Start(struct _SOOL_SoftStarterConfigStruct* config_ptr,
				struct _SOOL_SoftStarterSetupStruct* setup_ptr, struct _SOOL_SoftStarterStateStruct* state_ptr, uint32_t stamp);


/**
 * @brief Processes the possible change of the pulse value (according to the stamp)
 * @param setup_ptr
 * @param state_ptr
 * @param stamp
 * @return
 */
extern uint8_t SOOL_Effector_SoftStarter_Process(struct _SOOL_SoftStarterSetupStruct* setup_ptr,
				struct _SOOL_SoftStarterStateStruct* state_ptr, uint32_t stamp);


/**
 * @brief Evaluates whether the soft start procedure is finished
 * @param state_ptr
 * @return
 */
extern uint8_t SOOL_Effector_SoftStarter_IsFinished(const struct _SOOL_SoftStarterStateStruct* state_ptr);


/**
 * @brief Retrieves the last pulse value
 * @param state_ptr
 * @return
 */
extern uint16_t SOOL_Effector_SoftStarter_Get(const struct _SOOL_SoftStarterStateStruct* state_ptr);




#endif /* INC_SOOL_EFFECTORS_SOFTSTARTER_SOFTSTARTER_COMMON_H_ */
