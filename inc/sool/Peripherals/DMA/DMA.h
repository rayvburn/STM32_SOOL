/*
 * DMA.h
 *
 *  Created on: 10.07.2019
 *      Author: user
 */

#ifndef INC_SOOL_PERIPHERALS_DMA_DMA_H_
#define INC_SOOL_PERIPHERALS_DMA_DMA_H_

#include "DMA_common.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/* Forward declaration */
typedef struct _SOOL_DMA_Struct SOOL_DMA;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

struct _SOOL_DMA_Struct {

	struct _SOOL_DMA_SetupStruct	_setup;

	// configuration
	void (*SetPeriphBaseAddr)(volatile SOOL_DMA*, uint32_t);
	void (*SetMemoryBaseAddr)(volatile SOOL_DMA*, uint32_t);
	void (*SetBufferSize)(volatile SOOL_DMA*, uint32_t);
	void (*SetMemoryInc)(volatile SOOL_DMA*, FunctionalState);

	/**
	 * @brief Starts transfer conducted by DMA channel (enables it)
	 * @param
	 */
	void (*Start)(volatile SOOL_DMA*);

	/**
	 * @brief Stops DMA channel job (disables it)
	 * @param
	 */
	void (*Stop)(volatile SOOL_DMA*);



	uint8_t (*IsRunning)(volatile SOOL_DMA*);

	void (*EnableNVIC)(volatile SOOL_DMA*);
	void (*DisableNVIC)(volatile SOOL_DMA*);

	// interrupt handlers
	uint8_t (*_ErrorInterruptHandler)(volatile SOOL_DMA*);
	uint8_t (*_HalfInterruptHandler)(volatile SOOL_DMA*);
	uint8_t (*_CompleteInterruptHandler)(volatile SOOL_DMA*);
	uint8_t (*_GlobalInterruptHandler)(volatile SOOL_DMA*);

};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/**
 * @brief Creates an instance of a class that allows to manage DMA Channel.
 * @note Some peripherals may interference with each other, see DMA requests for each channel table
 * in reference manual (RM0008, Table 78. Summary of DMA1 requests for each channel, p. 282).
 * @note Call SetPeriphBaseAddr(), SetMemoryBaseAddr() and SetBufferSize() before first transfer.
 * @note EnableNVIC must be called after locating SOOL_DMA object(s) in proper IRQHandlers.
 * @note It utilizes the fact that for each DMA Channel CCR register has the same flag for checking
 * whether it is running currently (DMA_CCR1_EN).
 * @param DMAy
 * @param DMAy_Channelx
 * @param DMA_DIR
 * @param DMA_PeripheralInc
 * @param DMA_MemoryInc
 * @param DMA_PeripheralDataSize
 * @param DMA_MemoryDataSize
 * @param DMA_Mode
 * @param DMA_Priority
 * @param DMA_M2M
 * @param enable_it_tc
 * @param enable_it_ht
 * @param enable_it_error
 * @return
 */
extern volatile SOOL_DMA SOOL_Periph_DMA_Init(DMA_TypeDef *DMAy, DMA_Channel_TypeDef* DMAy_Channelx,
						uint32_t DMA_DIR, uint32_t DMA_PeripheralInc, uint32_t DMA_MemoryInc,
						uint32_t DMA_PeripheralDataSize, uint32_t DMA_MemoryDataSize, uint32_t DMA_Mode,
						uint32_t DMA_Priority, uint32_t DMA_M2M,
						uint8_t enable_it_tc, uint8_t enable_it_ht, uint8_t enable_it_error);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#endif /* INC_SOOL_PERIPHERALS_DMA_DMA_H_ */
