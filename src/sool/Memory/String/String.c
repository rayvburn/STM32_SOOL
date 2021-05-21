/*
 * Array.c
 *
 *  Created on: 26.04.2019
 *      Author: user
 */

#include <sool/Memory/String/String.h>
#include <stdlib.h>

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/* Char array */
static void SOOL_String_Add(SOOL_String *string_ptr, char c);
static void SOOL_String_Append(SOOL_String *string_ptr, const char *str);
static void SOOL_String_Terminate(SOOL_String *string_ptr);
static void SOOL_String_SetString(SOOL_String *string_ptr, const char *str);
//static char* SOOL_String_GetString(SOOL_String *string_ptr); 	 // dynamic allocation (memory needs to be freed after finished processing)
static const char* SOOL_String_GetString(SOOL_String *string_ptr); // return pointer to `data` field
static void SOOL_String_Clear(SOOL_String *string_ptr);
static uint8_t SOOL_String_Resize(SOOL_String *string_ptr, size_t new_capacity);
static void SOOL_String_Free(SOOL_String *string_ptr);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

SOOL_String SOOL_Memory_String_Init(const size_t capacity) {

	SOOL_String string;
	string._info.capacity = capacity;
	string._info.total = 0;
	string._info.add_index = 0;
	string._data = (char *)calloc( (size_t)capacity, sizeof(char) );

	string.Add = SOOL_String_Add;
	string.Append = SOOL_String_Append;
	string.Terminate = SOOL_String_Terminate;
	string.Clear = SOOL_String_Clear;
	string.GetString = SOOL_String_GetString;
	string.SetString = SOOL_String_SetString;
	string.Resize = SOOL_String_Resize;
	string.Free = SOOL_String_Free;

	return (string);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
/* Char array */
static void SOOL_String_Add(SOOL_String *string_ptr, char c) {

	if ( string_ptr->_info.add_index == string_ptr->_info.capacity ) {
		string_ptr->_info.add_index = 0;
	} else {
		if (++string_ptr->_info.total > string_ptr->_info.capacity ) {
			string_ptr->_info.total = string_ptr->_info.capacity;
		}
	}
	string_ptr->_data[string_ptr->_info.add_index] = c;
	string_ptr->_info.add_index++;

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void SOOL_String_Append(SOOL_String *string_ptr, const char *str) {

	/* Iterate over whole string (each ends with a 0 */
	while (*str) {

		/* Check whether the string should be resized due to too small buffer */
		if ( string_ptr->_info.capacity <= string_ptr->_info.total ) { 		// `<=` instead of `==` and `if` instead of `while` here just in case, SOOL_Array_Char acts as a circular buffer when using public functions only (not modifying its contents
			SOOL_String_Resize(string_ptr, string_ptr->_info.capacity + 1);
		}

		/* We're safe to add another character to the buffer */
		SOOL_String_Add(string_ptr, *str++); // pointer increment
	}

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void SOOL_String_Terminate(SOOL_String *string_ptr) {

	/* Check whether the string should be resized due to too small buffer */
	if ( string_ptr->_info.capacity <= string_ptr->_info.total ) { 		// `<=` instead of `==` and `if` instead of `while` here just in case, SOOL_Array_Char acts as a circular buffer when using public functions only (not modifying its contents
		SOOL_String_Resize(string_ptr, string_ptr->_info.capacity + 1);
	}

	/* We're safe to add another character to the buffer */
	SOOL_String_Add(string_ptr, '\0');

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void SOOL_String_SetString(SOOL_String *string_ptr, const char *str) {

	SOOL_String_Clear(string_ptr);
	while (*str) {
		SOOL_String_Add(string_ptr, *str++); // pointer increment
	}

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/* Dynamic allocation version */
//static char* SOOL_String_GetString(Array_String *string_ptr) {
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
static const char* SOOL_String_GetString(SOOL_String *string_ptr) {
	return (string_ptr->_data);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void SOOL_String_Clear(SOOL_String *string_ptr) {

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

static uint8_t SOOL_String_Resize(SOOL_String *string_ptr, const size_t new_capacity) {
	if (string_ptr == NULL) {
		return 0;
	}

	if (string_ptr->_info.capacity == new_capacity) {
		return 0;
	}

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

static void SOOL_String_Free(SOOL_String *string_ptr) {
	string_ptr->_info.add_index = 0;
	string_ptr->_info.capacity = 0;
	string_ptr->_info.total = 0;
	free(string_ptr->_data);
	string_ptr->_data = NULL;
}
