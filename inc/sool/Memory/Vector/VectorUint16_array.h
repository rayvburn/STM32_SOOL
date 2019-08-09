/*
 * VectorUint16_array.h
 *
 *  Created on: 09.08.2019
 *      Author: user
 */

#ifndef INC_SOOL_MEMORY_VECTOR_VECTORUINT16_ARRAY_H_
#define INC_SOOL_MEMORY_VECTOR_VECTORUINT16_ARRAY_H_

#include "sool/Memory/Vector/VectorUint16.h"

/// @brief Extends array of Vector_Uint16 elements by 1
/// @param v_ptr: pointer to the first element of the array
/// @param size_curr: current size of the array
/// @return 1 if operation successful, 0 otherwise
extern uint8_t SOOL_Memory_Vector_Uint16_Extend(SOOL_Vector_Uint16 *v_ptr, uint8_t size_curr);

#endif /* INC_SOOL_MEMORY_VECTOR_VECTORUINT16_ARRAY_H_ */
