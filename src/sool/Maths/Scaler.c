/*
 * Scaler.c
 *
 *  Created on: 11.12.2019
 *      Author: user
 */

#include <sool/Maths/Scaler.h>

int32_t SOOL_Maths_Scale(int32_t value_digital, int32_t value_min_physical, int32_t value_max_physical,
		int32_t value_min_digital, int32_t value_max_digital)
{

	//								 * physical range *                            * digital range *                * digital min *
	return ( (((value_digital)*(value_max_physical - value_min_physical))/(value_max_digital - value_min_digital)) + value_min_physical );
}
