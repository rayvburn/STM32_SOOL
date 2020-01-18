/*
 * AverageInt32.c
 *
 *  Created on: 18.01.2020
 *      Author: user
 */

#include <sool/Maths/Average/AverageInt32.h>

int32_t SOOL_Maths_Average_Int32(const int32_t* arr, uint8_t number) {

	if ( number == 0 ) {
		return (0);
	}

	int64_t avg = 0;

	for ( uint8_t i = 0; i < number; i++ ) {
		avg += *arr++;
	}

	avg /= number;
	return ((int32_t)avg);

}
