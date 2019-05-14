/*
 * ArrayInt16.h
 *
 *  Created on: 14.05.2019
 *      Author: user
 */

#ifndef INC_SOOL_MEMORY_ARRAY_ARRAYINT16_H_
#define INC_SOOL_MEMORY_ARRAY_ARRAYINT16_H_

#include <sool/Memory/Array/Array_common.h>
#include <stdint.h>
#include <stddef.h> // size_t

// - - - - - - - - - - - - - -

struct _SOOL_Array_Int16Struct; // forward declaration
typedef struct _SOOL_Array_Int16Struct SOOL_Array_Int16;

struct _SOOL_Array_Int16Struct {
	struct _SOOL_Array_Info 	_info;
	int16_t 			*_data;

	void 	(*Add)(SOOL_Array_Int16 *arr_ptr, int16_t val);			// acts like a circular buffer
	void 	(*Clear)(SOOL_Array_Int16 *arr_ptr);
	void	(*Free)(SOOL_Array_Int16 *arr_ptr);
};

// - - - - - - - - - - - - - -

SOOL_Array_Int16  SOOL_Memory_Array_Int16_Init(const size_t capacity);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#endif /* INC_SOOL_MEMORY_ARRAY_ARRAYINT16_H_ */
