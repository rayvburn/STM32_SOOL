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

	volatile USART_DMA_Periph usart_debug = SOOL_Periph_USART_DMA_Init(USART2, 115200, 10); // 26);

	/* Place handlers into proper global IRQHandlers */
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
	usart_debug.Send(&usart_debug, "bacdefpoqwdopqwkdopdasdkaops\n\0");

	for ( int i = 0; i < 500000; i++ ) {
		if ( i == 40000 ) {

		}
	}

	/* Calling ActivateReading() is needed if you DO NOT want
	 * the following characters to append current string -
	 * it sets the buffer pointer back to the start
	 * WITHOUT clearing its contents */
	//	usart_debug.ActivateReading(&usart_debug);

	/* Clearing does not protect the following received characters against appending
	 * the current string - it just clears a whole buffer */
//	usart_debug.ClearRxBuffer(&usart_debug);

	/* Clearing the TX buffer is not needed as long as you are sure
	 * that previous data weren't longer (in terms of length) than
	 * the following (unnecessary characters need to be trimmed) */
	usart_debug.Send(&usart_debug, "123456789 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33\n");
	for ( int i = 0; i < 500000; i++ ) {
		if ( i == 40000 ) {

		}
	}

	usart_debug.DeactivateReading(&usart_debug);
	usart_debug.Send(&usart_debug, "poiuyewq");


	uint8_t line_was_busy = 0;
	if ( usart_debug.IsTxLineBusy(&usart_debug) ) {
		line_was_busy = 1;
		int abcd = 0;
		abcd++;
	}

	for ( int i = 0; i < 500000; i++ ) {
		if ( i == 40000 ) {

		}
	}

	usart_debug.RestoreBuffersInitialSize(&usart_debug);
	usart_debug.ActivateReading(&usart_debug);
	Array_String str_test = SOOL_Memory_Array_String_Init(15);
	str_test.SetString(&str_test, "tyrueiwoqpowieu");

	usart_debug.Send(&usart_debug, str_test._data);

	for ( int i = 0; i < 500000; i++ ) {
		if ( i == 40000 ) {

		}
	}

	uint8_t data_rec = usart_debug.IsDataReceived(&usart_debug);

	for ( int i = 0; i < 500000; i++ ) {
		if ( i == 40000 ) {

		}
	}

	const volatile Array_String* temp = usart_debug.GetRxData(&usart_debug);

	int abc = 0;
	abc++;
	abc++;

	data_rec = usart_debug.IsDataReceived(&usart_debug);
	abc++;
	abc++;

	usart_debug.ClearRxBuffer(&usart_debug);
	abc++;
	abc++;

	usart_debug.RestoreBuffersInitialSize(&usart_debug);
	abc++;
	abc++;

	usart_debug.Destroy(&usart_debug);
	abc++;
	abc++;

	while (1) { }

	return (0);

}
