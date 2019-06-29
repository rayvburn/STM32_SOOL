/*
 * IRQ_TIM.h
 *
 *  Created on: 20.05.2019
 *      Author: user
 */

#ifndef INC_SOOL_IRQN_IRQ_TIM_H_
#define INC_SOOL_IRQN_IRQ_TIM_H_

// ====================================================================================
/* TIM_IRQ header file template */
// ====================================================================================

/* add header files */
//#include <.h>

/* add some Setter function to copy the objects here */
extern void SOOL_IRQn_TIM_SetObj (volatile void *obj_ptr);

/* IRQHandlers declaration */
extern void TIM1_BRK_IRQHandler();
extern void TIM1_UP_IRQHandler();
extern void TIM1_TRG_COM_IRQHandler();
extern void TIM1_CC_IRQHandler();
extern void TIM2_IRQHandler();
extern void TIM3_IRQHandler();
extern void TIM4_IRQHandler();

#endif /* INC_SOOL_IRQN_IRQ_TIM_H_ */
