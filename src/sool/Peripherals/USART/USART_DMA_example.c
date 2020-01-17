/*
 * USART_DMA_example.c
 *
 *  Created on: 11.05.2019
 *      Author: user
 */

#include <sool/IRQn/IRQ_DMA.h>
#include <sool/IRQn/IRQ_USART.h>
#include "sool/Peripherals/USART/USART_DMA.h"

int main(void)
{

	/* clock configuration needed */

	volatile SOOL_USART_DMA usart_dbg = SOOL_Periph_USART_DMA_Init(USART2, 115200, 10); // 26);

	/* Place handlers into proper global IRQHandlers */
	SOOL_IRQn_DMA_SetUsartDebug(&usart_dbg);
	SOOL_IRQn_USART_SetUsartDebug(&usart_dbg);
	// -------------------------------------------

	usart_dbg.ActivateReading(&usart_dbg);
	usart_dbg.Send(&usart_dbg, "123456\n\0");

	for ( int i = 0; i < 500000; i++ ) {
		if ( i == 40000 ) {

		}
	}

	usart_dbg.ActivateReading(&usart_dbg);
	usart_dbg.Send(&usart_dbg, "abcdefre\n\0");

	for ( int i = 0; i < 500000; i++ ) {
		if ( i == 40000 ) {

		}
	}

	usart_dbg.ActivateReading(&usart_dbg);
	usart_dbg.Send(&usart_dbg, "bacdefpoqwdopqwkdopdasdkaops\n\0");

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
	usart_dbg.Send(&usart_dbg, "123456789 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33\n");
	for ( int i = 0; i < 500000; i++ ) {
		if ( i == 40000 ) {

		}
	}

	usart_dbg.DeactivateReading(&usart_dbg);
	usart_dbg.Send(&usart_dbg, "poiuyewq");


	uint8_t line_was_busy = 0;
	if ( usart_dbg.IsTxLineBusy(&usart_dbg) ) {
		line_was_busy = 1;
		int abcd = 0;
		abcd++;
	}

	for ( int i = 0; i < 500000; i++ ) {
		if ( i == 40000 ) {

		}
	}

	usart_dbg.RestoreBuffersInitialSize(&usart_dbg, 2);
	usart_dbg.ActivateReading(&usart_dbg);
	SOOL_String str_test = SOOL_Memory_String_Init(15);
	str_test.SetString(&str_test, "tyrueiwoqpowieu");

	usart_dbg.Send(&usart_dbg, str_test._data);

	for ( int i = 0; i < 500000; i++ ) {
		if ( i == 40000 ) {

		}
	}

	uint8_t data_rec = usart_dbg.IsDataReceived(&usart_dbg);

	for ( int i = 0; i < 500000; i++ ) {
		if ( i == 40000 ) {

		}
	}

	const volatile SOOL_String* temp = usart_dbg.GetRxData(&usart_dbg);

	int abc = 0;
	abc++;
	abc++;

	data_rec = usart_dbg.IsDataReceived(&usart_dbg);
	abc++;
	abc++;

	usart_dbg.ClearRxBuffer(&usart_dbg);
	abc++;
	abc++;

	usart_dbg.RestoreBuffersInitialSize(&usart_dbg, 2);
	abc++;
	abc++;

	usart_dbg.Destroy(&usart_dbg);
	abc++;
	abc++;

	while (1) { }

	return (0);

}
