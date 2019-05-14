/*
 * EXTI_IRQ.c
 *
 *  Created on: 26.04.2019
 *      Author: user
 */

#include "sool/IRQn/EXTI_IRQ.h"

// ====================================================================================
/* EXTI_IRQ definition file template */
// ====================================================================================

/* place objects declarations here */
//static volatile void *obj;

/* add some Setter function to copy the objects here */
void EXTI_IRQ_Handler_SetObj(volatile void *obj_ptr) { }

// -----------------------------------------------

/* IRQHandlers definitions */
void EXTI0_IRQHandler()   	{ /* only 1 object can trigger interrupts on this line */ }
void EXTI1_IRQHandler()   	{ /* only 1 object can trigger interrupts on this line */ }
void EXTI2_IRQHandler()   	{ /* only 1 object can trigger interrupts on this line */ }
void EXTI3_IRQHandler()   	{ /* only 1 object can trigger interrupts on this line */ }
void EXTI4_IRQHandler()   	{ /* only 1 object can trigger interrupts on this line */ }
void EXTI9_5_IRQHandler()	{
	/* EXTI Line 5 */
	/* EXTI Line 6 */
	/* EXTI Line 7 */
	/* EXTI Line 8 */
	/* EXTI Line 9 */
}
void EXTI15_10_IRQHandler() {
	/* EXTI Line 10 */
	/* EXTI Line 11 */
	/* EXTI Line 12 */
	/* EXTI Line 13 */
	/* EXTI Line 14 */
	/* EXTI Line 15 */
}
