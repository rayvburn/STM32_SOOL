/*
 * AverageArrayU16.c
 *
 *  Created on: 22.08.2019
 *      Author: user
 */

#include "sool/Maths/Average/AverageArrayU16.h"

uint16_t SOOL_Maths_AverageU16(const SOOL_Array_Uint16 *arr_ptr) {

	uint16_t avg = 0;
	for ( uint32_t i = 0; i < arr_ptr->_info.total; i++ ) {
		avg += arr_ptr->_data[i];
	}
	avg /= arr_ptr->_info.total;
	return (avg);

}
