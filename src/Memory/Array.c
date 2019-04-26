/*
 * Array.c
 *
 *  Created on: 26.04.2019
 *      Author: user
 */

#include <include/Memory/Array.h>
#include <stdlib.h>

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// numeric array
static void ArrayInt16_Add(ArrayInt16 *arr_ptr, int16_t val);
static void ArrayInt16_Clear(ArrayInt16 *arr_ptr);
static void ArrayInt16_Free(ArrayInt16 *arr_ptr);

// string array
static void ArrayString_AddChar(ArrayString *string_ptr, char c);
static void ArrayString_SetString(ArrayString *string_ptr, const char *str);
static char* ArrayString_GetString(ArrayString *string_ptr);
static void ArrayString_Clear(ArrayString *string_ptr);
static void ArrayString_Free(ArrayString *string_ptr);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


ArrayInt16 SOOL_ArrayInt16_Init(const uint16_t capacity) {

	ArrayInt16 arr;
	arr.info.capacity = capacity;
	arr.info.total = 0;
	arr.info.add_index = 0;
	arr.items = (int16_t *)calloc( (size_t)capacity, sizeof(int16_t) );

	arr.Add = ArrayInt16_Add;
	arr.Clear = ArrayInt16_Clear;
	arr.Free = ArrayInt16_Free;

	return (arr);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

ArrayString SOOL_ArrayString_Init(const uint16_t capacity) {

	ArrayString string;
	string.info.capacity = capacity;
	string.info.total = 0;
	string.info.add_index = 0;
	string.items = (char *)calloc( (size_t)capacity, sizeof(char) );

	string.AddChar = ArrayString_AddChar;
	string.Clear = ArrayString_Clear;
	string.GetString = ArrayString_GetString;
	string.SetString = ArrayString_SetString;
	string.Free = ArrayString_Free;

	return (string);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/* Numeric array */
static void ArrayInt16_Add(ArrayInt16 *arr_ptr, int16_t val) {

	if ( arr_ptr->info.add_index == arr_ptr->info.capacity ) {
		arr_ptr->info.add_index = 0;
	} else {
		arr_ptr->info.total++;
	}
	arr_ptr->items[arr_ptr->info.add_index] = val;
	arr_ptr->info.add_index++;

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void ArrayInt16_Clear(ArrayInt16 *arr_ptr) {

	for ( uint16_t i = 0; i < arr_ptr->info.total; i++ ) {
		arr_ptr->items[i] = 0;
	}
	arr_ptr->info.add_index = 0;
	arr_ptr->info.total = 0;

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void ArrayInt16_Free(ArrayInt16 *arr_ptr) {
	free(arr_ptr);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
/* String array */
static void ArrayString_AddChar(ArrayString *string_ptr, char c) {

	if ( string_ptr->info.add_index == string_ptr->info.capacity ) {
		string_ptr->info.add_index = 0;
	} else {
		string_ptr->info.total++;
	}
	string_ptr->items[string_ptr->info.add_index] = c;
	string_ptr->info.add_index++;

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void ArrayString_SetString(ArrayString *string_ptr, const char *str) {

	ArrayString_Clear(string_ptr);
	while (*str) {
		ArrayString_AddChar(string_ptr, *str++); // pointer increment
	}

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static char* ArrayString_GetString(ArrayString *string_ptr) {

	char *ret = (char *)calloc( (size_t)string_ptr->info.total, sizeof(char) );
	if( !ret ) {
		return NULL;
	}

	for ( uint16_t i = 0; i < string_ptr->info.total; ++i) {
		ret[i] = string_ptr->items[i];
	}

	return (ret);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void ArrayString_Clear(ArrayString *string_ptr) {

	for ( uint16_t i = 0; i < string_ptr->info.total; i++ ) {
		string_ptr->items[i] = 0;
	}
	string_ptr->info.add_index = 0;
	string_ptr->info.total = 0;

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void ArrayString_Free(ArrayString *string_ptr) {
	free(string_ptr);
}
