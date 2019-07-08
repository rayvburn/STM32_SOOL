/*
 * ADC_Channel.h
 *
 *  Created on: 08.07.2019
 *      Author: user
 */

#ifndef INC_SOOL_PERIPHERALS_ADC_ADC_CHANNEL_H_
#define INC_SOOL_PERIPHERALS_ADC_ADC_CHANNEL_H_

#include <stdint.h>

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/* Forward declaration */
struct _SOOL_ADC_ChannelStruct;
typedef struct _SOOL_ADC_ChannelStruct SOOL_ADC_Channel;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/** Provides basic periodical events handling feature */
struct _SOOL_ADC_ChannelStruct {
	uint8_t rank; 			// index in array of read values, this is set by an ADC initializer
	uint8_t sample_time;
	uint8_t adc_channel;
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// Analog input must be configured first using PinConfig_AltFunction - see pinout for
// Pin - ADC_Channel correspondence
extern SOOL_ADC_Channel SOOL_Periph_ADC_InitializeChannel(uint8_t ADC_Channel, uint8_t ADC_SampleTime);

#endif /* INC_SOOL_PERIPHERALS_ADC_ADC_CHANNEL_H_ */
