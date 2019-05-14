/*
 * Array.h
 *
 *  Created on: 26.04.2019
 *      Author: user
 */

#ifndef INC_SOOL_MEMORY_ARRAY_H_
#define INC_SOOL_MEMORY_ARRAY_H_

#include <sool/Memory/Array/Array_common.h>
#include <stdint.h>
#include <stddef.h> // size_t

// - - - - - - - - - - - - - - - -

struct _SOOL_Array_CharStruct; // forward declaration
typedef struct _SOOL_Array_CharStruct SOOL_Array_Char;

struct _SOOL_Array_CharStruct {

	struct _SOOL_Array_Info 	_info;
	char 						*_data;

	void 	(*Add)(SOOL_Array_Char *string_ptr, char c);			// acts like a circular buffer
	void 	(*SetString)(SOOL_Array_Char *string_ptr, const char *str);
	const char* 	(*GetString)(SOOL_Array_Char *string_ptr);
	void 	(*Clear)(SOOL_Array_Char *string_ptr);
	uint8_t (*Resize)(SOOL_Array_Char *string_ptr, const size_t);
	void	(*Free)(SOOL_Array_Char *string_ptr);

};

// - - - - - - - - - - - - - - - -

SOOL_Array_Char SOOL_Memory_Array_Char_Init(const size_t capacity);

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

#endif /* INC_SOOL_MEMORY_ARRAY_H_ */
