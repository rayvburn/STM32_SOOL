/*
 * VectorUint16.h
 *
 *  Created on: 08.07.2019
 *      Author: user
 */

#ifndef INC_SOOL_MEMORY_VECTOR_VECTORUINT16_H_
#define INC_SOOL_MEMORY_VECTOR_VECTORUINT16_H_

#include "sool/Memory/Vector/Vector_common.h"
#include <stdint.h>
#include <stddef.h> // size_t

// - - - - - - - - - - - - - - - -

struct _SOOL_Vector_Uint16Struct; // forward declaration
typedef struct _SOOL_Vector_Uint16Struct SOOL_Vector_Uint16;

struct _SOOL_Vector_Uint16Struct {

	struct _SOOL_Vector_Info 	_info;
	uint16_t					*_data;

	void 	 (*Add)(SOOL_Vector_Uint16 *v_ptr, uint16_t val);
	uint16_t (*Get)(SOOL_Vector_Uint16 *v_ptr, uint16_t idx);
	uint8_t  (*Set)(SOOL_Vector_Uint16 *v_ptr, uint16_t idx, uint16_t value);
	uint8_t  (*Remove)(SOOL_Vector_Uint16 *v_ptr, uint16_t idx);
	void 	 (*Clear)(SOOL_Vector_Uint16 *v_ptr);
	void	 (*Free)(SOOL_Vector_Uint16 *v_ptr);

};

// - - - - - - - - - - - - - - - -

// Initializer does not allocate any memory
extern SOOL_Vector_Uint16 SOOL_Memory_Vector_Uint16_Init();

// - - - - - - - - - - - - - - - -

#endif /* INC_SOOL_MEMORY_VECTOR_VECTORUINT16_H_ */
