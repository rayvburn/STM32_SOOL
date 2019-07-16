/*
 * DMA.c
 *
 *  Created on: 10.07.2019
 *      Author: user
 */

#include "sool/Peripherals/DMA/DMA.h"
#include "sool/Peripherals/NVIC/NVIC.h"
#include "misc.h" // NVIC

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void SOOL_DMA_SetPeriphBaseAddr(volatile SOOL_DMA *dma, uint32_t addr);
static void SOOL_DMA_SetMemoryBaseAddr(volatile SOOL_DMA *dma, uint32_t addr);
static void SOOL_DMA_SetBufferSize(volatile SOOL_DMA *dma, uint32_t size);
static void SOOL_DMA_SetMemoryInc(volatile SOOL_DMA *dma, FunctionalState new_state);
static void SOOL_DMA_StartTransfer(volatile SOOL_DMA *dma);
static void SOOL_DMA_StopTransfer(volatile SOOL_DMA *dma);

static uint8_t SOOL_DMA_IsRunning(volatile SOOL_DMA *dma);

static void SOOL_DMA_EnableNVIC(volatile SOOL_DMA *dma);
static void SOOL_DMA_DisableNVIC(volatile SOOL_DMA *dma);

static uint8_t SOOL_DMA_ErrorInterruptHandler(volatile SOOL_DMA *dma);
static uint8_t SOOL_DMA_HalfInterruptHandler(volatile SOOL_DMA *dma);
static uint8_t SOOL_DMA_CompleteInterruptHandler(volatile SOOL_DMA *dma);
static uint8_t SOOL_DMA_GlobalInterruptHandler(volatile SOOL_DMA *dma);

// helper
void SOOL_DMA_UpdateSetup(volatile SOOL_DMA *dma, DMA_TypeDef *DMAy, DMA_Channel_TypeDef* DMAy_Channelx);
void SOOL_DMA_EnableInterrupts(volatile SOOL_DMA *dma, uint8_t enable_tc, uint8_t enable_ht, uint8_t enable_error);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

volatile SOOL_DMA SOOL_Periph_DMA_Init(DMA_TypeDef *DMAy, DMA_Channel_TypeDef* DMAy_Channelx,
		uint32_t DMA_DIR, uint32_t DMA_PeripheralInc, uint32_t DMA_MemoryInc,
		uint32_t DMA_PeripheralDataSize, uint32_t DMA_MemoryDataSize, uint32_t DMA_Mode,
		uint32_t DMA_Priority, uint32_t DMA_M2M,
		uint8_t enable_it_tc, uint8_t enable_it_ht, uint8_t enable_it_error) {

	/* New instance */
	volatile SOOL_DMA dma;

	/* Enable peripheral clock */
	if ( DMAy == DMA1 ) {
		RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	} else if ( DMAy == DMA2 ) {
		RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);
	}

	/* In fact in STM32F103C8T6 only DMA1 is available */
	SOOL_DMA_UpdateSetup(&dma, DMAy, DMAy_Channelx);

	/* Configure DMA peripheral */
	DMA_DeInit(DMAy_Channelx);                              							// clear DMA configuration
	DMA_ClearFlag(dma._setup.int_flags.ERROR_FLAG | dma._setup.int_flags.GLOBAL_FLAG | 	// clear interrupt flags
				  dma._setup.int_flags.HALF_FLAG  | dma._setup.int_flags.COMPLETE_FLAG);

	DMA_InitTypeDef dma_cfg;
	DMA_StructInit(&dma_cfg);

	dma_cfg.DMA_PeripheralBaseAddr = (uint32_t)0;						// to be set via member function
	dma_cfg.DMA_MemoryBaseAddr = (uint32_t)0;							// to be set via member function
	dma_cfg.DMA_BufferSize = 0;											// to be set via member function

	dma_cfg.DMA_DIR = DMA_DIR;											// transfer direction
	dma_cfg.DMA_PeripheralInc = DMA_PeripheralInc;						// auto-increment of address (peripheral's side)
	dma_cfg.DMA_MemoryInc = DMA_MemoryInc;								// auto-increment of address (buffer's side)
	dma_cfg.DMA_PeripheralDataSize = DMA_PeripheralDataSize;			// data size (peripheral)
	dma_cfg.DMA_MemoryDataSize = DMA_MemoryDataSize;					// data_size (buffer)
	dma_cfg.DMA_Mode = DMA_Mode;
	dma_cfg.DMA_Priority = DMA_Priority;
	dma_cfg.DMA_M2M = DMA_M2M;											// memory to memory setting

	DMA_Init(DMAy_Channelx, &dma_cfg);
	DMA_Cmd(DMAy_Channelx, DISABLE);

	/* Enable global interrupts for DMA */
	NVIC_InitTypeDef nvic;
	nvic.NVIC_IRQChannel = dma._setup.irqn;
	nvic.NVIC_IRQChannelCmd = DISABLE;
	nvic.NVIC_IRQChannelPreemptionPriority = 0;
	nvic.NVIC_IRQChannelSubPriority = 0;
	NVIC_Init(&nvic);

	/* Turn on interrupts */
	SOOL_DMA_EnableInterrupts(&dma, enable_it_tc, enable_it_ht, enable_it_error);

	/* Set member functions pointers */
	dma.SetPeriphBaseAddr = SOOL_DMA_SetPeriphBaseAddr;
	dma.SetMemoryBaseAddr = SOOL_DMA_SetMemoryBaseAddr;
	dma.SetBufferSize = SOOL_DMA_SetBufferSize;
	dma.SetMemoryInc = SOOL_DMA_SetMemoryInc;
	dma.Start = SOOL_DMA_StartTransfer;
	dma.Stop = SOOL_DMA_StopTransfer;

	dma.IsRunning = SOOL_DMA_IsRunning;

	dma.EnableNVIC = SOOL_DMA_EnableNVIC;
	dma.DisableNVIC = SOOL_DMA_DisableNVIC;

	dma._ErrorInterruptHandler = SOOL_DMA_ErrorInterruptHandler;
	dma._HalfInterruptHandler = SOOL_DMA_HalfInterruptHandler;
	dma._CompleteInterruptHandler = SOOL_DMA_CompleteInterruptHandler;
	dma._GlobalInterruptHandler = SOOL_DMA_GlobalInterruptHandler;

	return (dma);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void SOOL_DMA_SetPeriphBaseAddr(volatile SOOL_DMA *dma, uint32_t addr) {
	/* Update peripheral's Data Register address */
	dma->_setup.DMAy_Channelx->CPAR = (uint32_t)addr;
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static void SOOL_DMA_SetMemoryBaseAddr(volatile SOOL_DMA *dma, uint32_t addr) {
	/* Update memory base register address */
	dma->_setup.DMAy_Channelx->CMAR = (uint32_t)addr;
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static void SOOL_DMA_SetBufferSize(volatile SOOL_DMA *dma, uint32_t size) {
	/* Change buffer size (when big amount of data will come in parts) */
	dma->_setup.DMAy_Channelx->CNDTR = (uint32_t)size;
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static void SOOL_DMA_SetMemoryInc(volatile SOOL_DMA *dma, FunctionalState new_state) {

	/* Utilizes the fact that for each DMA channel MINC mask has the same value, for different
	 * MCUs than STM32F1 series it may not work */
	if ( new_state == ENABLE ) {
		dma->_setup.DMAy_Channelx->CCR |= (uint16_t)DMA_CCR1_MINC;
	} else {
		dma->_setup.DMAy_Channelx->CCR &= (uint16_t)(~(uint16_t)DMA_CCR1_MINC);
	}

}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static void SOOL_DMA_StartTransfer(volatile SOOL_DMA *dma) {
	/* Start DMA Channel's transfer */
	dma->_setup.DMAy_Channelx->CCR |= DMA_CCR1_EN;
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static void SOOL_DMA_StopTransfer(volatile SOOL_DMA *dma) {
	/* Disable DMA Channel*/
	dma->_setup.DMAy_Channelx->CCR &= (uint16_t)(~DMA_CCR1_EN);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t SOOL_DMA_IsRunning(volatile SOOL_DMA *dma) {
	// CCR is a `16-bit` register - after calculating bit value, conversion to uint8_t is needed
	if ( (dma->_setup.DMAy_Channelx->CCR & DMA_CCR1_EN) != ((uint16_t)0x00 )) {
		return (1);
	}
	return (0);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static void SOOL_DMA_EnableNVIC(volatile SOOL_DMA *dma) {
	SOOL_Periph_NVIC_Enable(dma->_setup.irqn);
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static void SOOL_DMA_DisableNVIC(volatile SOOL_DMA *dma) {
	SOOL_Periph_NVIC_Disable(dma->_setup.irqn);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t SOOL_DMA_ErrorInterruptHandler(volatile SOOL_DMA *dma) {

	if ( DMA_GetITStatus(dma->_setup.int_flags.ERROR_FLAG) == SET ) {

		/* Transfer Error Interrupt - happens for example when:
		 * 	- 	the peripheral register's address is wrong
		 *	- 	data couldn't be pushed due to the full buffer (RX-TX of MCU transfer)
		 */
		DMA_ClearITPendingBit(dma->_setup.int_flags.ERROR_FLAG);
//		DMA_ClearITPendingBit(dma->_setup.int_flags.GLOBAL_FLAG);
		// TODO: some error message

		return (1);

	}

	// indicate that no interrupt flag was recognized and cleared
	return (0);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t SOOL_DMA_HalfInterruptHandler(volatile SOOL_DMA *dma) {

	if ( DMA_GetITStatus(dma->_setup.int_flags.HALF_FLAG) == SET ) {

		/* Half Transfer Interrupt */
		DMA_ClearITPendingBit(dma->_setup.int_flags.HALF_FLAG);
//		DMA_ClearITPendingBit(dma->_setup.int_flags.GLOBAL_FLAG);
		return (1);

	}
	// indicate that no interrupt flag was recognized and cleared
	return (0);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t SOOL_DMA_CompleteInterruptHandler(volatile SOOL_DMA *dma) {

	 if ( DMA_GetITStatus(dma->_setup.int_flags.COMPLETE_FLAG) == SET ) {

			/* Transfer Complete Interrupt */
			DMA_ClearITPendingBit(dma->_setup.int_flags.COMPLETE_FLAG);
//			DMA_ClearITPendingBit(dma->_setup.int_flags.GLOBAL_FLAG);

			return (1);
	 }
	// indicate that no interrupt flag was recognized and cleared
	 return (0);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t SOOL_DMA_GlobalInterruptHandler(volatile SOOL_DMA *dma) {

	/* NOTE on interrupts: GLOBAL_FLAG clears all related interrupts i.e. TC, TE, HT;
	 * it may be a little faster to clear GLOBAL_FLAG at once than clearing 2 or more
	 * of them (with TC flag there is a HT flag active too) */

	if ( DMA_GetITStatus(dma->_setup.int_flags.GLOBAL_FLAG) == SET ) {

			/* Global DMA Channel Interrupt */
			DMA_ClearITPendingBit(dma->_setup.int_flags.GLOBAL_FLAG);
			return (1);

	}
	// indicate that no interrupt flag was recognized and cleared
	return (0);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void SOOL_DMA_UpdateSetup(volatile SOOL_DMA *dma, DMA_TypeDef *DMAy, DMA_Channel_TypeDef* DMAy_Channelx) {

	if ( DMAy == DMA1 ) {

		if (DMAy_Channelx == DMA1_Channel1) {

			dma->_setup.DMAy_Channelx = DMA1_Channel1;
			dma->_setup.irqn = DMA1_Channel1_IRQn;
			dma->_setup.int_flags.COMPLETE_FLAG = DMA1_FLAG_TC1;
			dma->_setup.int_flags.ERROR_FLAG = DMA1_FLAG_TE1;
			dma->_setup.int_flags.HALF_FLAG = DMA1_FLAG_HT1;
			dma->_setup.int_flags.GLOBAL_FLAG = DMA1_FLAG_GL1;

		} else if (DMAy_Channelx == DMA1_Channel2) {

			dma->_setup.DMAy_Channelx = DMA1_Channel2;
			dma->_setup.irqn = DMA1_Channel2_IRQn;
			dma->_setup.int_flags.COMPLETE_FLAG = DMA1_FLAG_TC2;
			dma->_setup.int_flags.ERROR_FLAG = DMA1_FLAG_TE2;
			dma->_setup.int_flags.HALF_FLAG = DMA1_FLAG_HT2;
			dma->_setup.int_flags.GLOBAL_FLAG = DMA1_FLAG_GL2;

		} else if (DMAy_Channelx == DMA1_Channel3) {

			dma->_setup.DMAy_Channelx = DMA1_Channel3;
			dma->_setup.irqn = DMA1_Channel3_IRQn;
			dma->_setup.int_flags.COMPLETE_FLAG = DMA1_FLAG_TC3;
			dma->_setup.int_flags.ERROR_FLAG = DMA1_FLAG_TE3;
			dma->_setup.int_flags.HALF_FLAG = DMA1_FLAG_HT3;
			dma->_setup.int_flags.GLOBAL_FLAG = DMA1_FLAG_GL3;

		} else if (DMAy_Channelx == DMA1_Channel4) {

			dma->_setup.DMAy_Channelx = DMA1_Channel4;
			dma->_setup.irqn = DMA1_Channel4_IRQn;
			dma->_setup.int_flags.COMPLETE_FLAG = DMA1_FLAG_TC4;
			dma->_setup.int_flags.ERROR_FLAG = DMA1_FLAG_TE4;
			dma->_setup.int_flags.HALF_FLAG = DMA1_FLAG_HT4;
			dma->_setup.int_flags.GLOBAL_FLAG = DMA1_FLAG_GL4;

		} else if (DMAy_Channelx == DMA1_Channel5) {

			dma->_setup.DMAy_Channelx = DMA1_Channel5;
			dma->_setup.irqn = DMA1_Channel5_IRQn;
			dma->_setup.int_flags.COMPLETE_FLAG = DMA1_FLAG_TC5;
			dma->_setup.int_flags.ERROR_FLAG = DMA1_FLAG_TE5;
			dma->_setup.int_flags.HALF_FLAG = DMA1_FLAG_HT5;
			dma->_setup.int_flags.GLOBAL_FLAG = DMA1_FLAG_GL5;

		} else if (DMAy_Channelx == DMA1_Channel6) {

			dma->_setup.DMAy_Channelx = DMA1_Channel6;
			dma->_setup.irqn = DMA1_Channel6_IRQn;
			dma->_setup.int_flags.COMPLETE_FLAG = DMA1_FLAG_TC6;
			dma->_setup.int_flags.ERROR_FLAG = DMA1_FLAG_TE6;
			dma->_setup.int_flags.HALF_FLAG = DMA1_FLAG_HT6;
			dma->_setup.int_flags.GLOBAL_FLAG = DMA1_FLAG_GL6;

		} else if (DMAy_Channelx == DMA1_Channel7) {

			dma->_setup.DMAy_Channelx = DMA1_Channel7;
			dma->_setup.irqn = DMA1_Channel7_IRQn;
			dma->_setup.int_flags.COMPLETE_FLAG = DMA1_FLAG_TC7;
			dma->_setup.int_flags.ERROR_FLAG = DMA1_FLAG_TE7;
			dma->_setup.int_flags.HALF_FLAG = DMA1_FLAG_HT7;
			dma->_setup.int_flags.GLOBAL_FLAG = DMA1_FLAG_GL7;

		}

	} else {

		/* Unsupported */

	}


}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void SOOL_DMA_EnableInterrupts(volatile SOOL_DMA *dma, uint8_t enable_tc, uint8_t enable_ht,
		uint8_t enable_error) {

	if ( enable_tc ) {
		DMA_ITConfig(dma->_setup.DMAy_Channelx, DMA_IT_TC, ENABLE);
	}
	if ( enable_ht ) {
		DMA_ITConfig(dma->_setup.DMAy_Channelx, DMA_IT_HT, ENABLE);
	}
	if ( enable_error ) {
		DMA_ITConfig(dma->_setup.DMAy_Channelx, DMA_IT_TE, ENABLE);
	}

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

