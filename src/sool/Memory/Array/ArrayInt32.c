/*
 * ArrayInt32.c
 *
 *  Created on: 08.11.2019
 *      Author: user
 */

#include "sool/Memory/Array/ArrayInt32.h"

/* Numeric array (Int32) */
static void Array_Int32_Add(SOOL_Array_Int32 *arr_ptr, int32_t val);
static void Array_Int32_Clear(SOOL_Array_Int32 *arr_ptr);
static void Array_Int32_Free(SOOL_Array_Int32 *arr_ptr);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

SOOL_Array_Int32 SOOL_Memory_Array_Int32_Init(const size_t capacity) {

	SOOL_Array_Int32 arr;
	arr._info.capacity = capacity;
	arr._info.total = 0;
	arr._info.add_index = 0;
	arr._data = (int32_t *)calloc( (size_t)capacity, sizeof(int32_t) );

	arr.Add = Array_Int32_Add;
	arr.Clear = Array_Int32_Clear;
	arr.Free = Array_Int32_Free;

	return (arr);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
/* Numeric array */
static void Array_Int32_Add(SOOL_Array_Int32 *arr_ptr, int32_t val) {

	if ( arr_ptr->_info.add_index == arr_ptr->_info.capacity ) {
		arr_ptr->_info.add_index = 0;
	} else {
		if (++arr_ptr->_info.total > arr_ptr->_info.capacity ) {
			arr_ptr->_info.total = arr_ptr->_info.capacity;
		}
	}
	arr_ptr->_data[arr_ptr->_info.add_index] = val;
	arr_ptr->_info.add_index++;

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void Array_Int32_Clear(SOOL_Array_Int32 *arr_ptr) {

	for ( size_t i = 0; i < arr_ptr->_info.total; i++ ) {
		arr_ptr->_data[i] = 0;
	}
	arr_ptr->_info.add_index = 0;
	arr_ptr->_info.total = 0;

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void Array_Int32_Free(SOOL_Array_Int32 *arr_ptr) {
	arr_ptr->_info.add_index = 0;
	arr_ptr->_info.capacity = 0;
	arr_ptr->_info.total = 0;
	free(arr_ptr->_data);
	arr_ptr->_data = NULL;
}


