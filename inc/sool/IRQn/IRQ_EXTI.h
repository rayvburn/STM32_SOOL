/*
 * EXTI_IRQ.h
 *
 *  Created on: 26.04.2019
 *      Author: user
 */

#ifndef INCLUDE_IRQN_EXTI_IRQ_H_
#define INCLUDE_IRQN_EXTI_IRQ_H_

// ====================================================================================
/* EXTI_IRQ header file template */
// ====================================================================================

/* add some Setter function to copy the objects here */
extern void SOOL_IRQn_EXTI_SetObj (volatile void *obj_ptr);

/* IRQHandlers declaration */
extern void EXTI0_IRQHandler();
extern void EXTI1_IRQHandler();
extern void EXTI2_IRQHandler();
extern void EXTI3_IRQHandler();
extern void EXTI4_IRQHandler();
extern void EXTI9_5_IRQHandler();
extern void EXTI15_10_IRQHandler();

#endif /* INCLUDE_IRQN_EXTI_IRQ_H_ */
