/*
 * QueueString.c
 *
 *  Created on: 08.07.2019
 *      Author: user
 */

#include "sool/Memory/Queue/QueueString.h"
#include <string.h>

static uint8_t SOOL_QueueString_IsEmpty(const SOOL_Queue_String *q_ptr);
static uint8_t SOOL_QueueString_IsFull(const SOOL_Queue_String *q_ptr);
static void SOOL_QueueString_Pop(SOOL_Queue_String *q_ptr);
static uint8_t SOOL_QueueString_PushString(SOOL_Queue_String *q_ptr, SOOL_String str);
static uint8_t SOOL_QueueString_Push(SOOL_Queue_String *q_ptr, const char *str);
static SOOL_String SOOL_QueueString_GetFront(const SOOL_Queue_String *q_ptr);
//static SOOL_String SOOL_QueueString_GetBack(const SOOL_Queue_String *q_ptr);
//static uint8_t SOOL_QueueString_GetSize(const SOOL_Queue_String *q_ptr);

// helper
static uint8_t SOOL_QueueString_Resize(SOOL_Queue_String *q_ptr, const size_t new_capacity);

// queue acts as a shift register (allows easier reallocation to save memory, queue always creates
// a solid block of memory)
static void SOOL_QueueString_Shift(SOOL_Queue_String *q_ptr);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

SOOL_Queue_String SOOL_Memory_Queue_String_Init(uint8_t max_size) {

	/* New instance */
	SOOL_Queue_String q;

	/* Initialize setup structure fields */
	q._setup.size = 0;
	q._setup.max_size = max_size;

	/* Update function pointers */
	q.GetFront = SOOL_QueueString_GetFront;
	q.IsEmpty = SOOL_QueueString_IsEmpty;
	q.IsFull = SOOL_QueueString_IsFull;
	q.Pop = SOOL_QueueString_Pop;
	q.Push = SOOL_QueueString_Push;
	q.PushString = SOOL_QueueString_PushString;

	return (q);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t SOOL_QueueString_IsEmpty(const SOOL_Queue_String *q_ptr) {

	if ( q_ptr->_setup.size == 0 ) {
		return (1);
	}
	return (0);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t SOOL_QueueString_IsFull(const SOOL_Queue_String *q_ptr) {

	if ( q_ptr->_setup.size >= q_ptr->_setup.max_size ) {
		// queue is full
		return (1);
	}
	return (0);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void SOOL_QueueString_Pop(SOOL_Queue_String *q_ptr) {

	/* Check current size first */
	if ( q_ptr->_setup.size == 0 ) {
		return;
	}

	/* Free memory allocated by the first element */
//	q_ptr->_data->Free(q_ptr->_data);
	q_ptr->_data[0].Free(&q_ptr->_data[0]);

	/* Front element gets overwritten */
	SOOL_QueueString_Shift(q_ptr);

	/* Decrement size */
	q_ptr->_setup.size--;

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t SOOL_QueueString_PushString(SOOL_Queue_String *q_ptr, SOOL_String str) {

	/* Check current size */
	if ( SOOL_QueueString_IsFull(q_ptr) ) {
		return (0);
	}

	/* Size is within allowable range */
	if ( !SOOL_QueueString_Resize(q_ptr, q_ptr->_setup.size + 1) ) {
		// something went wrong
		return (0);
	}

	/* Update queue (Resize() updates the size) */
	uint8_t new_elem_pos = (q_ptr->_setup.size - 1);
	*(q_ptr->_data + new_elem_pos) = str;

	return (1);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t SOOL_QueueString_Push(SOOL_Queue_String *q_ptr, const char *str) {

	SOOL_String obj = SOOL_Memory_String_Init(7);
	obj.SetString(&obj, str);

	/* Try to add string to the queue */
	uint8_t status = SOOL_QueueString_PushString(q_ptr, obj); // TODO pass ptr?

	// DO NOT FREE BECAUSE DATA WILL BE LOST INSIDE THE QUEUE, IT WILL BE FREE'd LATER
//	/* Free memory */
//	obj.Free(&obj);

	if ( status ) {
		return (1);
	}

	return (0);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static SOOL_String SOOL_QueueString_GetFront(const SOOL_Queue_String *q_ptr) {
	return (*(q_ptr->_data));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

//static SOOL_String SOOL_QueueString_GetBack(const SOOL_Queue_String *q_ptr) {
//
//}
//
// // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//
//static uint8_t SOOL_QueueString_GetSize(const SOOL_Queue_String *q_ptr) {
//	return (q_ptr->_setup.size);
//}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// helper
static uint8_t SOOL_QueueString_Resize(SOOL_Queue_String *q_ptr, const size_t new_capacity) {

	/* Backup some info */
	uint16_t old_capacity = q_ptr->_setup.size;
	SOOL_String *ptr_backup = q_ptr->_data;

	/* Try to (re)allocate memory */
	if ( q_ptr->_setup.size != 0 ) {
		q_ptr->_data = realloc( q_ptr->_data, new_capacity * sizeof(SOOL_String) ); // (SOOL_String*)
	} else {
		// FIXME: get rid of the internal pointer, allocate for const char*!
//		q_ptr->_data = calloc( q_ptr->_data, new_capacity * sizeof(SOOL_String) ); // (SOOL_String*)
		q_ptr->_data = (SOOL_String *)calloc( (size_t)new_capacity, sizeof(SOOL_String) );
	}

	/* Check if the reallocation was successful */
	if ( q_ptr->_data != NULL ) {

		/* Clear the new part of an Array (if Array is bigger than before) */
		SOOL_String str; // blank object to be put into queue
		if ( old_capacity < new_capacity ) {
			for ( size_t i = old_capacity; i < new_capacity; i++) {
				q_ptr->_data[i] = str;
			}
		}

		/* Update capacity */
		q_ptr->_setup.size = new_capacity;

		return (1);

	}

	/* If the reallocation failed - restore a previous pointer and return 0 */
	q_ptr->_data = ptr_backup;

	// TODO: some error message
	return (0);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void SOOL_QueueString_Shift(SOOL_Queue_String *q_ptr) {

	// shift value position
//	for ( uint8_t i = 0; i < (q_ptr->_setup.size - 1); i++) {
//		// q_ptr->_data[i] = q_ptr->_data[i+1];
//		 *(q_ptr->_data + i) = (*(q_ptr->_data + i + 1));
//	}

	memmove(q_ptr->_data, q_ptr->_data + 1, q_ptr->_setup.size - 2);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
