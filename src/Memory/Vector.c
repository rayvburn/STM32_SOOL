/*
 * Vector.c
 *
 *  Created on: 25.04.2019
 *      Author: user
 */

#include <include/Memory/Vector.h>
#include <stdio.h>
#include <stdlib.h>

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static int Vector_GetTotal(const Vector *v);
static void Vector_Resize(Vector*, const unsigned int);
static void Vector_Add(Vector*, void*);
static void Vector_Set(Vector*, const unsigned int, void*);
static void* Vector_GetElem(const Vector*, const unsigned int);
static void Vector_DeleteElem(Vector*, const unsigned int);
static void Vector_FreeMemory(Vector*);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

Vector SOOL_Vector_Init(const unsigned int initial_capacity) {

	Vector v;

	v.capacity = initial_capacity;
	v.total = 0;
	v.items = malloc(sizeof(void *) * initial_capacity);

	v.Add = Vector_Add;
	v.DeleteElem = Vector_DeleteElem;
	v.FreeMemory = Vector_FreeMemory;
	v.GetElem = Vector_GetElem;
	v.GetTotal = Vector_GetTotal;
	v.Set = Vector_Set;

	return (v);

}

// =====================================================================
static int Vector_GetTotal(const Vector *v) {
	return (v->total);
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static void Vector_Resize(Vector *v, const unsigned int new_capacity) {

    void **items = realloc(v->items, sizeof(void *) * new_capacity);
    if (items) {
    	// if reallocation ok
        v->items = items;
        v->capacity = new_capacity;
    }

}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static void Vector_Add(Vector *v, void *item) {

	if (v->capacity == v->total) {
		Vector_Resize(v, v->capacity * 2);
	}
	v->items[v->total++] = item;

}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static void Vector_Set(Vector *v, const unsigned int index, void *item) {

	 if (index >= 0 && index < v->total) {
		v->items[index] = item;
	 }

}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static void* Vector_GetElem(const Vector *v, const unsigned int index) {

	if (index >= 0 && index < v->total)
		return (v->items[index]);
	return (NULL);

}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static void Vector_DeleteElem(Vector *v, const unsigned int index) {

    if (index < 0 || index >= v->total) {
        return;
    }

    v->items[index] = NULL;

    for (int i = index; i < v->total - 1; i++) {
        v->items[i] = v->items[i + 1];
        v->items[i + 1] = NULL;
    }

    v->total--;

    if (v->total > 0 && v->total == v->capacity / 4) {
        Vector_Resize(v, v->capacity / 2);
    }

}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static void Vector_FreeMemory(Vector *v) {
	free(v->items);
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
