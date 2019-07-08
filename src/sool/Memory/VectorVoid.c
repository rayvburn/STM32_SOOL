/*
 * Vector.c
 *
 *  Created on: 25.04.2019
 *      Author: user
 */

#include <sool/Memory/VectorVoid.h>
#include <stdio.h>
#include <stdlib.h>

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static int Vector_GetTotal(const VectorVoid *v);
static void Vector_Resize(VectorVoid*, const unsigned int);
static void Vector_Add(VectorVoid*, void*);
static void Vector_Set(VectorVoid*, const unsigned int, void*);
static void* Vector_GetItem(const VectorVoid*, const unsigned int);
static void Vector_DeleteItem(VectorVoid*, const unsigned int);
static void Vector_FreeMemory(VectorVoid*);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

VectorVoid SOOL_Memory_VectorVoid_Init(const unsigned int initial_capacity) {

	VectorVoid v;

	v._capacity = initial_capacity;
	v._total = 0;
	v._items = malloc(sizeof(void *) * initial_capacity);

	v.Add = Vector_Add;
	v.DeleteItem = Vector_DeleteItem;
	v.FreeMemory = Vector_FreeMemory;
	v.GetItem = Vector_GetItem;
	v.GetTotal = Vector_GetTotal;
	v.Set = Vector_Set;

	return (v);

}

// =====================================================================
static int Vector_GetTotal(const VectorVoid *v) {
	return (v->_total);
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static void Vector_Resize(VectorVoid *v, const unsigned int new_capacity) {

    void **items = realloc(v->_items, sizeof(void *) * new_capacity);
    if (items) {
    	// if reallocation ok
        v->_items = items;
        v->_capacity = new_capacity;
    }

}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static void Vector_Add(VectorVoid *v, void *item) {

	if (v->_capacity == v->_total) {
		Vector_Resize(v, v->_capacity * 2);
	}
	v->_items[v->_total++] = item;

}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static void Vector_Set(VectorVoid *v, const unsigned int index, void *item) {

	 if (index >= 0 && index < v->_total) {
		v->_items[index] = item;
	 }

}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static void* Vector_GetItem(const VectorVoid *v, const unsigned int index) {

	if (index >= 0 && index < v->_total)
		return (v->_items[index]);
	return (NULL);

}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static void Vector_DeleteItem(VectorVoid *v, const unsigned int index) {

    if (index < 0 || index >= v->_total) {
        return;
    }

    v->_items[index] = NULL;

    for (int i = index; i < v->_total - 1; i++) {
        v->_items[i] = v->_items[i + 1];
        v->_items[i + 1] = NULL;
    }

    v->_total--;

    if (v->_total > 0 && v->_total == v->_capacity / 4) {
        Vector_Resize(v, v->_capacity / 2);
    }

}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static void Vector_FreeMemory(VectorVoid *v) {
	free(v->_items);
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
