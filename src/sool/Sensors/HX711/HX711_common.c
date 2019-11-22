/*
 * HX711_common.c
 *
 *  Created on: 22.11.2019
 *      Author: user
 */

#include "sool/Sensors/HX711/HX711_common.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

uint8_t SOOL_Sensors_HX711_DriftCompensation(int32_t *reading_ptr, struct _SOOL_HX711DriftStruct *drift_ptr) {

	uint8_t status = 2; // `2` means no offsets change

	if (*reading_ptr < 0 && *reading_ptr >= -drift_ptr->drift_threshold) {

		status = 1; // positive offset zeroed
		drift_ptr->offset_pos = 0;
		if (abs(*reading_ptr) > drift_ptr->offset_neg) {
			drift_ptr->offset_neg = abs(*reading_ptr);
		}

	} else if ( *reading_ptr < -drift_ptr->drift_threshold ) { 	// inverted force direction

		// offsets not changed
		*reading_ptr = abs(*reading_ptr);

	} else if ( *reading_ptr <= drift_ptr->drift_threshold ) {

		status = 0; // negative offset zeroed
		drift_ptr->offset_neg = 0;
		drift_ptr->offset_pos = abs(*reading_ptr); 				// `abs()` just in case

	}

	*reading_ptr = abs(*reading_ptr + drift_ptr->offset_neg - drift_ptr->offset_pos);
	return (status);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
