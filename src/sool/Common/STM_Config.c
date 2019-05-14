/*
 * STM_Config.c
 *
 *  Created on: 15.05.2019
 *      Author: user
 */

#include <sool/Common/STM_Config.h>

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

void SOOL_Common_DisableJTAG() {

	// Ref manual, p.183 AFIO_MAPR
	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;				// ENABLE clock for alternate function - needed for remapping
	AFIO->MAPR   |= AFIO_MAPR_SWJ_CFG_JTAGDISABLE;

}
