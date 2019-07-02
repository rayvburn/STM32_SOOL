/*
 * GPIO_common.c
 *
 *  Created on: 02.07.2019
 *      Author: user
 */

#include "sool/Peripherals/GPIO/GPIO_common.h"


uint8_t SOOL_Periph_GPIO_ReadInputDataBit(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin) {
	  return ( (uint8_t)(GPIOx->IDR & (uint32_t)GPIO_Pin) );
}

uint8_t SOOL_Periph_GPIO_ReadOutputDataBit(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin) {
	return ( (uint8_t)(GPIOx->ODR & (uint32_t)GPIO_Pin) );
}

void SOOL_Periph_GPIO_SetBits(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin) {
	GPIOx->BSRR = GPIO_Pin;
}

void SOOL_Periph_GPIO_ResetBits(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin) {
	GPIOx->BRR = GPIO_Pin;
}
