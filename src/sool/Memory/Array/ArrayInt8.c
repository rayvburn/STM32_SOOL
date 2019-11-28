/*
 * ArrayInt8.c
 *
 *  Created on: 28.11.2019
 *      Author: user
 */

#include "sool/Memory/Array/ArrayInt8.h"

/* Numeric array (Int16) */
static void Array_Int8_Add(SOOL_Array_Int8 *arr_ptr, int8_t val);
static void Array_Int8_Clear(SOOL_Array_Int8 *arr_ptr);
static void Array_Int8_Free(SOOL_Array_Int8 *arr_ptr);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

SOOL_Array_Int8 SOOL_Memory_Array_Int8_Init(const size_t capacity) {

	SOOL_Array_Int8 arr;
	arr._info.capacity = capacity;
	arr._info.total = 0;
	arr._info.add_index = 0;
	arr._data = (int8_t *)calloc( (size_t)capacity, sizeof(int8_t) );

	arr.Add = Array_Int8_Add;
	arr.Clear = Array_Int8_Clear;
	arr.Free = Array_Int8_Free;

	return (arr);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
/* Numeric array */
static void Array_Int8_Add(SOOL_Array_Int8 *arr_ptr, int8_t val) {

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

static void Array_Int8_Clear(SOOL_Array_Int8 *arr_ptr) {

	for ( size_t i = 0; i < arr_ptr->_info.total; i++ ) {
		arr_ptr->_data[i] = 0;
	}
	arr_ptr->_info.add_index = 0;
	arr_ptr->_info.total = 0;

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void Array_Int8_Free(SOOL_Array_Int8 *arr_ptr) {
	arr_ptr->_info.add_index = 0;
	arr_ptr->_info.capacity = 0;
	arr_ptr->_info.total = 0;
	free(arr_ptr->_data);
	arr_ptr->_data = NULL;
}
