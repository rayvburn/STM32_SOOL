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
static uint8_t SOOL_QueueString_Resize(SOOL_Queue_String *q_ptr, int8_t size_change);

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

	/* Try to resize the queue (and update its size) */
	if ( !SOOL_QueueString_Resize(q_ptr, -1) ) {
		// something went wrong
		return;
	}

//	/* Front element gets overwritten */
//	SOOL_QueueString_Shift(q_ptr);
//
//	/* Decrement size */
//	q_ptr->_setup.size--;

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t SOOL_QueueString_PushString(SOOL_Queue_String *q_ptr, SOOL_String str) {

	/* Check current size */
	if ( SOOL_QueueString_IsFull(q_ptr) ) {
		return (0);
	}

	/* Size is within allowable range */
	if ( !SOOL_QueueString_Resize(q_ptr, +1) ) {
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

	/* Create new String instance */
	SOOL_String string = SOOL_Memory_String_Init(1); // (7)
//	obj.SetString(&obj, str);
	string.Append(&string, str);

	/* Try to add string to the queue */
	uint8_t status = SOOL_QueueString_PushString(q_ptr, string); // TODO pass ptr?

	/* Return status of the operation */
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
/**
 *
 * @param q_ptr
 * @param size_change - can be +1 or -1 for the queue
 * @return
 */
static uint8_t SOOL_QueueString_Resize(SOOL_Queue_String *q_ptr, int8_t size_change) {

	/* Backup some info */
	uint16_t old_capacity = q_ptr->_setup.size;
	SOOL_String *ptr_backup = q_ptr->_data;

	/* Try to (re)allocate memory */
	if ( q_ptr->_setup.size != 0 ) {

		// Seems that at least 1 element is present in the queue.
		// Check whether element to be added or deleted.
		if ( size_change == 1 ) {

			q_ptr->_data = realloc( q_ptr->_data, (q_ptr->_setup.size + 1) * sizeof(SOOL_String) );

		} else if ( size_change == -1 ) {

			// Check if there are 1 or more elements in the queue
			if ( q_ptr->_setup.size == 1) {

				// nothing to be done, first (and the only) element's memory has already been freed
				q_ptr->_setup.size = 0;
				return (1);

			} else {

				// Due to Queue structure in memory, let's reallocate it. Use current second element
				// as a base address as the first element needs to be Popped now and its memory has already
				// been freed.
//				q_ptr->_data = realloc( q_ptr->_data + 1, (q_ptr->_setup.size - 1) * sizeof(SOOL_String) );
				q_ptr->_data = realloc( &q_ptr->_data[1], (q_ptr->_setup.size - 1) * sizeof(SOOL_String) );

			}

		}

	} else {

		// This can happen only when element needs to be added.
		// 0 elements in the queue, memory must be allocated from scratch
		q_ptr->_data = (SOOL_String *)calloc( (size_t)1, sizeof(SOOL_String) );

	}

	/* Check if the reallocation was successful */
	if ( q_ptr->_data != NULL ) {

		/* Update size */
		q_ptr->_setup.size += size_change;

		/* Clear the new part of a Queue (if Queue is bigger than before) */
		if ( size_change == 1 ) {
			SOOL_String str; // blank object to be put into queue
			q_ptr->_data[q_ptr->_setup.size - 1] = str;
		}

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
	for ( int16_t i = 0; i < (q_ptr->_setup.size - 1); i++) {
		// q_ptr->_data[i] = q_ptr->_data[i+1];
		 *(q_ptr->_data + i) = (*(q_ptr->_data + i + 1));
	}

//	memmove(q_ptr->_data, q_ptr->_data + 1, q_ptr->_setup.size - 2);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
