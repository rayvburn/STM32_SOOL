/*
 * Filters.h
 *
 *  Created on: 20.10.2018
 *      Author: user
 */

#ifndef FILTERS_H_
#define FILTERS_H_

#include <stdint.h>
#include <string.h>		// memset()

extern void 	Filter_ShiftValues	(int16_t new, int16_t *arr, uint8_t arr_dim);
extern int16_t 	Filter_GetAvg		(const int16_t *arr, uint8_t arr_dim);

// NON-CONST IS WRONG HERE BECAUSE OF ELEMENTS SWAPPING! IF IT WILL BE A RUNNING MEDIAN THEN IT WILL FAIL
extern int16_t 	Filter_GetMedian	(int16_t *arr, uint8_t arr_dim);
// NON-CONST IS WRONG HERE BECAUSE OF ELEMENTS SWAPPING! IF IT WILL BE A RUNNING MEDIAN THEN IT WILL FAIL

extern void 	Filter_ClearArray	(int16_t *arr);

#endif /* FILTERS_H_ */
