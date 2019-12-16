/*
 * ADC_common.h
 *
 *  Created on: 16.12.2019
 *      Author: user
 */

#ifndef INC_SOOL_PERIPHERALS_ADC_ADC_COMMON_H_
#define INC_SOOL_PERIPHERALS_ADC_ADC_COMMON_H_

#include "stdint.h"
#include "stm32f10x_gpio.h"

/**
 * @brief Converts ADC Channel (MCU specific) to the GPIO port and pin setup.
 * @param ADC_Channel
 * @param GPIO_Port: a value updated by a function if a proper ADC channel was given
 * @param GPIO_Pin: a value updated by a function if a proper ADC channel was given
 * @note Valid for STM32F103C8T6 (see doc for potential comparison and conflicts)
 */
extern void SOOL_ADC_Common_ConvertChannelToPortPin(uint8_t ADC_Channel, GPIO_TypeDef** GPIO_Port, uint16_t* GPIO_Pin);

#endif /* INC_SOOL_PERIPHERALS_ADC_ADC_COMMON_H_ */
