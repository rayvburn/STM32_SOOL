/*
 * ACS711.h
 *
 *  Created on: 11.12.2019
 *      Author: user
 */

#ifndef INC_SOOL_SENSORS_ACS711_ACS711_H_
#define INC_SOOL_SENSORS_ACS711_ACS711_H_

#include <stdint.h>
#include <sool/Peripherals/ADC/ADC_DMA.h>
#include <sool/Peripherals/ADC/ADC_Channel.h>
#include <sool/Sensors/Button/Button.h>
#include <sool/Effectors/PinSwitch/PinSwitch.h>

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

struct _SOOL_ACS711SetupStruct {
	int32_t					max_current;	// scaling factor
	int32_t					min_current;
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

struct _SOOL_ACS711StateStruct {
	uint8_t 				overcurrent_occurred;
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

typedef struct _SOOL_ACS711Struct SOOL_ACS711;

struct _SOOL_ACS711Struct {

	SOOL_ADC_DMA*						base_adc_dma_ptr;	// for convenient reading acquisition (no need to pass ADC_DMA to function)
	SOOL_ADC_Channel					base_adc_channel;
	SOOL_Button							base_fault;
	SOOL_PinSwitch 						base_reset;

	struct _SOOL_ACS711SetupStruct		_setup;
	struct _SOOL_ACS711StateStruct		_state;

	void		(*Reset)(volatile SOOL_ACS711 *cs_ptr);
	uint8_t		(*DidFault)(volatile SOOL_ACS711 *cs_ptr);
	int32_t		(*GetCurrent)(volatile SOOL_ACS711 *cs_ptr);
	uint8_t 	(*_InterruptHandler)(volatile SOOL_ACS711 *cs_ptr);

};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// current_maximum: defines a maximum current (physical value in amps/milliamps)
		// raw ADC reading divider so the `GetCurrent` method returns human-readable data
// @note to reset the sensor the AOI403 P-MOSFET can be used
extern volatile SOOL_ACS711 SOOL_Sensors_ACS711_Init(uint8_t ADC_Channel, uint8_t ADC_SampleTime, volatile SOOL_ADC_DMA *adc_dma_ptr,
		GPIO_TypeDef* fault_port, uint16_t fault_pin, GPIO_TypeDef* reset_port, uint16_t reset_pin,
		int32_t current_minimum, int32_t current_maximum);

// remember to put interrupt handler (fault pin) into the proper EXTI interrupt handler
// does not handle ADC startup
extern void SOOL_Sensors_ACS711_Startup(volatile SOOL_ACS711* cs_ptr);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#endif /* INC_SOOL_SENSORS_ACS711_ACS711_H_ */
