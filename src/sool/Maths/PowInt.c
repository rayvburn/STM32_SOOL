/*
 * PowInt.c
 *
 *  Created on: 20.08.2019
 *      Author: user
 */

#include "sool/Maths/PowInt.h"

int32_t SOOL_Maths_PowInt(int32_t base, int32_t power) {

	if ( power == 0 ) {
		return (1);
	}

	int32_t base_ret = base;
	for ( int32_t i = 1; i < power; i++ ) {
		base_ret *= base;
	}

	return (base_ret);

}
