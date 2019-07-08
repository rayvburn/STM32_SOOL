/*
 * ADC_Channel.c
 *
 *  Created on: 08.07.2019
 *      Author: user
 */

#include "sool/Peripherals/ADC/ADC_Channel.h"

SOOL_ADC_Channel SOOL_Periph_ADC_InitializeChannel(uint8_t adc_channel) {

	SOOL_ADC_Channel channel;
	channel.adc_channel = adc_channel;
	channel.rank = 0;
	return (channel);

}
