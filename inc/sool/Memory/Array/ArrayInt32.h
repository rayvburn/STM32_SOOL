/*
 * ArrayInt32.h
 *
 *  Created on: 08.11.2019
 *      Author: user
 */

#ifndef INC_SOOL_MEMORY_ARRAY_ARRAYINT32_H_
#define INC_SOOL_MEMORY_ARRAY_ARRAYINT32_H_

#include <sool/Memory/Array/Array_common.h>
#include <stdint.h>
#include <stddef.h> // size_t

// - - - - - - - - - - - - - -

struct _SOOL_Array_Int32Struct; // forward declaration
typedef struct _SOOL_Array_Int32Struct SOOL_Array_Int32;

struct _SOOL_Array_Int32Struct {
	struct _SOOL_Array_Info 	_info;
	int32_t 					*_data;

	void 	(*Add)(SOOL_Array_Int32 *arr_ptr, int32_t val);			// acts like a circular buffer
	void 	(*Clear)(SOOL_Array_Int32 *arr_ptr);
	void	(*Free)(SOOL_Array_Int32 *arr_ptr);
};

// - - - - - - - - - - - - - -

SOOL_Array_Int32  SOOL_Memory_Array_Int32_Init(const size_t capacity);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


#endif /* INC_SOOL_MEMORY_ARRAY_ARRAYINT32_H_ */
