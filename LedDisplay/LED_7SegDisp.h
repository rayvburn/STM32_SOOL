/*
 * 7SegDisp.h
 *
 *  Created on: 02.10.2018
 *      Author: user
 */

#ifndef LED_7SEGDISP_H_
#define LED_7SEGDISP_H_

#include <stdint.h>
#include "Globals.h"

/*
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#define LED_DISPLAY_BLINK_DOTS_ID 	10000u
#define LED_DISPLAY_BLINK_PERIOD  	1500u
#define LED_DISPLAY_MUX_PERIOD    	2u // [ms]
#define LED_DISPLAY_BLINK_INDICATOR ((LED_DISPLAY_BLINK_PERIOD)/(LED_DISPLAY_MUX_PERIOD))

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// prototypes
extern void 		LED_Display_GPIO_Init();
extern void 		LED_Display_Timer_Init();
extern void 		LED_Display_SetMuxedValue(const uint16_t value);
extern void			TIM2_IRQHandler();
extern void 		LED_Display_Handler(const uint16_t value, const uint8_t disp_nbr);
extern void 		LED_Display_TurnOnAllSeg();
extern void 		LED_Display_TurnOffAllSeg();
extern void 		LED_Display_TurnOffAllDisp();
extern void 		LED_BlinkDots_Handler(const uint8_t disp_nbr);
extern void 		LED_Display_ManageDot(const uint8_t state, const uint8_t disp_nbr);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
*/

#endif /* 7SEGDISP_H_ */
