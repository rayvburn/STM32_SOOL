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
void SOOL_EXTI_IRQn_SetObj(volatile void *obj_ptr) { }

// -----------------------------------------------

/* IRQHandlers definitions
 * --------------------------
 * NOTE: do not forget to delete every instance of interrupt handler
 * when you stop using related `class`' object (the one which triggers
 * interrupt) because application will go into infinite loop when a different
 * line in the same handler will trigger interrupt (method = indefinite pointer then)
 * --------------------------
 */
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
