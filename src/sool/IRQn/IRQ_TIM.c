/*
 * IRQ_TIM.c
 *
 *  Created on: 20.05.2019
 *      Author: user
 */

#include "sool/IRQn/IRQ_TIM.h"

// ====================================================================================
/* TIM_IRQ definition file template */
// ====================================================================================

/* place objects declarations here */
//static volatile void *obj;

/* add some Setter function to copy the objects here */
void SOOL_IRQn_TIM_SetObj (volatile void *obj_ptr) { }

/* IRQHandlers declaration */
void TIM1_BRK_IRQHandler() 		{ }
void TIM1_UP_IRQHandler() 		{ }
void TIM1_TRG_COM_IRQHandler() 	{ }
void TIM1_CC_IRQHandler() 		{ }
void TIM2_IRQHandler() 			{ }
void TIM3_IRQHandler() 			{ }
void TIM4_IRQHandler() 			{ }
