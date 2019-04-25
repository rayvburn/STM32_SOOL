/*
 * Potentiometer.h
 *
 *  Created on: 20.10.2018
 *      Author: user
 */

#ifndef POTENTIOMETER_H_
#define POTENTIOMETER_H_

#include "stm32f10x.h"

// hard-coded pins, port, etc.
#define POTENTIOMETER_ADC_CHANNEL		ADC_Channel_0
#define POTENTIOMETER_FILTERING_SAMPLES	128u

extern void 	Potentiometer_Config();
extern uint16_t Potentiometer_GetReading();
extern uint16_t Potentiometer_GetFilteredReading();

#endif /* POTENTIOMETER_H_ */
