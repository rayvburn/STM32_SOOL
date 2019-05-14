/*
 * StmConfig.h
 *
 *  Created on: 03.10.2018
 *      Author: user
 */

#ifndef COMMON_STMCONFIG_H_
#define COMMON_STMCONFIG_H_

#include "stm32f10x.h"
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
/* STM Clock and pin remapping configuration TEMPLATE
 * with default settings (72 MHz for F103C8T6) */
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void SOOL_Common_PinRemapDefault() {

	// RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;     		 		// ENABLE clock for alternate function
	// AFIO->MAPR |= AFIO_MAPR_SWJ_CFG_JTAGDISABLE;  		// disable JTDI on PA15

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE); 	// ENABLE clock for alternate function - needed for remapping

	/* JTAG Remapping - disable JTDI on PA15 */
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

	/* I2C Remapping
	 *	0: No remap (SCL/PB6, SDA/PB7)
	 *	1: Remap 	(SCL/PB8, SDA/PB9)

	GPIO_PinRemapConfig(GPIO_Remap_I2C1, ENABLE);
	 */
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void SOOL_Common_ClockConfigDefault() {

	RCC_DeInit();
	RCC_HSEConfig(RCC_HSE_ON); 									// enable external high speed oscillator
	ErrorStatus HSEStartUpStatus = RCC_WaitForHSEStartUp();
	if ( HSEStartUpStatus == SUCCESS ) {
		FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
		FLASH_SetLatency(FLASH_Latency_2); 						// flash latency
		RCC_HCLKConfig(RCC_SYSCLK_Div1); 						// HCLK = SYSCLK
		RCC_PCLK2Config(RCC_HCLK_Div1);  						// PCLK2 = HCLK
		RCC_PCLK1Config(RCC_HCLK_Div2);  						// PCLK1 = HCLK/2
		RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9); 	// PLLCLK = 8MHz * 9 = 72 MHz
		RCC_PLLCmd(ENABLE);
		while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET); 	// wait for PLL start
		RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK); 				// PLL selected as sysclk source
		while(RCC_GetSYSCLKSource() != 0x08);					// wait until PLL is surely used as system clock
	}

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#endif /* COMMON_STMCONFIG_H_ */
