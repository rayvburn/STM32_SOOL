/*
 * HX711_common.c
 *
 *  Created on: 22.11.2019
 *      Author: user
 */

#include "sool/Sensors/HX711/HX711_common.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

uint8_t SOOL_Sensors_HX711_DriftCompensation(int32_t *reading_ptr, struct _SOOL_HX711DriftStruct *drift_ptr) {

	/* Status: 0 indicates that the new value is within the drift threshold range (idle);
	 * Status: 1 indicates that the load cell is probably in active mode (weighting) */
	uint8_t status = 0;

	/* Decide whether to update idle readings array */
	/* Performs filtering of values, ignores those which stick out from
	 * the initial average (i.e. in idle). Note that initial average may
	 * differ a lot from the average which exists after high tensions
	 * were applied to the sensor.
	 * Slowly changing values are added to the array. */
	int16_t avg = SOOL_Maths_Average_Int16(drift_ptr->samples._data, drift_ptr->samples._info.total);
//	if ( abs(*reading_ptr) <= abs(2 * avg) ) {
//	if ( abs(*reading_ptr - avg) <= 2 * abs(avg) ) {
	if ( abs(*reading_ptr - avg) <= abs(drift_ptr->drift_threshold) ) {
		// assuming that readings will not change dramatically between 2 consecutive samples
		// thus the newest sample is not calculated in during further computations
		// for the current iteration
		drift_ptr->samples.Add(&drift_ptr->samples, *reading_ptr);
		status = 1;
	}

	/* Count in offsets */
	*reading_ptr -= avg;

	return (status);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
