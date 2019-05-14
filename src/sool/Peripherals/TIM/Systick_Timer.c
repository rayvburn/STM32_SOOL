/*
 * Systick_Timer.c
 *
 *  Created on: 08.10.2018
 *      Author: user
 */

#include "sool/Peripherals/TIM/Systick_Timer.h"

// variables to store current time
volatile static uint32_t seconds_elapsed;
volatile static uint32_t tenths_elapsed;
volatile static uint32_t hundredths_elapsed;

// helper counters for timing
volatile static uint8_t seconds_counter = 0;
volatile static uint8_t tenths_counter = 0;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/**
 * Configures SysTick to `tick` 100 times per second;
 * internally counts elapsed tenth`s of second and seconds
 * inside the ISR  */
void SOOL_Periph_SysTick_DefaultConfig() {
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);
	SysTick_Config( SystemCoreClock / 100 ); // MCU-specific variable
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void SysTick_Handler() {

	// hundredth of second counter
	hundredths_elapsed++;

	// tenths of second counter
	if ( ++tenths_counter == 10 ) {
		tenths_counter = 0;
		tenths_elapsed++;
	}

	// seconds counter
	if ( ++seconds_counter == 100 ) {
		seconds_counter = 0;
		seconds_elapsed++;
	}

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uint32_t SOOL_Periph_SysTick_GetSec() 				{ 	return (seconds_elapsed);		}
uint32_t SOOL_Periph_SysTick_GetTenthsOfSec() 		{	return (tenths_elapsed);		}
uint32_t SOOL_Periph_SysTick_GetHundredthsOfSec() 	{	return (hundredths_elapsed); 	}
