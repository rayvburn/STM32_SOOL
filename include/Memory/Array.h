/*
 * Array.h
 *
 *  Created on: 26.04.2019
 *      Author: user
 */

#ifndef INCLUDE_MEMORY_ARRAY_H_
#define INCLUDE_MEMORY_ARRAY_H_

#include <stdint.h>

// - - - - - - - - - - - - - - - -

typedef struct {
	uint16_t total;
	uint16_t capacity;
	uint16_t add_index;	// stores index of an element to-be-added
} Array_Info;

// - - - - - - - - - - - - - - - -

struct Array_Int16Struct; // forward declaration
typedef struct Array_Int16Struct Array_Int16;

struct Array_Int16Struct {
	Array_Info info;
	int16_t *data;

	void 	(*Add)(Array_Int16 *arr_ptr, int16_t val);			// acts like a circular buffer
	void 	(*Clear)(Array_Int16 *arr_ptr);
	void	(*Free)(Array_Int16 *arr_ptr);
};

// - - - - - - - - - - - - - - - -

// TODO: ADD null-termination?
struct Array_StringStruct; // forward declaration
typedef struct Array_StringStruct Array_String;

struct Array_StringStruct {

	Array_Info info;
	char *data;

	void 	(*AddChar)(Array_String *string_ptr, char c);			// acts like a circular buffer
	void 	(*SetString)(Array_String *string_ptr, const char *str);
	char* 	(*GetString)(Array_String *string_ptr); /// IMPORTANT: free memory after finished processing!
	void 	(*Clear)(Array_String *string_ptr);
	void	(*Free)(Array_String *string_ptr);

};

// - - - - - - - - - - - - - - - -

Array_Int16  SOOL_Array_Int16_Init(const uint16_t capacity);
Array_String SOOL_Array_String_Init(const uint16_t capacity);

// - - - - - - - - - - - - - - - -

/* Test code: (BEFORE REFACTOR)
ArrayString str1 = SOOL_ArrayString_Init(15);
str1.AddChar(&str1, 'o');
str1.AddChar(&str1, 'p');
str1.AddChar(&str1, 'a');
str1.AddChar(&str1, 'f');
str1.AddChar(&str1, 'v');
str1.AddChar(&str1, '\n');
char* str_out = str1.GetString(&str1);
int a = 2;
a++;
free(str_out);
str1.SetString(&str1, "OLEASDaWQ KG");
*/

// - - - - - - - - - - - - - - - -

#endif /* INCLUDE_MEMORY_ARRAY_H_ */
