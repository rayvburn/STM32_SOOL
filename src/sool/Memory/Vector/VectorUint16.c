/*
 * VectorUint16.c
 *
 *  Created on: 08.07.2019
 *      Author: user
 */

#include "sool/Memory/Vector/VectorUint16.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void SOOL_Vector_Add(SOOL_Vector_Uint16 *v_ptr, uint16_t c);
static uint16_t SOOL_Vector_Get(SOOL_Vector_Uint16 *v_ptr, uint16_t idx);
static uint8_t SOOL_Vector_Set(SOOL_Vector_Uint16 *v_ptr, uint16_t idx, uint16_t value);
static uint8_t SOOL_Vector_Remove(SOOL_Vector_Uint16 *v_ptr, uint16_t idx);
static void SOOL_Vector_Clear(SOOL_Vector_Uint16 *v_ptr);
static void	SOOL_Vector_Free(SOOL_Vector_Uint16 *v_ptr);
static uint8_t SOOL_Vector_Find(SOOL_Vector_Uint16 *v_ptr, uint16_t *index_ptr, uint16_t value);

// helper
static uint8_t SOOL_Vector_Resize(SOOL_Vector_Uint16 *v_ptr, unsigned int new_size);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

SOOL_Vector_Uint16 SOOL_Memory_Vector_Uint16_Init() {

	/* New object */
	SOOL_Vector_Uint16 v;

	/* Initial size, no memory allocation here */
	v._info.size = 0;

	/* Assign function pointers */
	v.Add = SOOL_Vector_Add;
	v.Clear = SOOL_Vector_Clear;
	v.Free = SOOL_Vector_Free;
	v.Get = SOOL_Vector_Get;
	v.Remove = SOOL_Vector_Remove;
	v.Set = SOOL_Vector_Set;
	v.Find = SOOL_Vector_Find;

	return (v);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void SOOL_Vector_Add(SOOL_Vector_Uint16 *v_ptr, uint16_t val) {

	if ( v_ptr->_info.size == 0 ) {

		/* Allocate memory */
		v_ptr->_data = (uint16_t *)calloc( (size_t)1, sizeof(uint16_t) );
		v_ptr->_info.size = 1;

	} else {

		/* Resize buffer */
		SOOL_Vector_Resize(v_ptr, v_ptr->_info.size + 1);

	}

	/* We're safe to add value to the buffer */
	SOOL_Vector_Set(v_ptr, v_ptr->_info.size - 1, val);

}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static uint16_t SOOL_Vector_Get(SOOL_Vector_Uint16 *v_ptr, uint16_t idx) {

	if ( idx < v_ptr->_info.size ) {
		return (*(v_ptr->_data + idx));
	}
	return (0);

}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static uint8_t SOOL_Vector_Set(SOOL_Vector_Uint16 *v_ptr, uint16_t idx, uint16_t value) {

	if ( idx < v_ptr->_info.size ) {
		*(v_ptr->_data + idx) = value;
		return (1);
	}
	return (0);

}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static uint8_t SOOL_Vector_Remove(SOOL_Vector_Uint16 *v_ptr, uint16_t idx) {

	if ( idx < v_ptr->_info.size ) {

		// check `idx`, shift values only when `idx` is smaller than last element's index
		if ( idx != (v_ptr->_info.size - 1) ) {

			/* Shift values located after `idx` one position back */
			for ( uint16_t i = idx; i < (v_ptr->_info.size - 1); i++ ) {
				(*(v_ptr->_data + i)) = (*(v_ptr->_data + i + 1));
			}

		}

		/* Try to resize buffer */
		if ( SOOL_Vector_Resize(v_ptr, v_ptr->_info.size - 1) ) {
			return (1);
		}
		return (0);

	}
	return (0);

}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static void SOOL_Vector_Clear(SOOL_Vector_Uint16 *v_ptr) {

	for ( size_t i = 0; i < v_ptr->_info.size; i++ ) {
		v_ptr->_data[i] = 0;
	}

}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static void	SOOL_Vector_Free(SOOL_Vector_Uint16 *v_ptr) {
	free(v_ptr->_data);
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static uint8_t SOOL_Vector_Find(SOOL_Vector_Uint16 *v_ptr, uint16_t *index_ptr, uint16_t value) {

	for ( uint16_t i = 0; i < (v_ptr->_info.size); i++ ) {
		if ( v_ptr->_data[i] == value ) {
			*index_ptr = i;
			return (1);
		}
	}
	return (0);

}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static uint8_t SOOL_Vector_Resize(SOOL_Vector_Uint16 *v_ptr, unsigned int new_size) {

	/* Backup some info */
	uint16_t old_size = v_ptr->_info.size;
	uint16_t *ptr_backup = v_ptr->_data;

	/* Try to reallocate memory */
	v_ptr->_data = realloc( (uint16_t*)v_ptr->_data, new_size * sizeof(uint16_t) );

	/* Check if the reallocation was successful */
	if ( v_ptr->_data != NULL ) {

		/* Clear the new part of an Array (if Array is bigger than before) */
		if ( old_size < new_size ) {
			for ( size_t i = old_size; i < new_size; i++) {
				v_ptr->_data[i] = 0;
			}
		}

		/* Update capacity */
		v_ptr->_info.size = new_size;

		return (1);

	}

	/* If the reallocation failed - restore a previous pointer and return 0 */
	v_ptr->_data = ptr_backup;

	// TODO: some error message
	return (0);

}
