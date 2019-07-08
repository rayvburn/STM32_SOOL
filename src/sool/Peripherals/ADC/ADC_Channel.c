/*
 * ADC_Channel.c
 *
 *  Created on: 08.07.2019
 *      Author: user
 */

#include "sool/Peripherals/ADC/ADC_Channel.h"

SOOL_ADC_Channel SOOL_Periph_ADC_InitializeChannel(uint8_t ADC_Channel, uint8_t ADC_SampleTime) {

	/* Create a new ADC channel */
	SOOL_ADC_Channel channel;

	/* Save internal values */
	channel.adc_channel = ADC_Channel;
	channel.sample_time = ADC_SampleTime;
	channel.rank = 0; // temp value, wrong in terms of channel initialization (1-16 are valid)

	return (channel);

}
