/*
 * HX711_common.h
 *
 *  Created on: 22.11.2019
 *      Author: user
 */

#ifndef INC_SOOL_SENSORS_HX711_HX711_COMMON_H_
#define INC_SOOL_SENSORS_HX711_HX711_COMMON_H_

#include "stdint.h"

struct _SOOL_HX711DriftStruct {
	uint8_t drift_threshold;
	uint8_t offset_neg;
	uint8_t offset_pos;
};

extern uint8_t SOOL_Sensors_HX711_DriftCompensation(int32_t *reading_ptr, struct _SOOL_HX711DriftStruct *drift_ptr);

#endif /* INC_SOOL_SENSORS_HX711_HX711_COMMON_H_ */
