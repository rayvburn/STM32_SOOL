/*
 * AverageInt8.c
 *
 *  Created on: 28.11.2019
 *      Author: user
 */

#include "sool/Maths/Average/AverageInt8.h"

int8_t SOOL_Maths_Average_Int8(const int8_t* arr, uint8_t number) {

	if ( number == 0 ) {
		return (0);
	}

	int64_t avg = 0;

	for ( uint8_t i = 0; i < number; i++ ) {
		avg += *arr++;
	}

	avg /= number;
	return ((int8_t)avg);

}

