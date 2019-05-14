/*
 * Array.c
 *
 *  Created on: 26.04.2019
 *      Author: user
 */

#include <sool/Memory/Array/ArrayChar.h>
#include <stdlib.h>

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/* Char array */
static void Array_Char_Add(SOOL_Array_Char *string_ptr, char c);
static void Array_Char_SetString(SOOL_Array_Char *string_ptr, const char *str);
//static char* Array_Char_GetString(Array_String *string_ptr); 	 // dynamic allocation (memory needs to be freed after finished processing)
static const char* Array_Char_GetString(SOOL_Array_Char *string_ptr); // return pointer to `data` field
static void Array_Char_Clear(SOOL_Array_Char *string_ptr);
static uint8_t Array_Char_Resize(SOOL_Array_Char *string_ptr, size_t new_capacity);
static void Array_Char_Free(SOOL_Array_Char *string_ptr);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

SOOL_Array_Char SOOL_Memory_Array_Char_Init(const size_t capacity) {

	SOOL_Array_Char string;
	string._info.capacity = capacity;
	string._info.total = 0;
	string._info.add_index = 0;
	string._data = (char *)calloc( (size_t)capacity, sizeof(char) );

	string.Add = Array_Char_Add;
	string.Clear = Array_Char_Clear;
	string.GetString = Array_Char_GetString;
	string.SetString = Array_Char_SetString;
	string.Resize = Array_Char_Resize;
	string.Free = Array_Char_Free;

	return (string);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
/* Char array */
static void Array_Char_Add(SOOL_Array_Char *string_ptr, char c) {

	if ( string_ptr->_info.add_index == string_ptr->_info.capacity ) {
		string_ptr->_info.add_index = 0;
	} else {
		string_ptr->_info.total++;
	}
	string_ptr->_data[string_ptr->_info.add_index] = c;
	string_ptr->_info.add_index++;

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void Array_Char_SetString(SOOL_Array_Char *string_ptr, const char *str) {

	Array_Char_Clear(string_ptr);
	while (*str) {
		Array_Char_Add(string_ptr, *str++); // pointer increment
	}

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/* Dynamic allocation version */
//static char* Array_Char_GetString(Array_String *string_ptr) {
//
//	char *ret = (char *)calloc( (size_t)string_ptr->_info.total, sizeof(char) );
//	if( !ret ) {
//		return NULL;
//	}
//
//	for ( size_t i = 0; i < string_ptr->_info.total; ++i) {
//		ret[i] = string_ptr->_data[i];
//	}
//
//	return (ret);
//
//}

/* Return const pointer to `data` field version */
static const char* Array_Char_GetString(SOOL_Array_Char *string_ptr) {
	return (string_ptr->_data);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void Array_Char_Clear(SOOL_Array_Char *string_ptr) {

	/* NOTE: this needs to be done on the full string length
	 * because when used by DMA there is no way to count number
	 * of items on the fly */
//	for ( size_t i = 0; i < string_ptr->_info.total; i++ ) {
	for ( size_t i = 0; i < string_ptr->_info.capacity; i++ ) {
		string_ptr->_data[i] = 0;
	}
	string_ptr->_info.add_index = 0;
	string_ptr->_info.total = 0;

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t Array_Char_Resize(SOOL_Array_Char *string_ptr, const size_t new_capacity) {

	/* Backup some info */
	uint16_t old_capacity = string_ptr->_info.capacity;
	char *ptr_backup = string_ptr->_data;

	/* Try to reallocate memory */
	string_ptr->_data = realloc( (char*)string_ptr->_data, new_capacity * sizeof(char) );

	/* Check if the reallocation was successful */
	if ( string_ptr->_data != NULL ) {

		/* Clear the new part of an Array (if Array is bigger than before) */
		if ( old_capacity < new_capacity ) {
			for ( size_t i = old_capacity; i < new_capacity; i++) {
				string_ptr->_data[i] = 0;
			}
		}

		/* Update capacity */
		string_ptr->_info.capacity = new_capacity;

		return (1);

	}

	/* If the reallocation failed - restore a previous pointer and return 0 */
	string_ptr->_data = ptr_backup;
	// TODO: some error message
	return (0);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void Array_Char_Free(SOOL_Array_Char *string_ptr) {
	free(string_ptr->_data);
}
