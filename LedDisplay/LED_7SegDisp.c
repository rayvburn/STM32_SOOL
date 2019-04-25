/*
 * LED_7SegDisp.c
 *
 *  Created on: 02.10.2018
 *      Author: user
 */

#include "LED_7SegDisp.h"
#include "stm32f10x.h"

#include "USART.h"
/*
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

volatile static uint16_t value_to_disp = LED_DISPLAY_BLINK_DOTS_ID;    	// value to display variable
volatile static uint16_t timer_ticks_dots_blinking = 0;					// to handle on and off dots state

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void LED_Display_GPIO_Init() {

	// clock
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD, ENABLE);

	// configure GPIO
	GPIO_InitTypeDef gpio;
	GPIO_StructInit(&gpio);

	gpio.GPIO_Mode = GPIO_Mode_Out_PP;
	gpio.GPIO_Speed = GPIO_Speed_10MHz;

	// segments
	gpio.GPIO_Pin = LED_DISPLAY_SEG_A_PIN;
	GPIO_Init(LED_DISPLAY_SEG_A_PORT, &gpio);

	gpio.GPIO_Pin = LED_DISPLAY_SEG_B_PIN;
	GPIO_Init(LED_DISPLAY_SEG_B_PORT, &gpio);

	gpio.GPIO_Pin = LED_DISPLAY_SEG_C_PIN;
	GPIO_Init(LED_DISPLAY_SEG_C_PORT, &gpio);

	gpio.GPIO_Pin = LED_DISPLAY_SEG_D_PIN;
	GPIO_Init(LED_DISPLAY_SEG_D_PORT, &gpio);

	gpio.GPIO_Pin = LED_DISPLAY_SEG_E_PIN;
	GPIO_Init(LED_DISPLAY_SEG_E_PORT, &gpio);

	gpio.GPIO_Pin = LED_DISPLAY_SEG_F_PIN;
	GPIO_Init(LED_DISPLAY_SEG_F_PORT, &gpio);

	gpio.GPIO_Pin = LED_DISPLAY_SEG_G_PIN;
	GPIO_Init(LED_DISPLAY_SEG_G_PORT, &gpio);

	gpio.GPIO_Pin = LED_DISPLAY_SEG_DP_PIN;
	GPIO_Init(LED_DISPLAY_SEG_DP_PORT, &gpio);

	// displays
	gpio.GPIO_Pin = LED_DISPLAY_DISP1_PIN;
	GPIO_Init(LED_DISPLAY_DISP1_PORT, &gpio);

	gpio.GPIO_Pin = LED_DISPLAY_DISP2_PIN;
	GPIO_Init(LED_DISPLAY_DISP2_PORT, &gpio);

	gpio.GPIO_Pin = LED_DISPLAY_DISP3_PIN;
	GPIO_Init(LED_DISPLAY_DISP3_PORT, &gpio);

	gpio.GPIO_Pin = LED_DISPLAY_DISP4_PIN;
	GPIO_Init(LED_DISPLAY_DISP4_PORT, &gpio);

	// turn off
	LED_Display_TurnOffAllSeg();
	LED_Display_TurnOffAllDisp();

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void LED_Display_Timer_Init() {

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	// time base
	TIM_TimeBaseInitTypeDef tim;
	TIM_TimeBaseStructInit(&tim);
	tim.TIM_CounterMode = TIM_CounterMode_Up;

	// overflow after 2 ms with 72 MHz clock
	tim.TIM_Prescaler = 7200 - 1;
	tim.TIM_Period = 20 - 1;

	tim.TIM_ClockDivision = 0;
	tim.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM2, &tim);

	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
	TIM_Cmd(TIM2, ENABLE);

	NVIC_InitTypeDef nvic;
	nvic.NVIC_IRQChannel = TIM2_IRQn;
	nvic.NVIC_IRQChannelPreemptionPriority = 0;
	nvic.NVIC_IRQChannelSubPriority = 0;
	nvic.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvic);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

inline void LED_Display_SetMuxedValue(const uint16_t value) {
	value_to_disp = value;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void TIM2_IRQHandler() {

	if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET) {

		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);

		// multiplexing
		volatile static uint8_t disp_nbr = 1;
		volatile static uint8_t state_transition_flag = 0;

		if ( disp_nbr++ > 4 ) {
			disp_nbr = 1;
		}

		if ( value_to_disp != LED_DISPLAY_BLINK_DOTS_ID ) {

			if ( state_transition_flag == 0 ) {
				state_transition_flag = 1;
				LED_Display_TurnOffAllSeg();
			}

			LED_Display_Handler(value_to_disp, disp_nbr);


		} else if ( value_to_disp == LED_DISPLAY_BLINK_DOTS_ID ) {

			if ( state_transition_flag == 1 ) {
				state_transition_flag = 0;
				timer_ticks_dots_blinking = 0;
				LED_Display_TurnOffAllSeg();
			}
			UART_SendString("BLINK DOTS\n");
			LED_BlinkDots_Handler(disp_nbr);
			timer_ticks_dots_blinking++;

		}

	}

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void LED_Display_Handler(const uint16_t value, const uint8_t disp_nbr) {

	volatile uint8_t digit = 0;
	volatile uint8_t value_thous = (uint8_t)( value / 1000);
	volatile uint8_t value_hund =  (uint8_t)((value - value_thous * 1000)/100);
	volatile uint8_t value_tens =  (uint8_t)((value - value_thous * 1000 - value_hund * 100) / 10);
	volatile uint8_t value_unit =  (uint8_t)( value - value_thous * 1000 - value_hund * 100 - value_tens * 10);

	LED_Display_TurnOffAllDisp();
	LED_Display_TurnOffAllSeg();

	switch (disp_nbr) {
		case(1):
			digit = value_thous;
			GPIO_SetBits(LED_DISPLAY_DISP1_PORT, LED_DISPLAY_DISP1_PIN);
			break;
		case(2):
			digit = value_hund;
			GPIO_SetBits(LED_DISPLAY_DISP2_PORT, LED_DISPLAY_DISP2_PIN);
			break;
		case(3):
			digit = value_tens;
			GPIO_SetBits(LED_DISPLAY_DISP3_PORT, LED_DISPLAY_DISP3_PIN);
			break;
		case(4):
			digit = value_unit;
			GPIO_SetBits(LED_DISPLAY_DISP4_PORT, LED_DISPLAY_DISP4_PIN);
			break;
	}

	switch(digit) {

		case(0):
			GPIO_SetBits(LED_DISPLAY_SEG_A_PORT, LED_DISPLAY_SEG_A_PIN);
			GPIO_SetBits(LED_DISPLAY_SEG_B_PORT, LED_DISPLAY_SEG_B_PIN);
			GPIO_SetBits(LED_DISPLAY_SEG_C_PORT, LED_DISPLAY_SEG_C_PIN);
			GPIO_SetBits(LED_DISPLAY_SEG_D_PORT, LED_DISPLAY_SEG_D_PIN);
			GPIO_SetBits(LED_DISPLAY_SEG_E_PORT, LED_DISPLAY_SEG_E_PIN);
			GPIO_SetBits(LED_DISPLAY_SEG_F_PORT, LED_DISPLAY_SEG_F_PIN);
			break;
		case(1):
			GPIO_SetBits(LED_DISPLAY_SEG_B_PORT, LED_DISPLAY_SEG_B_PIN);
			GPIO_SetBits(LED_DISPLAY_SEG_C_PORT, LED_DISPLAY_SEG_C_PIN);
			break;
		case(2):
			GPIO_SetBits(LED_DISPLAY_SEG_A_PORT, LED_DISPLAY_SEG_A_PIN);
			GPIO_SetBits(LED_DISPLAY_SEG_B_PORT, LED_DISPLAY_SEG_B_PIN);
			GPIO_SetBits(LED_DISPLAY_SEG_G_PORT, LED_DISPLAY_SEG_G_PIN);
			GPIO_SetBits(LED_DISPLAY_SEG_E_PORT, LED_DISPLAY_SEG_E_PIN);
			GPIO_SetBits(LED_DISPLAY_SEG_D_PORT, LED_DISPLAY_SEG_D_PIN);
			break;
		case(3):
			GPIO_SetBits(LED_DISPLAY_SEG_A_PORT, LED_DISPLAY_SEG_A_PIN);
			GPIO_SetBits(LED_DISPLAY_SEG_B_PORT, LED_DISPLAY_SEG_B_PIN);
			GPIO_SetBits(LED_DISPLAY_SEG_G_PORT, LED_DISPLAY_SEG_G_PIN);
			GPIO_SetBits(LED_DISPLAY_SEG_C_PORT, LED_DISPLAY_SEG_C_PIN);
			GPIO_SetBits(LED_DISPLAY_SEG_D_PORT, LED_DISPLAY_SEG_D_PIN);
			break;
		case(4):
			GPIO_SetBits(LED_DISPLAY_SEG_F_PORT, LED_DISPLAY_SEG_F_PIN);
			GPIO_SetBits(LED_DISPLAY_SEG_G_PORT, LED_DISPLAY_SEG_G_PIN);
			GPIO_SetBits(LED_DISPLAY_SEG_B_PORT, LED_DISPLAY_SEG_B_PIN);
			GPIO_SetBits(LED_DISPLAY_SEG_C_PORT, LED_DISPLAY_SEG_C_PIN);
			break;
		case(5):
			GPIO_SetBits(LED_DISPLAY_SEG_A_PORT, LED_DISPLAY_SEG_A_PIN);
			GPIO_SetBits(LED_DISPLAY_SEG_F_PORT, LED_DISPLAY_SEG_F_PIN);
			GPIO_SetBits(LED_DISPLAY_SEG_G_PORT, LED_DISPLAY_SEG_G_PIN);
			GPIO_SetBits(LED_DISPLAY_SEG_C_PORT, LED_DISPLAY_SEG_C_PIN);
			GPIO_SetBits(LED_DISPLAY_SEG_D_PORT, LED_DISPLAY_SEG_D_PIN);
			break;
		case(6):
			GPIO_SetBits(LED_DISPLAY_SEG_A_PORT, LED_DISPLAY_SEG_A_PIN);
			GPIO_SetBits(LED_DISPLAY_SEG_F_PORT, LED_DISPLAY_SEG_F_PIN);
			GPIO_SetBits(LED_DISPLAY_SEG_E_PORT, LED_DISPLAY_SEG_E_PIN);
			GPIO_SetBits(LED_DISPLAY_SEG_D_PORT, LED_DISPLAY_SEG_D_PIN);
			GPIO_SetBits(LED_DISPLAY_SEG_C_PORT, LED_DISPLAY_SEG_C_PIN);
			GPIO_SetBits(LED_DISPLAY_SEG_G_PORT, LED_DISPLAY_SEG_G_PIN);
			break;
		case(7):
			GPIO_SetBits(LED_DISPLAY_SEG_A_PORT, LED_DISPLAY_SEG_A_PIN);
			GPIO_SetBits(LED_DISPLAY_SEG_B_PORT, LED_DISPLAY_SEG_B_PIN);
			GPIO_SetBits(LED_DISPLAY_SEG_C_PORT, LED_DISPLAY_SEG_C_PIN);
			break;
		case(8):
			GPIO_SetBits(LED_DISPLAY_SEG_A_PORT, LED_DISPLAY_SEG_A_PIN);
			GPIO_SetBits(LED_DISPLAY_SEG_B_PORT, LED_DISPLAY_SEG_B_PIN);
			GPIO_SetBits(LED_DISPLAY_SEG_C_PORT, LED_DISPLAY_SEG_C_PIN);
			GPIO_SetBits(LED_DISPLAY_SEG_D_PORT, LED_DISPLAY_SEG_D_PIN);
			GPIO_SetBits(LED_DISPLAY_SEG_E_PORT, LED_DISPLAY_SEG_E_PIN);
			GPIO_SetBits(LED_DISPLAY_SEG_F_PORT, LED_DISPLAY_SEG_F_PIN);
			GPIO_SetBits(LED_DISPLAY_SEG_G_PORT, LED_DISPLAY_SEG_G_PIN);
			break;
		case(9):
			GPIO_SetBits(LED_DISPLAY_SEG_A_PORT, LED_DISPLAY_SEG_A_PIN);
			GPIO_SetBits(LED_DISPLAY_SEG_B_PORT, LED_DISPLAY_SEG_B_PIN);
			GPIO_SetBits(LED_DISPLAY_SEG_G_PORT, LED_DISPLAY_SEG_G_PIN);
			GPIO_SetBits(LED_DISPLAY_SEG_F_PORT, LED_DISPLAY_SEG_F_PIN);
			GPIO_SetBits(LED_DISPLAY_SEG_C_PORT, LED_DISPLAY_SEG_C_PIN);
			GPIO_SetBits(LED_DISPLAY_SEG_D_PORT, LED_DISPLAY_SEG_D_PIN);
			break;
	}

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void LED_Display_TurnOnAllSeg() {
	GPIO_SetBits(LED_DISPLAY_SEG_A_PORT, LED_DISPLAY_SEG_A_PIN);
	GPIO_SetBits(LED_DISPLAY_SEG_B_PORT, LED_DISPLAY_SEG_B_PIN);
	GPIO_SetBits(LED_DISPLAY_SEG_C_PORT, LED_DISPLAY_SEG_C_PIN);
	GPIO_SetBits(LED_DISPLAY_SEG_D_PORT, LED_DISPLAY_SEG_D_PIN);
	GPIO_SetBits(LED_DISPLAY_SEG_E_PORT, LED_DISPLAY_SEG_E_PIN);
	GPIO_SetBits(LED_DISPLAY_SEG_F_PORT, LED_DISPLAY_SEG_F_PIN);
	GPIO_SetBits(LED_DISPLAY_SEG_G_PORT, LED_DISPLAY_SEG_G_PIN);
	GPIO_SetBits(LED_DISPLAY_SEG_DP_PORT, LED_DISPLAY_SEG_DP_PIN);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void LED_Display_TurnOffAllSeg() {
	GPIO_ResetBits(LED_DISPLAY_SEG_A_PORT, LED_DISPLAY_SEG_A_PIN);
	GPIO_ResetBits(LED_DISPLAY_SEG_B_PORT, LED_DISPLAY_SEG_B_PIN);
	GPIO_ResetBits(LED_DISPLAY_SEG_C_PORT, LED_DISPLAY_SEG_C_PIN);
	GPIO_ResetBits(LED_DISPLAY_SEG_D_PORT, LED_DISPLAY_SEG_D_PIN);
	GPIO_ResetBits(LED_DISPLAY_SEG_E_PORT, LED_DISPLAY_SEG_E_PIN);
	GPIO_ResetBits(LED_DISPLAY_SEG_F_PORT, LED_DISPLAY_SEG_F_PIN);
	GPIO_ResetBits(LED_DISPLAY_SEG_G_PORT, LED_DISPLAY_SEG_G_PIN);
	GPIO_ResetBits(LED_DISPLAY_SEG_DP_PORT, LED_DISPLAY_SEG_DP_PIN);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

inline void LED_Display_TurnOffAllDisp() {
	GPIO_ResetBits(LED_DISPLAY_DISP1_PORT, LED_DISPLAY_DISP1_PIN);
	GPIO_ResetBits(LED_DISPLAY_DISP2_PORT, LED_DISPLAY_DISP2_PIN);
	GPIO_ResetBits(LED_DISPLAY_DISP3_PORT, LED_DISPLAY_DISP3_PIN);
	GPIO_ResetBits(LED_DISPLAY_DISP4_PORT, LED_DISPLAY_DISP4_PIN);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void LED_BlinkDots_Handler(const uint8_t disp_nbr) {

  volatile static uint8_t current_state = 0;

  if ( timer_ticks_dots_blinking >= LED_DISPLAY_BLINK_INDICATOR ) {
	  timer_ticks_dots_blinking = 0;
      switch ( current_state ) {
        case(1):
          current_state = 0;
          break;
        case(0):
          current_state = 1;
          break;
      }
  }
  LED_Display_ManageDot(current_state, disp_nbr);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void LED_Display_ManageDot(const uint8_t state, const uint8_t disp_nbr) {

	LED_Display_TurnOffAllDisp();

	switch (disp_nbr) {
		case(1):
			GPIO_SetBits(LED_DISPLAY_DISP1_PORT, LED_DISPLAY_DISP1_PIN);
			break;
	  case(2):
			GPIO_SetBits(LED_DISPLAY_DISP2_PORT, LED_DISPLAY_DISP2_PIN);
			break;
	  case(3):
			GPIO_SetBits(LED_DISPLAY_DISP3_PORT, LED_DISPLAY_DISP3_PIN);
			break;
	  case(4):
			GPIO_SetBits(LED_DISPLAY_DISP4_PORT, LED_DISPLAY_DISP4_PIN);
			break;
	}

	switch (state) {
		case(1):
			GPIO_SetBits(LED_DISPLAY_SEG_DP_PORT, LED_DISPLAY_SEG_DP_PIN);
			break;
		case(0):
			GPIO_ResetBits(LED_DISPLAY_SEG_DP_PORT, LED_DISPLAY_SEG_DP_PIN);
			break;
	}

}
*/
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
