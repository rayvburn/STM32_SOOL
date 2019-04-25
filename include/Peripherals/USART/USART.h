/*
 * usart.h
 *
 *  Created on: 16.08.2018
 *      Author: user
 */

#ifndef USART_H_
#define USART_H_

// - - - - - - - - - - - - - - - -

// ST libs
#include "stm32f10x.h"
#include "stm32f10x_usart.h"
#include "misc.h"

// C libs
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// - - - - - - - - - - - - - - - -
#define USART_RX_BUFFER_LENGTH	4
// - - - - - - - - - - - - - - - -

typedef enum {
	MCU_COMMUNICATION = 0,
	DEBUGGING
} USART_ID;

// - - - - - - - - - - - - - - - -

typedef struct {
	USART_TypeDef*		usart_periph;
	uint32_t 			baudrate;
	NVIC_InitTypeDef 	nvic;
} USARTConfiguration;

// - - - - - - - - - - - - - - - -

typedef struct {
	uint16_t 			rx_buffer[USART_RX_BUFFER_LENGTH];
	uint8_t 			rx_buffer_counter;
	uint8_t 			new_data_flag;
} USARTReception;

// - - - - - - - - - - - - - - - -

typedef struct {
	USARTConfiguration 	config;
	USARTReception		reception;
} USARTSetup;

// - - - - - - - - - - - - - - - -

extern void USART_Config(USART_ID id, USART_TypeDef* usart_periph_id, uint32_t baud,
						 FunctionalState int_tx_en, FunctionalState int_rx_en,
						 FunctionalState remapped, FunctionalState state, FunctionalState nvic_en);
extern void USART_SwitchCmd(USART_ID id, FunctionalState state);

// transmit-specific functions
extern void 		USART_SendSigleChar(USART_ID id, char c);
extern void 		USART_SendString(USART_ID id, const char* str);
extern void 		USART_SendIntValue(USART_ID id, uint32_t value);
extern void 		USART_SendRxBuffer(USART_ID rx_buf_usart_id, USART_ID debug_usart_id);
extern uint8_t 		USART_GetRxBufferCounter(USART_ID id);

// reception-specific functions
extern void 		USART_ClearRxBuffer(USART_ID id);
extern uint8_t 		USART_DataAvailable(USART_ID id);

// project-specific functions
extern uint8_t 		USART_IsMessageValid(USART_ID id);
extern uint8_t 		USART_ProcessRxBufferData(USART_ID id);

// IRQHandlers, need to be adjusted in each application
extern void 		USART2_IRQHandler();
extern void 		USART1_IRQHandler();

// - - - - - - - - - - - - - - - -

#endif /* USART_H_ */
