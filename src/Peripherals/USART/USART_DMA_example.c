/*
 * USART_DMA_example.c
 *
 *  Created on: 11.05.2019
 *      Author: user
 */

#include <include/Peripherals/USART/USART_DMA.h>
#include <include/IRQn/DMA_IRQ.h>
#include <include/IRQn/USART_IRQ.h>

int main(void)
{

	/* clock configuration needed */

	volatile USART_DMA_Periph usart_debug = SOOL_USART_DMA_Init(USART2, 115200, 26);
	DMA_IRQ_Handler_SetUsartDebug(&usart_debug);
	USART_IRQ_Handler_SetUsartDebug(&usart_debug);
	// -------------------------------------------

	usart_debug.ActivateReading(&usart_debug);
	usart_debug.Send(&usart_debug, "123456\n\0");

	for ( int i = 0; i < 500000; i++ ) {
		if ( i == 40000 ) {

		}
	}

	usart_debug.ActivateReading(&usart_debug);
	usart_debug.Send(&usart_debug, "abcdefre\n\0");

	for ( int i = 0; i < 500000; i++ ) {
		if ( i == 40000 ) {

		}
	}

	usart_debug.ActivateReading(&usart_debug);
	usart_debug.Send(&usart_debug, "abcdefpoqwdopqwkdopdasdkaops\n\0");

	for ( int i = 0; i < 500000; i++ ) {
		if ( i == 40000 ) {

		}
	}

	usart_debug.ActivateReading(&usart_debug);
	usart_debug.Send(&usart_debug, "abcdefpo123qwdop45qwkdopdasd56ka123oadw1312ps\n\0");

	for ( int i = 0; i < 500000; i++ ) {
		if ( i == 40000 ) {

		}
	}

	const volatile Array_String* temp = usart_debug.GetRxData(&usart_debug);

}
