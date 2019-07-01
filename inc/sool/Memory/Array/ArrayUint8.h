/*
 * ArrayUint8.h
 *
 *  Created on: 30.06.2019
 *      Author: user
 */

#ifndef INC_SOOL_MEMORY_ARRAY_ARRAYUINT8_H_
#define INC_SOOL_MEMORY_ARRAY_ARRAYUINT8_H_

#include <sool/Memory/Array/Array_common.h>
#include <stdint.h>
#include <stddef.h> // size_t

// - - - - - - - - - - - - - -

struct _SOOL_Array_Uint8Struct; // forward declaration
typedef struct _SOOL_Array_Uint8Struct SOOL_Array_Uint8;

struct _SOOL_Array_Uint8Struct {
	struct _SOOL_Array_Info 	_info;
	uint8_t 			*_data;

	void 	(*Add)(SOOL_Array_Uint8 *arr_ptr, uint8_t val);			// acts like a circular buffer
	void 	(*Clear)(SOOL_Array_Uint8 *arr_ptr);
	void	(*Free)(SOOL_Array_Uint8 *arr_ptr);
};

// - - - - - - - - - - - - - -

SOOL_Array_Uint8  SOOL_Memory_Array_Uint8_Init(const size_t capacity);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#endif /* INC_SOOL_MEMORY_ARRAY_ARRAYUINT8_H_ */
