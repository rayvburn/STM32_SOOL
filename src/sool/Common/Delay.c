/*
 * Delay.c
 *
 *  Created on: 22.08.2019
 *      Author: user
 */

#include "sool/Common/Delay.h"

void SOOL_Common_Delay(long unsigned int ms, long unsigned int core_clock) {
	long unsigned int stop = ms * core_clock / 72 * 4000; 	// experimentally
	for ( long unsigned int i = 0; i < stop; i++) { }
}

void SOOL_Common_DelayUs(long unsigned int us, long unsigned int core_clock) {
	long unsigned int stop = us * core_clock / 72 * 4; 		// experimentally
	for ( long unsigned int i = 0; i < stop; i++) { }
}
