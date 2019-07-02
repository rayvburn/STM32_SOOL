/*
 * GPIO_common.h
 *
 *  Created on: 02.07.2019
 *      Author: user
 */

#ifndef INC_SOOL_PERIPHERALS_GPIO_GPIO_COMMON_H_
#define INC_SOOL_PERIPHERALS_GPIO_GPIO_COMMON_H_

#include "stm32f10x.h"

 /**
  * @brief  Reads the specified input port pin.
  * @param  GPIOx: where x can be (A..G) to select the GPIO peripheral.
  * @param  GPIO_Pin:  specifies the port bit to read.
  *   This parameter can be GPIO_Pin_x where x can be (0..15).
  * @retval The input port pin value.
  */
extern uint8_t SOOL_Periph_GPIO_ReadInputDataBit(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);


/**
  * @brief  Reads the specified output data port bit.
  * @param  GPIOx: where x can be (A..G) to select the GPIO peripheral.
  * @param  GPIO_Pin:  specifies the port bit to read.
  *   This parameter can be GPIO_Pin_x where x can be (0..15).
  * @retval The output port pin value.
  */
extern uint8_t SOOL_Periph_GPIO_ReadOutputDataBit(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);


/**
  * @brief  Sets the selected data port bits.
  * @param  GPIOx: where x can be (A..G) to select the GPIO peripheral.
  * @param  GPIO_Pin: specifies the port bits to be written.
  *   This parameter can be any combination of GPIO_Pin_x where x can be (0..15).
  * @retval None
  */
extern void SOOL_Periph_GPIO_SetBits(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);


/**
  * @brief  Clears the selected data port bits.
  * @param  GPIOx: where x can be (A..G) to select the GPIO peripheral.
  * @param  GPIO_Pin: specifies the port bits to be written.
  *   This parameter can be any combination of GPIO_Pin_x where x can be (0..15).
  * @retval None
  */
extern void SOOL_Periph_GPIO_ResetBits(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);


#endif /* INC_SOOL_PERIPHERALS_GPIO_GPIO_COMMON_H_ */
