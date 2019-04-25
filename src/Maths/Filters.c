/*
 * Filters.c
 *
 *  Created on: 20.10.2018
 *      Author: user
 */

#include "Maths/Filters.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// private define
#define QUICK_SELECT_ELEM_SWAP(a,b) { register int16_t t=(a);(a)=(b);(b)=t; }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void Filter_ShiftValues(int16_t new, int16_t *arr, uint8_t arr_dim) {

	for ( uint8_t i = 0; i < (arr_dim-1); i++ ) {
		*(arr + (arr_dim-1) - i) = *(arr + (arr_dim-1) - i - 1); // shift register
	}
	*arr = new;

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

int16_t Filter_GetAvg(const int16_t *arr, uint8_t arr_dim) {

	int64_t sum = 0;
	for ( uint8_t i = 0; i < arr_dim; i++ ) {
		sum += *(arr + i);
	}

	// to speed up calculations
	if ( arr_dim == 256 ) {
		return sum >> 8;
	} else if ( arr_dim == 128 ) {
		return sum >> 7;
	} else if ( arr_dim == 64 ) {
		return sum >> 6;
	} else if ( arr_dim == 32 ) {
		return sum >> 5;
	} else if ( arr_dim == 16 ) {
		return sum >> 4;
	} else if ( arr_dim == 8 ) {
		return sum >> 3;
	} else if ( arr_dim == 4 ) {
		return sum >> 2;
	} else if ( arr_dim == 2 ) {
		return sum >> 1;
	} else {
		return (sum/arr_dim);
	}

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

int16_t Filter_GetMedian (int16_t *arr, uint8_t arr_dim) {

	// QuickSelect method, http://ndevilla.free.fr/median/median/

    int low, high;
    int median;
    int middle, ll, hh;

    low = 0 ;
    high = arr_dim-1 ;
    median = (low + high) / 2;

    for (;;) {

        if (high <= low) /* One element only */
            return *(arr+median);

        if (high == low + 1) {  /* Two elements only */
            if ( (*(arr+low)) > (*(arr+high)) ) {
            	QUICK_SELECT_ELEM_SWAP( *(arr+low), *(arr+high) ) ;
            }
            return *(arr+median);
        }

    /* Find median of low, middle and high items; swap into position low */
    middle = (low + high) / 2;

    if ( (*(arr+middle)) > (*(arr+high)) )  QUICK_SELECT_ELEM_SWAP( (*(arr+middle)), (*(arr+high)) ) ;
    if ( (*(arr+low)) 	 > (*(arr+high)) )  QUICK_SELECT_ELEM_SWAP( (*(arr+low)),    (*(arr+high)) ) ;
    if ( (*(arr+middle)) > (*(arr+low))  )  QUICK_SELECT_ELEM_SWAP( (*(arr+middle)), (*(arr+low))) ;

    /* Swap low item (now in position middle) into position (low+1) */
    QUICK_SELECT_ELEM_SWAP( (*(arr+middle)) , (*(arr+low+1)) );

    /* Nibble from each end towards middle, swapping items when stuck */
    ll = low + 1;
    hh = high;
    for (;;) {
        do ll++; while ( (*(arr+low)) > (*(arr+ll)) ) ;
        do hh--; while ( (*(arr+hh))  > (*(arr+low)) ) ;

        if (hh < ll)
        break;

        QUICK_SELECT_ELEM_SWAP( (*(arr+ll)) , (*(arr+hh)) ) ;
    }

    /* Swap middle item (in position low) back into correct position */
    QUICK_SELECT_ELEM_SWAP( (*(arr+low)), (*(arr+hh)) ) ;

    /* Re-set active partition */
    if (hh <= median)
        low = ll;
        if (hh >= median)
        high = hh - 1;
    }

    return (0);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void Filter_ClearArray(int16_t *arr) {
	memset(arr, 0, sizeof(*arr));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
