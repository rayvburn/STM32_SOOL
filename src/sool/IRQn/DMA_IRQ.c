/*
 * DMA_IRQ.c
 *
 *  Created on: 09.05.2019
 *      Author: user
 */

#include "sool/IRQn/DMA_IRQ.h"

// ====================================================================================
/* DMA_IRQ definition file template */
// ====================================================================================

/* place objects declarations here */
//static volatile void *obj;

/* add some Setter function to copy the objects here */
void SOOL_DMA_IRQn_SetObj (volatile void *obj_ptr) { }

// -----------------------------------------------

/* IRQHandlers definitions */
void DMA1_Channel1_IRQHandler() {

	/* DMA1 Channel1 global interrupt */

	/* ADC1 		peripheral request signal */
	/* TIM2_CH3		peripheral request signal */
	/* TIM4_CH1		peripheral request signal */

}

// -----------------------------------------------

void DMA1_Channel2_IRQHandler() {

	/* DMA1 Channel2 global interrupt */

	/* USART3_TX 	peripheral request signal */
	/* TIM1_CH1		peripheral request signal */
	/* TIM2_UP		peripheral request signal */
	/* TIM3_CH3 	peripheral request signal */
	/* SPI1_RX  	peripheral request signal */

}

// -----------------------------------------------

void DMA1_Channel3_IRQHandler() {

	/* DMA1 Channel3 global interrupt */

	/* USART3_RX 	peripheral request signal */
	/* TIM1_CH2 	peripheral request signal */
	/* TIM3_CH4 	peripheral request signal */
	/* TIM3_UP 		peripheral request signal */
	/* SPI1_TX 		peripheral request signal */

}

// -----------------------------------------------

void DMA1_Channel4_IRQHandler() {

	/* DMA1 Channel4 global interrupt */

	/* USART1_TX 	peripheral request signal */
	/* TIM1_CH4 	peripheral request signal */
	/* TIM1_TRIG 	peripheral request signal */
	/* TIM1_COM 	peripheral request signal */
	/* TIM4_CH2 	peripheral request signal */
	/* SPI/I2S2_RX 	peripheral request signal */
	/* I2C2_TX 		peripheral request signal */

}

// -----------------------------------------------

void DMA1_Channel5_IRQHandler() {

	/* DMA1 Channel5 global interrupt */

	/* USART1_RX 	peripheral request signal */
	/* TIM1_UP 		peripheral request signal */
	/* SPI/I2S2_TX 	peripheral request signal */
	/* TIM2_CH1 	peripheral request signal */
	/* TIM4_CH3 	peripheral request signal */
	/* I2C2_RX 		peripheral request signal */

}

// -----------------------------------------------

void DMA1_Channel6_IRQHandler() {

	/* DMA1 Channel6 global interrupt */

	/* USART2_RX 	peripheral request signal */
	/* TIM1_CH3 	peripheral request signal */
	/* TIM3_CH1 	peripheral request signal */
	/* TIM3_TRIG 	peripheral request signal */
	/* I2C1_TX 		peripheral request signal */

}

// -----------------------------------------------

void DMA1_Channel7_IRQHandler() {

	/* DMA1 Channel7 global interrupt */

	/* USART2_TX 	peripheral request signal */
	/* TIM2_CH2 	peripheral request signal */
	/* TIM2_CH4 	peripheral request signal */
	/* TIM4_UP 		peripheral request signal */
	/* I2C1_RX 		peripheral request signal */

}
