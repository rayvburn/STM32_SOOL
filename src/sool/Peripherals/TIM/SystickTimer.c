/*
 * Systick_Timer.c
 *
 *  Created on: 08.10.2018
 *      Author: user
 */

#include <sool/Peripherals/TIM/SystickTimer.h>

//// variables to store current time
volatile static uint32_t millis_elapsed = 0;

#ifdef SOOL_SYSTICK_EXTENDED_VERSION
volatile static uint32_t seconds_elapsed;
volatile static uint32_t tenths_elapsed;
volatile static uint32_t hundredths_elapsed;
#endif


//// helper counters for timing
#ifdef SOOL_SYSTICK_EXTENDED_VERSION
volatile static uint16_t seconds_counter = 0;
volatile static uint8_t tenths_counter = 0;
volatile static uint8_t hundredths_counter = 0;
#endif

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/** Configures SysTick to `tick` 1000 times per second;
 * internally counts elapsed milliseconds inside the ISR  */
void SOOL_Periph_TIM_SysTick_DefaultConfig() {
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);
	SysTick_Config( SystemCoreClock / 1000 ); // MCU-specific variable
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void SysTick_Handler() {

	millis_elapsed++;

#ifdef SOOL_SYSTICK_EXTENDED_VERSION
	// hundredth of a second counter
	if ( ++hundredths_counter == 10 ) {
		hundredths_counter = 0;
		hundredths_elapsed++;
	}

	// tenths of a second counter
	if ( ++tenths_counter == 100 ) {
		tenths_counter = 0;
		tenths_elapsed++;
	}

	// seconds counter
	if ( ++seconds_counter == 1000 ) {
		seconds_counter = 0;
		seconds_elapsed++;
	}
#endif

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uint32_t 	SOOL_Periph_TIM_SysTick_GetMillis()				{ 	return (millis_elapsed);		}

#ifdef SOOL_SYSTICK_EXTENDED_VERSION
uint32_t 	SOOL_Periph_TIM_SysTick_GetSec() 				{ 	return (seconds_elapsed);		}
uint32_t 	SOOL_Periph_TIM_SysTick_GetTenthsOfSec() 		{	return (tenths_elapsed);		}
uint32_t 	SOOL_Periph_TIM_SysTick_GetHundredthsOfSec() 	{	return (hundredths_elapsed); 	}
#endif

