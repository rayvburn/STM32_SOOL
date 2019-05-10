/*
 * Common.h
 *
 *  Created on: 10.05.2019
 *      Author: user
 */

#ifndef INCLUDE_PERIPHERALS_DMA_COMMON_H_
#define INCLUDE_PERIPHERALS_DMA_COMMON_H_

#include <stdint.h>

// - - - - - - - - - - - - - - - -

/* Capitals due to the fact that it should not be mutable;
 * for details see `@defgroup DMA_interrupts_definition`
 * in stm32f10x_dma.h */
typedef struct {
	uint32_t 				ERROR_FLAG;
	uint32_t				HALF_FLAG;
	uint32_t 				COMPLETE_FLAG;
	uint32_t				GLOBAL_FLAG;	// optional
} DMA_InterruptFlags;

// - - - - - - - - - - - - - - - -

#endif /* INCLUDE_PERIPHERALS_DMA_COMMON_H_ */
