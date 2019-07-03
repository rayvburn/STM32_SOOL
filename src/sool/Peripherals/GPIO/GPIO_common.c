/*
 * GPIO_common.c
 *
 *  Created on: 02.07.2019
 *      Author: user
 */

#include "sool/Peripherals/GPIO/GPIO_common.h"


uint8_t SOOL_Periph_GPIO_ReadInputDataBit(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin) {

	if ( (GPIOx->IDR & GPIO_Pin) != (uint32_t)0x0000 ) {
		return (1);
	}
	return (0);

}

uint8_t SOOL_Periph_GPIO_ReadOutputDataBit(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin) {

	if ( (GPIOx->ODR & GPIO_Pin) != (uint32_t)0x0000 ) {
		return (1);
	}
	return (0);

}

void SOOL_Periph_GPIO_SetBits(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin) {
	GPIOx->BSRR = GPIO_Pin;
}

void SOOL_Periph_GPIO_ResetBits(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin) {
	GPIOx->BRR = GPIO_Pin;
}
