/*
 * ArrayInt8.h
 *
 *  Created on: 28.11.2019
 *      Author: user
 */

#ifndef INC_SOOL_MEMORY_ARRAY_ARRAYINT8_H_
#define INC_SOOL_MEMORY_ARRAY_ARRAYINT8_H_

#include <sool/Memory/Array/Array_common.h>
#include <stdint.h>
#include <stddef.h> // size_t

// - - - - - - - - - - - - - -

struct _SOOL_Array_Int8Struct; // forward declaration
typedef struct _SOOL_Array_Int8Struct SOOL_Array_Int8;

struct _SOOL_Array_Int8Struct {
	struct _SOOL_Array_Info 	_info;
	int8_t 						*_data;

	void 	(*Add)(SOOL_Array_Int8 *arr_ptr, int8_t val);			// acts like a circular buffer
	void 	(*Clear)(SOOL_Array_Int8 *arr_ptr);
	void	(*Free)(SOOL_Array_Int8 *arr_ptr);
};

// - - - - - - - - - - - - - -

SOOL_Array_Int8  SOOL_Memory_Array_Int8_Init(const size_t capacity);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#endif /* INC_SOOL_MEMORY_ARRAY_ARRAYINT8_H_ */
