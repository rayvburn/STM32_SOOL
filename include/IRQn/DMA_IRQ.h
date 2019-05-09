/*
 * DMA_IRQ.h
 *
 *  Created on: 09.05.2019
 *      Author: user
 */

#ifndef INCLUDE_IRQN_DMA_IRQ_H_
#define INCLUDE_IRQN_DMA_IRQ_H_

// ====================================================================================
/* DMA_IRQ header file template */
// ====================================================================================

/* add some Setter function to copy the objects here */
extern void DMA_IRQ_Handler_SetObj (volatile void *obj_ptr);

/* IRQHandlers declaration */
extern void DMA1_Channel1_IRQHandler();		/* DMA1 Channel1 global interrupt                   */
extern void DMA1_Channel2_IRQHandler();		/* DMA1 Channel2 global interrupt                   */
extern void DMA1_Channel3_IRQHandler();		/* DMA1 Channel3 global interrupt                   */
extern void DMA1_Channel4_IRQHandler();		/* DMA1 Channel4 global interrupt                   */
extern void DMA1_Channel5_IRQHandler();		/* DMA1 Channel5 global interrupt                   */
extern void DMA1_Channel6_IRQHandler();		/* DMA1 Channel6 global interrupt                   */
extern void DMA1_Channel7_IRQHandler();		/* DMA1 Channel7 global interrupt                   */

#endif /* INCLUDE_IRQN_DMA_IRQ_H_ */
