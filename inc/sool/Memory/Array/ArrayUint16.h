/*
 * ArrayUint16.h
 *
 *  Created on: 30.06.2019
 *      Author: user
 */

#ifndef INC_SOOL_MEMORY_ARRAY_ARRAYUINT16_H_
#define INC_SOOL_MEMORY_ARRAY_ARRAYUINT16_H_

#include <sool/Memory/Array/Array_common.h>
#include <stdint.h>
#include <stddef.h> // size_t

// - - - - - - - - - - - - - -

struct _SOOL_Array_Uint16Struct; // forward declaration
typedef struct _SOOL_Array_Uint16Struct SOOL_Array_Uint16;

struct _SOOL_Array_Uint16Struct {
	struct _SOOL_Array_Info 	_info;
	uint16_t 			*_data;

	void 	(*Add)(SOOL_Array_Uint16 *arr_ptr, uint16_t val);			// acts like a circular buffer
	void 	(*Clear)(SOOL_Array_Uint16 *arr_ptr);
	void	(*Free)(SOOL_Array_Uint16 *arr_ptr);
};

// - - - - - - - - - - - - - -

SOOL_Array_Uint16  SOOL_Memory_Array_Uint16_Init(const size_t capacity);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#endif /* INC_SOOL_MEMORY_ARRAY_ARRAYUINT16_H_ */
