/*
 * IRQ_I2C.c
 *
 *  Created on: 02.12.2019
 *      Author: user
 */

#include <sool/IRQn/IRQ_I2C.h>

// ====================================================================================
/* I2C_IRQ definition file template */
// ====================================================================================

/* place objects declarations here */
//static volatile void *obj;

/* add some Setter function to copy the objects here */
void SOOL_IRQn_I2C_SetObj (volatile void *obj_ptr) { }

// -----------------------------------------------

/* IRQHandlers definitions */
void I2C1_IRQHandler() {
	/* I2C1 global interrupt */
}

void I2C2_IRQHandler() {
	/* I2C2 global interrupt */
}
