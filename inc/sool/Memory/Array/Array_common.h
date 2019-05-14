/*
 * Array_common.h
 *
 *  Created on: 14.05.2019
 *      Author: user
 */

#ifndef INC_SOOL_MEMORY_ARRAY_ARRAY_COMMON_H_
#define INC_SOOL_MEMORY_ARRAY_ARRAY_COMMON_H_

#include <stdint.h>

struct _SOOL_Array_Info {
	uint16_t total;
	uint16_t capacity;
	uint16_t add_index;	// stores index of an element to-be-added
};

#endif /* INC_SOOL_MEMORY_ARRAY_ARRAY_COMMON_H_ */
