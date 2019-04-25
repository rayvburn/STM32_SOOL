/*
 * Systick_Timer.c
 *
 *  Created on: 08.10.2018
 *      Author: user
 */

#include "Systick_Timer.h"

#include "USART.h"

volatile static uint32_t seconds_elapsed;
volatile static uint32_t tithings_elapsed;

void SysTick_Configuration() {
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);
	SysTick_Config( 72000000 / 10 ); // lack of hard-coded SystemCoreClock env constant (system_stm32f10x.h - extern)
}

void SysTick_Handler() {

	volatile static uint8_t sec_counter = 0;
	tithings_elapsed++;
	sec_counter++;
	if ( sec_counter==10 ) {
		// USART_SendString(DEBUGGING, "Another second elapsed\n");
		seconds_elapsed++;
		sec_counter = 0;
	}

}

uint32_t SysTick_GetSeconds() {
	return seconds_elapsed;
}

uint32_t SysTick_GetTithingsOfSec() {
	return tithings_elapsed;
}
