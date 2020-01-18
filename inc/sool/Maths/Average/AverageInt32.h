/*
 * AverageInt32.h
 *
 *  Created on: 18.01.2020
 *      Author: user
 */

#ifndef INC_SOOL_MATHS_AVERAGE_AVERAGEINT32_H_
#define INC_SOOL_MATHS_AVERAGE_AVERAGEINT32_H_

#include "stdint.h"

/**
 * @brief Calculates average value of array elements
 * @param arr: a pointer to non-mutable int32_t data
 * @param number: how many elements should be averaged
 * @return
 */
extern int32_t SOOL_Maths_Average_Int32(const int32_t* arr, uint8_t number);

#endif /* INC_SOOL_MATHS_AVERAGE_AVERAGEINT32_H_ */
