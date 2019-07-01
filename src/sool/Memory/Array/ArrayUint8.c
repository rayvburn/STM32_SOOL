/*
 * ArrayUint8.c
 *
 *  Created on: 30.06.2019
 *      Author: user
 */

#include <sool/Memory/Array/ArrayUint8.h>

/* Numeric array (uint8_t) */
static void Array_Uint8_Add(SOOL_Array_Uint8 *arr_ptr, uint8_t val);
static void Array_Uint8_Clear(SOOL_Array_Uint8 *arr_ptr);
static void Array_Uint8_Free(SOOL_Array_Uint8 *arr_ptr);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

SOOL_Array_Uint8 SOOL_Memory_Array_Uint8_Init(const size_t capacity) {

	SOOL_Array_Uint8 arr;
	arr._info.capacity = capacity;
	arr._info.total = 0;
	arr._info.add_index = 0;
	arr._data = (uint8_t *)calloc( (size_t)capacity, sizeof(uint8_t) );

	arr.Add = Array_Uint8_Add;
	arr.Clear = Array_Uint8_Clear;
	arr.Free = Array_Uint8_Free;

	return (arr);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
/* Numeric array */
static void Array_Uint8_Add(SOOL_Array_Uint8 *arr_ptr, uint8_t val) {

	if ( arr_ptr->_info.add_index == arr_ptr->_info.capacity ) {
		arr_ptr->_info.add_index = 0;
	} else {
		arr_ptr->_info.total++;
	}
	arr_ptr->_data[arr_ptr->_info.add_index] = val;
	arr_ptr->_info.add_index++;

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void Array_Uint8_Clear(SOOL_Array_Uint8 *arr_ptr) {

	for ( size_t i = 0; i < arr_ptr->_info.total; i++ ) {
		arr_ptr->_data[i] = 0;
	}
	arr_ptr->_info.add_index = 0;
	arr_ptr->_info.total = 0;

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void Array_Uint8_Free(SOOL_Array_Uint8 *arr_ptr) {
	free(arr_ptr->_data);
}



