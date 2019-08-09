/*
 * VectorUint16_array.c
 *
 *  Created on: 09.08.2019
 *      Author: user
 */

#include "sool/Memory/Vector/VectorUint16_array.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

uint8_t SOOL_Memory_Vector_Uint16_Extend(SOOL_Vector_Uint16 *v_ptr, uint8_t size_curr) {

	if ( size_curr == 0 ) {

		/* -------------- Allocate memory -------------- */
		v_ptr = (SOOL_Vector_Uint16 *)calloc( (size_t)1, sizeof(SOOL_Vector_Uint16) );

		if ( v_ptr != NULL ) {
			return (1);
		}

		return (0);

	} else {

		/* -------------- Resize buffer -------------- */

		/* Backup some info */
		uint16_t *ptr_backup = v_ptr;

		/* Try to reallocate memory */
		v_ptr = realloc( (SOOL_Vector_Uint16*)v_ptr->_data, (size_curr + 1) * sizeof(SOOL_Vector_Uint16) );

		/* Check if the reallocation was successful */
		if ( v_ptr != NULL ) {
			return (1);
		}

		/* If the reallocation failed - restore a previous pointer and return 0 */
		v_ptr = ptr_backup;

		// TODO: some error message
		return (0);

	}

}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
