/*
 * Time_common.c
 *
 *  Created on: 24.05.2019
 *      Author: user
 */

#include <sool/Workflow/Time_common.h>
#include <limits.h>

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

uint32_t SOOL_Workflow_Common_ComputeTimeDifference(uint32_t time_start, uint32_t time_end) {

	uint32_t time_diff = 0;

	/* Check if a timer's overflow has occurred */
	if ( time_end < time_start ) {

		/* Overflow has occurred */
		time_diff = (UINT32_MAX - time_start) + time_end;
		//time_diff = (4294967295U - time_start) + time_end;

	} else {

		/* No overflow */
		time_diff = time_end - time_start;

	}

	return (time_diff);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
