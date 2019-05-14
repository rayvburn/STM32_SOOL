/*
 * Add.h
 *
 *  Created on: 14.05.2019
 *      Author: user
 */

#ifndef INC_SOOL_MEMORY_ARRAY_TEMPLATES_ADD_H_
#define INC_SOOL_MEMORY_ARRAY_TEMPLATES_ADD_H_

#define ARRAY_ADD(ptr, item) 																				 \
({ 																											 \
																											 \
	(ptr->_info.add_index == string_ptr->_info.capacity) ? (ptr->_info.add_index = 0) : (ptr->_info.total++);\
	ptr->_data[string_ptr->_info.add_index] = item;															 \
	ptr->_info.add_index++;																					 \
})

#endif /* INC_SOOL_MEMORY_ARRAY_TEMPLATES_ADD_H_ */
