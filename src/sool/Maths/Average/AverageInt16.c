/*
 * AverageInt16.c
 *
 *  Created on: 28.11.2019
 *      Author: user
 */

#include "sool/Maths/Average/AverageInt16.h"

int16_t SOOL_Maths_Average_Int16(const int16_t* arr, uint8_t number) {

	if ( number == 0 ) {
		return (0);
	}

	int64_t avg = 0;

	for ( uint8_t i = 0; i < number; i++ ) {
		avg += *arr++;
	}

	avg /= number;
	return ((int16_t)avg);

}



