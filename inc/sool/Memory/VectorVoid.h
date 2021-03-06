/*
 * Vector
 *
 *  Created on: 25.04.2019
 *      Author: user
 */

#ifndef MEMORY_VECTOR_
#define MEMORY_VECTOR_

/* Reference:
 * https://eddmann.com/posts/implementing-a-dynamic-vector-array-in-c/ */

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// forward declare a structure and typedef it to further make use of it inside a structure
struct VectorStruct;

/// \brief Vector is a dynamically growing array which contains
/// a set of pointers to pointers of a given type - thus each
/// element must be stored in memory;
/// Due to storing pointers, when an element changes value
/// it's value in vector also changes - no copy is done!
typedef struct VectorStruct VectorVoid;

struct VectorStruct {

	// fields
    void **_items;	// vector of pointers to pointers
    int _capacity;
    int _total;

    // functions
    int 	(*GetTotal)(const VectorVoid *v);
	void 	(*Add)(VectorVoid*, void*);
	void 	(*Set)(VectorVoid*, const unsigned int, void*);
	void* 	(*GetItem)(const VectorVoid*, const unsigned int);
	void 	(*DeleteItem)(VectorVoid*, const unsigned int);
	void 	(*FreeMemory)(VectorVoid*);

};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

VectorVoid SOOL_Memory_VectorVoid_Init(const unsigned int initial_capacity);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/* Example test code:
 *

	Vector vector_uint = SOOL_Vector_Init(3);
	unsigned int number = 90;
	vector_uint.Add(&vector_uint, &number);
	number = 92;
	vector_uint.Add(&vector_uint, &number);
	unsigned int total_num  = vector_uint.GetTotal(&vector_uint);
	unsigned int* elem_0 	= (unsigned int *)vector_uint.GetItem(&vector_uint, 0);
	unsigned int* elem_1 	= (unsigned int *)vector_uint.GetItem(&vector_uint, 1);
	unsigned int elem_0_val = *elem_0;
	unsigned int elem_1_val = *elem_1;

 *
 */

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#endif /* MEMORY_VECTOR_ */
