/*
 * AverageInt8.h
 *
 *  Created on: 28.11.2019
 *      Author: user
 */

#ifndef INC_SOOL_MATHS_AVERAGE_AVERAGEINT8_H_
#define INC_SOOL_MATHS_AVERAGE_AVERAGEINT8_H_

#include "stdint.h"

/**
 * @brief Calculates average value of array elements
 * @param arr: a pointer to non-mutable int8_t data
 * @param number: how many elements should be averaged (value mustn't be bigger
 * @return
 */
extern int8_t SOOL_Maths_Average_Int8(const int8_t* arr, uint8_t number);

#endif /* INC_SOOL_MATHS_AVERAGE_AVERAGEINT8_H_ */
