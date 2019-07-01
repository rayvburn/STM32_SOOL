/*
 * NVIC.c
 *
 *  Created on: 30.06.2019
 *      Author: user
 */

#include "sool/Peripherals/NVIC/NVIC.h"
#include "misc.h"

void SOOL_Periph_NVIC_Enable(uint8_t irq_channel) {

	// from ST's `misc.c`
	/* Enable the Selected IRQ Channels --------------------------------------*/
    NVIC->ISER[irq_channel >> 0x05] = (uint32_t)0x01 << (irq_channel & (uint8_t)0x1F);

}

void SOOL_Periph_NVIC_Disable(uint8_t irq_channel) {

	// from ST's `misc.c`
    /* Disable the Selected IRQ Channels -------------------------------------*/
    NVIC->ICER[irq_channel >> 0x05] = (uint32_t)0x01 << (irq_channel & (uint8_t)0x1F);

}
