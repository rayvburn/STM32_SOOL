/*
 * ArrayUint16.c
 *
 *  Created on: 30.06.2019
 *      Author: user
 */

#include <sool/Memory/Array/ArrayUint16.h>

/* Numeric array (Uint16) */
static void Array_Uint16_Add(SOOL_Array_Uint16 *arr_ptr, uint16_t val);
static void Array_Uint16_Clear(SOOL_Array_Uint16 *arr_ptr);
static void Array_Uint16_Free(SOOL_Array_Uint16 *arr_ptr);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

SOOL_Array_Uint16 SOOL_Memory_Array_Uint16_Init(const size_t capacity) {

	SOOL_Array_Uint16 arr;
	arr._info.capacity = capacity;
	arr._info.total = 0;
	arr._info.add_index = 0;
	arr._data = (uint16_t *)calloc( (size_t)capacity, sizeof(uint16_t) );

	arr.Add = Array_Uint16_Add;
	arr.Clear = Array_Uint16_Clear;
	arr.Free = Array_Uint16_Free;

	return (arr);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
/* Numeric array */
static void Array_Uint16_Add(SOOL_Array_Uint16 *arr_ptr, uint16_t val) {

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

static void Array_Uint16_Clear(SOOL_Array_Uint16 *arr_ptr) {

	for ( size_t i = 0; i < arr_ptr->_info.total; i++ ) {
		arr_ptr->_data[i] = 0;
	}
	arr_ptr->_info.add_index = 0;
	arr_ptr->_info.total = 0;

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void Array_Uint16_Free(SOOL_Array_Uint16 *arr_ptr) {
	arr_ptr->_info.add_index = 0;
	arr_ptr->_info.capacity = 0;
	arr_ptr->_info.total = 0;
	free(arr_ptr->_data);
	arr_ptr->_data = NULL;
}


