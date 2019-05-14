/*
 * Vector.c
 *
 *  Created on: 25.04.2019
 *      Author: user
 */

#include "sool/Memory/Vector.h"
#include <stdio.h>
#include <stdlib.h>

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static int Vector_GetTotal(const Vector *v);
static void Vector_Resize(Vector*, const unsigned int);
static void Vector_Add(Vector*, void*);
static void Vector_Set(Vector*, const unsigned int, void*);
static void* Vector_GetItem(const Vector*, const unsigned int);
static void Vector_DeleteItem(Vector*, const unsigned int);
static void Vector_FreeMemory(Vector*);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

Vector SOOL_Memory_Vector_Init(const unsigned int initial_capacity) {

	Vector v;

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
static int Vector_GetTotal(const Vector *v) {
	return (v->_total);
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static void Vector_Resize(Vector *v, const unsigned int new_capacity) {

    void **items = realloc(v->_items, sizeof(void *) * new_capacity);
    if (items) {
    	// if reallocation ok
        v->_items = items;
        v->_capacity = new_capacity;
    }

}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static void Vector_Add(Vector *v, void *item) {

	if (v->_capacity == v->_total) {
		Vector_Resize(v, v->_capacity * 2);
	}
	v->_items[v->_total++] = item;

}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static void Vector_Set(Vector *v, const unsigned int index, void *item) {

	 if (index >= 0 && index < v->_total) {
		v->_items[index] = item;
	 }

}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static void* Vector_GetItem(const Vector *v, const unsigned int index) {

	if (index >= 0 && index < v->_total)
		return (v->_items[index]);
	return (NULL);

}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static void Vector_DeleteItem(Vector *v, const unsigned int index) {

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
static void Vector_FreeMemory(Vector *v) {
	free(v->_items);
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
