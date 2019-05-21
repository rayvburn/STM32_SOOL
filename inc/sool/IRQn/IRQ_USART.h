/*
 * USART_IRQ.h
 *
 *  Created on: 10.05.2019
 *      Author: user
 */

#ifndef INC_SOOL_IRQN_IRQ_USART_H_
#define INC_SOOL_IRQN_IRQ_USART_H_

// ====================================================================================
/* USART_IRQ header file template */
// ====================================================================================

/* add some Setter function to copy the objects here */
extern void SOOL_IRQn_USART_SetObj (volatile void *obj_ptr);

/* IRQHandlers declaration */
extern void USART1_IRQHandler();
extern void USART2_IRQHandler();
extern void USART3_IRQHandler();

#endif /* INC_SOOL_IRQN_IRQ_USART_H_ */
