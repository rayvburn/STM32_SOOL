/*
 * PID_common.h
 *
 *  Created on: 19.06.2020
 *      Author: user
 */

#ifndef INC_SOOL_EFFECTORS_PID_PID_COMMON_H_
#define INC_SOOL_EFFECTORS_PID_PID_COMMON_H_

#include <stdint.h>

enum PIDMode {
	PID_MODE_MANUAL = 0u,
	PID_MODE_AUTOMATIC = 1u
};

enum PIDDirection {
	PID_DIRECTION_DIRECT = 0u,
	PID_DIRECTION_REVERSE = 1u,
};

enum PIDProportional {
	PID_P_ON_M = 0u,
	PID_P_ON_E = 1u,
};

struct _SOOL_PID_ConfigStruct {
	/// @ref enum PIDDirection
	uint8_t controller_direction;
	uint32_t sample_time;
	uint8_t in_auto;
	uint32_t last_time;
	// @ref enum PIDProportional
	// Adds Proportional on Measurement
	uint8_t p_on;
	/// Adds Proportional on Error
	uint8_t p_on_e;
};

#endif /* INC_SOOL_EFFECTORS_PID_PID_COMMON_H_ */
