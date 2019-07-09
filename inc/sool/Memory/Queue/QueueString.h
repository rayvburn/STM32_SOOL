/*
 * QueueString.h
 *
 *  Created on: 08.07.2019
 *      Author: user
 */

#ifndef INC_SOOL_MEMORY_QUEUE_QUEUESTRING_H_
#define INC_SOOL_MEMORY_QUEUE_QUEUESTRING_H_

#include <stdint.h>
#include "sool/Memory/String/String.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

struct _SOOL_QueueStringSetupStruct {
	uint8_t size;
	uint8_t max_size;
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

//struct _SOOL_QueueStringStateStruct {
//	uint8_t front_idx;
//	uint8_t back_idx;
//};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/* Forward declaration */
struct _SOOL_QueueStringStruct;
typedef struct _SOOL_QueueStringStruct SOOL_Queue_String;

/**
 * Structure defines front and back elements positions in advance. Front element is placed at 0 position
 * whereas back element at (size - 1). Each Pop() invocation shifts values so old front element is overwritten
 * by the next one in the queue. What's more Pop() reallocates memory to utilize it efficiently
 * (uses as much as currently needs).
 */
struct _SOOL_QueueStringStruct {

	struct _SOOL_QueueStringSetupStruct	_setup;
//	struct _SOOL_QueueStringStateStruct _state;
	SOOL_String*						_data;

	uint8_t 	(*IsEmpty)(const SOOL_Queue_String*);
	uint8_t 	(*IsFull)(const SOOL_Queue_String*);
	void 		(*Pop)(SOOL_Queue_String*);
	uint8_t 	(*PushString)(SOOL_Queue_String*, SOOL_String); // exceptionally as structure, do not inflate it over different types
	uint8_t 	(*Push)(SOOL_Queue_String*, const char*);
	SOOL_String (*GetFront)(const SOOL_Queue_String*);
//	SOOL_String (*GetBack)(const SOOL_Queue_String*);
//	uint8_t 	(*GetSize)(const SOOL_Queue_String*);

};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

extern SOOL_Queue_String SOOL_Memory_Queue_String_Init(uint8_t max_size);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// TEST CODE
/*
	SOOL_Queue_String test = SOOL_Memory_Queue_String_Init(8);
	uint8_t empty = test.IsEmpty(&test);
	uint8_t full = test.IsFull(&test);
	SOOL_String str_fill = SOOL_Memory_String_Init(7);
	str_fill.SetString(&str_fill, "09qwioi");
	test.PushString(&test, str_fill);
	SOOL_String test_str;
	test.Push(&test, "afdsada");
	test_str = test.GetFront(&test);
	test.Push(&test, "qwertyu");
	test_str = test.GetFront(&test);
	test.Push(&test, "poiuytr");
	test_str = test.GetFront(&test);
	test.Push(&test, "bgtrfv");
	test_str = test.GetFront(&test);
	test.Push(&test, "plmnjiu");
	test_str = test.GetFront(&test);
	test.Push(&test, "uhbnji");
	test_str = test.GetFront(&test);
	test.Push(&test, "q8w7re6");
	test_str = test.GetFront(&test);
	test.Push(&test, "8qfvsoz5");
	test_str = test.GetFront(&test);
	test.Push(&test, "cyhwqkn");
	test_str = test.GetFront(&test);
	test.Pop(&test);
	test_str = test.GetFront(&test);
 */

#endif /* INC_SOOL_MEMORY_QUEUE_QUEUESTRING_H_ */
