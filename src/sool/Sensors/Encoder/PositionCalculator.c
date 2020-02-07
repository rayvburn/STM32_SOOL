/*
 * PositionCalculator.c
 *
 *  Created on: 05.02.2020
 *      Author: user
 */

#include <sool/Sensors/Encoder/PositionCalculator.h>

// ------------------------------------------------------------------------------------------------

uint16_t SOOL_Sensor_Encoder_PositionCalculator_ComputeGoal(uint16_t pos_current, int32_t pos_diff) {

	uint32_t val = pos_current + pos_diff;

	/* Check if a timer's overflow has occurred */
	if ( pos_diff > 0xFFFF ) {

		/* Error (see function documentation) */
		return (pos_current);

	} else if ( val > 0xFFFF ) {

		/* Overflow has occurred */
		val = val - 0xFFFF;

	}

	return (val);

}

// ------------------------------------------------------------------------------------------------

uint16_t SOOL_Sensor_Encoder_PositionCalculator_ComputeDiff(uint16_t pos_current, uint16_t pos_goal, uint8_t upcounting) {

	// NOTE: timers are 16-bit, thus 0xFFFF

	// equality
	if ( pos_current == pos_goal ) {
		return (0);
	}

	uint16_t value = 0;

	// evaluate counting direction
	switch (upcounting) {

		// down-counting
		case(0):
			if ( pos_goal > pos_current ) {
				value = pos_current + (0xFFFF - pos_goal);
			} else {  //  pos_goal < pos_current
				value = pos_current - pos_goal;
			}
			break;

		// up-counting
		case(1):
			if ( pos_goal > pos_current ) {
				value = pos_goal - pos_current;
			} else {  //  pos_goal < pos_current
				value = (0xFFFF - pos_current) + pos_goal;
			}
			break;

		default:
			break;

	}

	return (value);

}

// ------------------------------------------------------------------------------------------------
