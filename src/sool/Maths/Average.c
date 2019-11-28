/*
 * Average.c
 *
 *  Created on: 28.11.2019
 *      Author: user
 */

#include "sool/Maths/Average.h"

int64_t SOOL_Maths_Average(uint32_t base_addr, uint8_t size_of_element, uint8_t number) {

	int64_t avg = 0;

	for ( uint8_t i = 0; i < number; i++ ) {
		uint32_t addr = base_addr + (uint32_t)(i * size_of_element);
		// FIXME: need a workaround for this
		// invalid type argument of unary '*' (have 'long unsigned int') dereference
//		avg += *addr; // avg += (int64_t)(*addr);
	}

	return (avg / number);

}
