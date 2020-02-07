/*
 * SoftStarterCustom.h
 *
 *  Created on: 06.02.2020
 *      Author: user
 */

#ifndef INC_SOOL_EFFECTORS_SOFTSTARTER_SOFTSTARTERCUSTOM_H_
#define INC_SOOL_EFFECTORS_SOFTSTARTER_SOFTSTARTERCUSTOM_H_

#include <sool/Effectors/SoftStarter/SoftStarter_common.h>

struct _SOOL_SoftStarterCustomStruct;
typedef struct _SOOL_SoftStarterCustomStruct SOOL_SoftStarterCustom;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

struct _SOOL_SoftStarterCustomStruct {

	struct _SOOL_SoftStarterSetupStruct	 _setup;
	struct _SOOL_SoftStarterStateStruct  _state;
	struct _SOOL_SoftStarterConfigStruct _config;

	/**
	 * @brief Reconfigures internal parameters of the ramp
	 * @param ss_ptr
	 * @param pulse_start
	 * @param pulse_end
	 * @param duration: how long the ramp should be executed
	 * @return 0 if operation was NOT successful
	 */
	uint8_t (*Reconfigure)(SOOL_SoftStarterCustom* ss_ptr, uint16_t pulse_start, uint16_t pulse_end, uint32_t duration);

	/**
	 * @brief Starts the soft starting procedure.
	 * @param ss_ptr
	 * @param stamp: reference value used for comparison with `duration`
	 */
	void (*Start)(SOOL_SoftStarterCustom* ss_ptr, uint32_t stamp);

	/**
	 * @brief Processes the possible change of the pulse value (according to the stamp)
	 * @param ss_ptr
	 * @return 1 if pulse was updated
	 */
	uint8_t (*Process)(SOOL_SoftStarterCustom* ss_ptr, uint32_t stamp);

	/**
	 * @brief Evaluates whether the soft start procedure has finished
	 * @param ss_ptr
	 * @return
	 * @note The internal state flag `finished` is not cleared after this call (stays until the next `Start` call).
	 */
	uint8_t (*IsFinished)(const SOOL_SoftStarterCustom* ss_ptr);

	/**
	 * @brief Retrieves the last pulse value
	 * @param ss_ptr
	 * @return Returns the newly calculated `pulse` value (or the last valid
	 * one if the method is called after `Process` returns 0)
	 */
	uint16_t (*Get)(const SOOL_SoftStarterCustom* ss_ptr);

};

// --------------------------------------------------------------

/**
 * @brief Provides a smooth change of the `pulse` value (timer-specific) along the `arbitrary unit` given
 *   by `duration` parameter. The ramp can be of a rising or falling tendency (adjustable
 *   via pulse_start > pulse_end or pulse_start < pulse_end).
 * @note Check @ref SoftStarter too
 * @param pulse_start
 * @param pulse_end
 * @param duration: how long the soft starting procedure should take [in arbitrary units,
 *   where the current value must be updated in each iteration of the control loop]
 * @return
 */
extern SOOL_SoftStarterCustom SOOL_Effector_SoftStarterCustom_Initialize(uint16_t pulse_start, uint16_t pulse_end, uint32_t duration);

// --------------------------------------------------------------

#endif /* INC_SOOL_EFFECTORS_SOFTSTARTER_SOFTSTARTERCUSTOM_H_ */
