/*
 * USART_IRQ.h
 *
 *  Created on: 10.05.2019
 *      Author: user
 */

#ifndef INCLUDE_IRQN_USART_IRQ_H_
#define INCLUDE_IRQN_USART_IRQ_H_

// ====================================================================================
/* USART_IRQ header file template */
// ====================================================================================

/* add some Setter function to copy the objects here */
extern void USART_IRQ_Handler_SetObj (volatile void *obj_ptr);

/* IRQHandlers declaration */
extern void USART1_IRQHandler();
extern void USART2_IRQHandler();
extern void USART3_IRQHandler();

#endif /* INCLUDE_IRQN_USART_IRQ_H_ */
