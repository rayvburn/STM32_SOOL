/*
 * NVIC.h
 *
 *  Created on: 30.06.2019
 *      Author: user
 */

#ifndef INC_SOOL_PERIPHERALS_NVIC_NVIC_H_
#define INC_SOOL_PERIPHERALS_NVIC_NVIC_H_

#include <stdint.h>

extern void SOOL_Periph_NVIC_Enable(uint8_t irq_channel);
extern void SOOL_Periph_NVIC_Disable(uint8_t irq_channel);

#endif /* INC_SOOL_PERIPHERALS_NVIC_NVIC_H_ */
