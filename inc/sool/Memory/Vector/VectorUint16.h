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

	uint8_t	 (*Add)(SOOL_Vector_Uint16 *v_ptr, uint16_t val);
	uint16_t (*Get)(SOOL_Vector_Uint16 *v_ptr, uint16_t idx);
	uint8_t  (*Set)(SOOL_Vector_Uint16 *v_ptr, uint16_t idx, uint16_t value);
	uint8_t  (*Remove)(SOOL_Vector_Uint16 *v_ptr, uint16_t idx);
	void 	 (*Clear)(SOOL_Vector_Uint16 *v_ptr);
	void	 (*Free)(SOOL_Vector_Uint16 *v_ptr);
	uint8_t  (*Find)(SOOL_Vector_Uint16 *v_ptr, uint16_t *index_ptr, uint16_t value); // returns 1 if found, index_ptr is points to the first element which is equal to the given value

};

// - - - - - - - - - - - - - - - -

/**
 *
 * @note Initializer does not allocate any memory
 * @note Example of use @ https://gitlab.com/frb-pow/002tubewaterflowmcu/blob/63200cd02eac11177d323c57a406d01d8ad62d96/src/main.c#L102
 * @return
 */
extern SOOL_Vector_Uint16 SOOL_Memory_Vector_Uint16_Init();

// - - - - - - - - - - - - - - - -

#endif /* INC_SOOL_MEMORY_VECTOR_VECTORUINT16_H_ */
