/*
 * USART.c
 *
 *  Created on: 26.07.2018
 *      Author: user
 */

#include "USART.h"

// - - - - - - - - - - - - - - - -

volatile static USARTSetup* usart_ptr;
volatile static USARTSetup  usart_debug;
volatile static USARTSetup  usart_mcu;

// - - - - - - - - - - - - - - - -

static void USART_SetLocalPointer(USART_ID id);

// - - - - - - - - - - - - - - - -

void USART_Config(USART_ID id, USART_TypeDef* usart_periph_id, uint32_t baud,
		 		  FunctionalState int_tx_en, FunctionalState int_rx_en,
				  FunctionalState remapped, FunctionalState state, FunctionalState nvic_en) {

	GPIO_TypeDef* port;

	// TODO: remap handling, USART could go to PD/PC when remapped
	if ( usart_periph_id == USART1 || usart_periph_id == USART2 ) {
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
		port = GPIOA;
	} else if ( usart_periph_id == USART3 ) {
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
		port = GPIOB;
	}

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,  ENABLE);

	(usart_periph_id == USART1) ? (RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE)) : (0);
	(usart_periph_id == USART2) ? (RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE)) : (0);
	(usart_periph_id == USART3) ? (RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE)) : (0);

	uint16_t rx_pin, tx_pin;
	uint8_t irqn;

	// TODO: remap handling
	if ( usart_periph_id == USART1 ) {
		tx_pin = GPIO_Pin_9;
		rx_pin = GPIO_Pin_10;
		irqn = USART1_IRQn;
	} else if ( usart_periph_id == USART2 ) {
		tx_pin = GPIO_Pin_2;
		rx_pin = GPIO_Pin_3;
		irqn = USART2_IRQn;
	} else if ( usart_periph_id == USART3 ) {
		tx_pin = GPIO_Pin_10;
		rx_pin = GPIO_Pin_11;
		irqn = USART3_IRQn;
	}

	GPIO_InitTypeDef gpio;
	GPIO_StructInit(&gpio);
	gpio.GPIO_Pin = rx_pin;		// RX
	gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(port, &gpio);

	gpio.GPIO_Pin = tx_pin;		// TX
	gpio.GPIO_Mode = GPIO_Mode_AF_PP;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(port, &gpio);

	USART_InitTypeDef usart;
	USART_StructInit(&usart);
	usart.USART_BaudRate = baud;
	USART_Init(usart_periph_id, &usart);

	USART_ITConfig(usart_periph_id, USART_IT_RXNE, int_rx_en);
	USART_ITConfig(usart_periph_id, USART_IT_TXE,  int_tx_en);

	NVIC_InitTypeDef nvic;
	nvic.NVIC_IRQChannel = irqn;
	nvic.NVIC_IRQChannelPreemptionPriority = 0;
	nvic.NVIC_IRQChannelSubPriority = 0;
	nvic.NVIC_IRQChannelCmd = nvic_en;
	NVIC_Init(&nvic);

	USART_SetLocalPointer(id);
	usart_ptr->config.usart_periph = usart_periph_id;
	usart_ptr->config.baudrate = baud;
	usart_ptr->config.nvic = nvic;

	USART_ClearRxBuffer(id);
	USART_Cmd(usart_periph_id, state);

}

// - - - - - - - - - - - - - - - -

static void USART_SetLocalPointer(USART_ID id) {

	switch(id) {
	case(MCU_COMMUNICATION):
		usart_ptr = &usart_mcu;
		break;
	case(DEBUGGING):
		usart_ptr = &usart_debug;
		break;
	default:
		usart_ptr = 0;
		break;
	}

}

// - - - - - - - - - - - - - - - -

void USART_SwitchCmd(USART_ID id, FunctionalState state) {
	USART_SetLocalPointer(id);
	USART_Cmd(usart_ptr->config.usart_periph, state);
}

// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
// = = = = = = = = = = = = = = = = = =  TRANSMITTING = = = = = = = = = = = = = = = = = = = = = = = =
// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

void USART_SendSigleChar(USART_ID id, char c) {

	USART_SetLocalPointer(id);
	if ( c == '\n') {
		while (USART_GetFlagStatus(usart_ptr->config.usart_periph, USART_FLAG_TXE) == RESET);
		USART_SendData(usart_ptr->config.usart_periph, '\r');
	}
	while (USART_GetFlagStatus(usart_ptr->config.usart_periph, USART_FLAG_TXE) == RESET);
	USART_SendData(usart_ptr->config.usart_periph, c);

}

// - - - - - - - - - - - - - - - -

void USART_SendString(USART_ID id, const char* str) {
	while (*str) {
		USART_SendSigleChar(id, *str++); // pointer increment
	}
}

// - - - - - - - - - - - - - - - -

void USART_SendIntValue(USART_ID id, uint32_t value) {

	uint8_t mln 	  =  value / 1000000;
	uint8_t thous100  = (value - mln * 1000000) / 100000;
	uint8_t thous10   = (value - mln * 1000000 - thous100 * 100000) / 10000;
	uint8_t thous     = (value - mln * 1000000 - thous100 * 100000 - thous10 * 10000) / 1000;
	uint8_t hund      = (value - mln * 1000000 - thous100 * 100000 - thous10 * 10000 - thous * 1000) / 100;
	uint8_t tens      = (value - mln * 1000000 - thous100 * 100000 - thous10 * 10000 - thous * 1000 - hund * 100) / 10;
	uint8_t unities   = (value - mln * 1000000 - thous100 * 100000 - thous10 * 10000 - thous * 1000 - hund * 100 - tens * 10);

	char mln_char = mln + '0';
	char thous100_char = thous100 + '0';
	char thous10_char = thous10 + '0';
	char thous_char = thous + '0';
	char hund_char = hund + '0';
	char tens_char = tens +'0';
	char unities_char = unities + '0';

	if ( mln ) {

		USART_SendSigleChar(id, mln_char);
		USART_SendSigleChar(id, thous100_char);
		USART_SendSigleChar(id, thous10_char);
		USART_SendSigleChar(id, thous_char);
		USART_SendSigleChar(id, hund_char);
		USART_SendSigleChar(id, tens_char);
		USART_SendSigleChar(id, unities_char);

	} else if ( thous100 ) {

		USART_SendSigleChar(id, thous100_char);
		USART_SendSigleChar(id, thous10_char);
		USART_SendSigleChar(id, thous_char);
		USART_SendSigleChar(id, hund_char);
		USART_SendSigleChar(id, tens_char);
		USART_SendSigleChar(id, unities_char);

	} else if ( thous10 ) {

		USART_SendSigleChar(id, thous10_char);
		USART_SendSigleChar(id, thous_char);
		USART_SendSigleChar(id, hund_char);
		USART_SendSigleChar(id, tens_char);
		USART_SendSigleChar(id, unities_char);

	} else if ( thous ) {

		USART_SendSigleChar(id, thous_char);
		USART_SendSigleChar(id, hund_char);
		USART_SendSigleChar(id, tens_char);
		USART_SendSigleChar(id, unities_char);

	} else if ( hund ) {

		USART_SendSigleChar(id, thous_char); 	// dbg
		USART_SendSigleChar(id, hund_char);
		USART_SendSigleChar(id, tens_char);
		USART_SendSigleChar(id, unities_char);

	} else if ( tens ) {

		USART_SendSigleChar(id, thous_char); 	// dbg
		USART_SendSigleChar(id, hund_char);		// dbg
		USART_SendSigleChar(id, tens_char);
		USART_SendSigleChar(id, unities_char);

	} else if ( unities ) {

		USART_SendSigleChar(id, thous_char); 	// dbg
		USART_SendSigleChar(id, hund_char);		// dbg
		USART_SendSigleChar(id, tens_char); 	// dbg
		USART_SendSigleChar(id, unities_char);

	} else {

		USART_SendSigleChar(id, thous_char); 	// dbg
		USART_SendSigleChar(id, hund_char);		// dbg
		USART_SendSigleChar(id, tens_char); 	// dbg
		USART_SendSigleChar(id, unities_char);	// dbg

	}

}

// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
// = = = = = = = = = = = = = = = = = = = RECEIVING = = = = = = = = = = = = = = = = = = = = = = = = =
// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

// Message format:
// !message_id!
// messages id are enumerated in Globals.h

// - - - - - - - - - - - - - - - -

void USART_SendRxBuffer(USART_ID rx_buf_usart_id, USART_ID debug_usart_id) {

	USART_SetLocalPointer(rx_buf_usart_id);
	for ( uint8_t i = 0; i < USART_RX_BUFFER_LENGTH; i++ ) {
		// USART_SendSigleChar(debug_usart_id, (char)(usart_ptr->reception.rx_buffer[i]) );
		USART_SendSigleChar(debug_usart_id, (char)(*(usart_ptr->reception.rx_buffer+i)) );
		USART_SendSigleChar(debug_usart_id, '|');
	}

}

// - - - - - - - - - - - - - - - -

uint8_t USART_GetRxBufferCounter(USART_ID id) {
	USART_SetLocalPointer(id);
	return usart_ptr->reception.rx_buffer_counter;
}

// - - - - - - - - - - - - - - - -

void USART_ClearRxBuffer(USART_ID id) {

	USART_SetLocalPointer(id);
	memset(usart_ptr->reception.rx_buffer, 0, USART_RX_BUFFER_LENGTH*sizeof(uint16_t));
	usart_ptr->reception.rx_buffer_counter = 0;
	usart_ptr->reception.new_data_flag = 0;

}

// - - - - - - - - - - - - - - - -

uint8_t USART_DataAvailable(USART_ID id) {
	USART_SetLocalPointer(id);
	uint8_t temp = usart_ptr->reception.new_data_flag;
	usart_ptr->reception.new_data_flag = 0;
	return temp;
}

// - - - - - - - - - - - - - - - -

uint8_t USART_IsMessageValid(USART_ID id) {

	USART_SetLocalPointer(id);

	// dataframe is 4 chars long, starts with '!', second-to-last is '!'
	if ( ( *(usart_ptr->reception.rx_buffer) == '!' ) && ( *(usart_ptr->reception.rx_buffer+2) == '!') ) {
		return 1;
	} else {
		return 0;
	}

}

// - - - - - - - - - - - - - - - -

MCU_Message USART_ProcessRxBufferData(USART_ID id) {

	USART_SetLocalPointer(id);

	switch (*(usart_ptr->reception.rx_buffer+1)) {

	case ( (uint16_t)BULLET_FALLEN + '0' ):	// enum casting
		USART_ClearRxBuffer(id);
		return BULLET_FALLEN;
		break;

	default:
		USART_ClearRxBuffer(id);
		return UNKNOWN;
		break;

	}

}

// - - - - - - - - - - - - - - - -
// USART DEBUGGING IRQHandler
// - - - - - - - - - - - - - - - -
void USART2_IRQHandler() {

	USART_HandleInterrupt(&usart_debug);
/*
	if ( USART_GetITStatus(usart_debug.config.usart_periph, USART_IT_RXNE) != RESET ) {
		USART_ClearITPendingBit(usart_debug.config.usart_periph, USART_FLAG_RXNE);

		usart_debug.reception.rx_buffer[usart_debug.reception.rx_buffer_counter] = USART_ReceiveData(usart_debug.config.usart_periph);

		if ( usart_debug.reception.rx_buffer[usart_debug.reception.rx_buffer_counter] == '\n' ) {
			usart_debug.reception.new_data_flag = 1;		// means that data is complete and ready to read as a whole
		} else {
			usart_debug.reception.rx_buffer_counter++;
		}

	}
*/

}

// - - - - - - - - - - - - - - - -
// USART MCU COMMUNICATION IRQHandler
// - - - - - - - - - - - - - - - -
void USART1_IRQHandler() {

	USART_HandleInterrupt(&usart_mcu);
/*
	if ( USART_GetITStatus(usart_mcu.config.usart_periph, USART_IT_RXNE) != RESET ) {
		USART_ClearITPendingBit(usart_mcu.config.usart_periph, USART_FLAG_RXNE);

		usart_mcu.reception.rx_buffer[usart_mcu.reception.rx_buffer_counter] = USART_ReceiveData(usart_mcu.config.usart_periph);

		if ( usart_mcu.reception.rx_buffer[usart_mcu.reception.rx_buffer_counter] == '\n' ) {
			usart_mcu.reception.new_data_flag = 1;		// means that data is complete and ready to read as a whole
		} else {
			usart_mcu.reception.rx_buffer_counter++;
		}

	}
*/

}

static void USART_HandleInterrupt(USARTSetup *usart_setup_ptr) {

	if ( USART_GetITStatus(usart_setup_ptr->config.usart_periph, USART_IT_RXNE) != RESET ) {

		USART_ClearITPendingBit(usart_setup_ptr->config.usart_periph, USART_FLAG_RXNE);

		if ( !usart_setup_ptr->reception.new_data_flag ) {

			// to prevent receiving new data from buffer when old ones aren't read yet
			usart_setup_ptr->reception.rx_buffer[usart_setup_ptr->reception.rx_buffer_counter] = USART_ReceiveData(usart_setup_ptr->config.usart_periph);

		}

		if ( usart_setup_ptr->reception.rx_buffer[usart_setup_ptr->reception.rx_buffer_counter] == '\n' || 	// new line character
			 usart_setup_ptr->reception.rx_buffer[usart_setup_ptr->reception.rx_buffer_counter] == '\r' ) {	// carriage return character

			usart_setup_ptr->reception.new_data_flag = 1;				// means that data is complete and ready to read as a whole
			USART_ReceiveData(usart_setup_ptr->config.usart_periph); 	// in case of "\r\n" data ending

		} else {

			usart_setup_ptr->reception.rx_buffer_counter++;

		}

	}

}