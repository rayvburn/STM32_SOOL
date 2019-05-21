/*
 * EXTI_IRQ.h
 *
 *  Created on: 26.04.2019
 *      Author: user
 */

#ifndef INC_SOOL_IRQN_IRQ_EXTI_H_
#define INC_SOOL_IRQN_IRQ_EXTI_H_

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

#endif /* INC_SOOL_IRQN_IRQ_EXTI_H_ */
