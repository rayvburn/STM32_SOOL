/*
 * Scaler.h
 *
 *  Created on: 11.12.2019
 *      Author: user
 */

#ifndef INC_SOOL_MATHS_SCALER_H_
#define INC_SOOL_MATHS_SCALER_H_

#include <stdint.h>

/**
 *
 * @param value_digital
 * @param value_min_physical
 * @param value_max_physical
 * @param value_min_digital
 * @param value_max_digital
 * @return
 * @note `value_digital` correctness is not evaluated
 */
extern int32_t SOOL_Maths_Scale(int32_t value_digital, int32_t value_min_physical, int32_t value_max_physical,
						 	    int32_t value_min_digital, int32_t value_max_digital);

#endif /* INC_SOOL_MATHS_SCALER_H_ */
