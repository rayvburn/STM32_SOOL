/*
 * Array.c
 *
 *  Created on: 26.04.2019
 *      Author: user
 */

#include <include/Memory/Array.h>
#include <stdlib.h>

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// numeric array (Int16)
static void Array_Int16_Add(Array_Int16 *arr_ptr, int16_t val);
static void Array_Int16_Clear(Array_Int16 *arr_ptr);
static void Array_Int16_Free(Array_Int16 *arr_ptr);

// string array
static void Array_String_AddChar(Array_String *string_ptr, char c);
static void Array_String_SetString(Array_String *string_ptr, const char *str);
static char* Array_String_GetString(Array_String *string_ptr);
static void Array_String_Clear(Array_String *string_ptr);
static void Array_String_Free(Array_String *string_ptr);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


Array_Int16 SOOL_Array_Int16_Init(const uint16_t capacity) {

	Array_Int16 arr;
	arr.info.capacity = capacity;
	arr.info.total = 0;
	arr.info.add_index = 0;
	arr.data = (int16_t *)calloc( (size_t)capacity, sizeof(int16_t) );

	arr.Add = Array_Int16_Add;
	arr.Clear = Array_Int16_Clear;
	arr.Free = Array_Int16_Free;

	return (arr);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

Array_String SOOL_Array_String_Init(const uint16_t capacity) {

	Array_String string;
	string.info.capacity = capacity;
	string.info.total = 0;
	string.info.add_index = 0;
	string.data = (char *)calloc( (size_t)capacity, sizeof(char) );

	string.AddChar = Array_String_AddChar;
	string.Clear = Array_String_Clear;
	string.GetString = Array_String_GetString;
	string.SetString = Array_String_SetString;
	string.Free = Array_String_Free;

	return (string);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/* Numeric array */
static void Array_Int16_Add(Array_Int16 *arr_ptr, int16_t val) {

	if ( arr_ptr->info.add_index == arr_ptr->info.capacity ) {
		arr_ptr->info.add_index = 0;
	} else {
		arr_ptr->info.total++;
	}
	arr_ptr->data[arr_ptr->info.add_index] = val;
	arr_ptr->info.add_index++;

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void Array_Int16_Clear(Array_Int16 *arr_ptr) {

	for ( uint16_t i = 0; i < arr_ptr->info.total; i++ ) {
		arr_ptr->data[i] = 0;
	}
	arr_ptr->info.add_index = 0;
	arr_ptr->info.total = 0;

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void Array_Int16_Free(Array_Int16 *arr_ptr) {
	free(arr_ptr);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
/* String array */
static void Array_String_AddChar(Array_String *string_ptr, char c) {

	if ( string_ptr->info.add_index == string_ptr->info.capacity ) {
		string_ptr->info.add_index = 0;
	} else {
		string_ptr->info.total++;
	}
	string_ptr->data[string_ptr->info.add_index] = c;
	string_ptr->info.add_index++;

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void Array_String_SetString(Array_String *string_ptr, const char *str) {

	Array_String_Clear(string_ptr);
	while (*str) {
		Array_String_AddChar(string_ptr, *str++); // pointer increment
	}

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static char* Array_String_GetString(Array_String *string_ptr) {

	char *ret = (char *)calloc( (size_t)string_ptr->info.total, sizeof(char) );
	if( !ret ) {
		return NULL;
	}

	for ( uint16_t i = 0; i < string_ptr->info.total; ++i) {
		ret[i] = string_ptr->data[i];
	}

	return (ret);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void Array_String_Clear(Array_String *string_ptr) {

	for ( uint16_t i = 0; i < string_ptr->info.total; i++ ) {
		string_ptr->data[i] = 0;
	}
	string_ptr->info.add_index = 0;
	string_ptr->info.total = 0;

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void Array_String_Free(Array_String *string_ptr) {
	free(string_ptr);
}
