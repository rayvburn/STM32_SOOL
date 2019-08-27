/*
 * Array.h
 *
 *  Created on: 26.04.2019
 *      Author: user
 */

#ifndef INC_SOOL_MEMORY_STRING_STRING_H_
#define INC_SOOL_MEMORY_STRING_STRING_H_

#include <sool/Memory/Array/Array_common.h>
#include <stdint.h>
#include <stddef.h> // size_t

// - - - - - - - - - - - - - - - -

struct _SOOL_StringStruct; // forward declaration
typedef struct _SOOL_StringStruct SOOL_String;

struct _SOOL_StringStruct {

	struct _SOOL_Array_Info 	_info;
	char 						*_data;

	void 	(*Add)(SOOL_String *string_ptr, char c);			// acts like a circular buffer
	void	(*Append)(SOOL_String *string_ptr, const char *str); // keeps adding a given string to the end of Array_Char but RESIZES the buffer as it becomes too small to store a given string
	void 	(*Terminate)(SOOL_String *string_ptr);
	void 	(*SetString)(SOOL_String *string_ptr, const char *str); // if str is longer than Array_Char capacity then values from the start of the array will be overwritten
	const char* 	(*GetString)(SOOL_String *string_ptr);
	void 	(*Clear)(SOOL_String *string_ptr);
	uint8_t (*Resize)(SOOL_String *string_ptr, const size_t);
	void	(*Free)(SOOL_String *string_ptr);

};

// - - - - - - - - - - - - - - - -

SOOL_String SOOL_Memory_String_Init(const size_t capacity);

// - - - - - - - - - - - - - - - -

/* Test code: (BEFORE REFACTOR)
ArrayString str1 = SOOL_ArrayString_Init(15);
str1.Add(&str1, 'o');
str1.Add(&str1, 'p');
str1.Add(&str1, 'a');
str1.Add(&str1, 'f');
str1.Add(&str1, 'v');
str1.Add(&str1, '\n');
char* str_out = str1.GetString(&str1);
int a = 2;
a++;
// free(str_out); DEPRECATED
str1.SetString(&str1, "OLEASDaWQ KG");
*/

// - - - - - - - - - - - - - - - -

#endif /* INC_SOOL_MEMORY_STRING_STRING_H_ */
