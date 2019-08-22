/*
 * Delay.c
 *
 *  Created on: 22.08.2019
 *      Author: user
 */

#include "sool/Common/Delay.h"

void SOOL_Common_Delay(long unsigned int ms, long unsigned int core_clock) {
	unsigned int core_clock_mhz = core_clock / 1000000;
	long unsigned int stop = ms * (core_clock_mhz * 5000 / 72); 	// experimentally
	for ( long unsigned int i = 0; i < stop; i++) { }
}

void SOOL_Common_DelayUs(long unsigned int us, long unsigned int core_clock) {
	unsigned int core_clock_mhz = core_clock / 1000000;
	long unsigned int stop = us * (core_clock_mhz * 5 / 72); 	// experimentally
	for ( long unsigned int i = 0; i < stop; i++) { }
}
