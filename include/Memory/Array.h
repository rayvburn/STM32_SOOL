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
} ArrayInfo;

// - - - - - - - - - - - - - - - -

struct ArrayInt16Struct; // forward declaration
typedef struct ArrayInt16Struct ArrayInt16;

struct ArrayInt16Struct {
	ArrayInfo info;
	int16_t *items;

	void 	(*Add)(ArrayInt16 *arr_ptr, int16_t val);			// acts like a circular buffer
	void 	(*Clear)(ArrayInt16 *arr_ptr);
	void	(*Free)(ArrayInt16 *arr_ptr);
};

// - - - - - - - - - - - - - - - -

struct ArrayStringStruct; // forward declaration
typedef struct ArrayStringStruct ArrayString;

struct ArrayStringStruct {

	ArrayInfo info;
	char *items;

	void 	(*AddChar)(ArrayString *string_ptr, char c);			// acts like a circular buffer
	void 	(*SetString)(ArrayString *string_ptr, const char *str);
	char* 	(*GetString)(ArrayString *string_ptr); /// IMPORTANT: free memory after finished processing!
	void 	(*Clear)(ArrayString *string_ptr);
	void	(*Free)(ArrayString *string_ptr);

};

// - - - - - - - - - - - - - - - -

ArrayInt16 SOOL_ArrayInt16_Init(const uint16_t capacity);
ArrayString SOOL_ArrayString_Init(const uint16_t capacity);

// - - - - - - - - - - - - - - - -

/* Test code:
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
