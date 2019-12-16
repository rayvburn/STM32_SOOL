/*
 * ACS711.c
 *
 *  Created on: 11.12.2019
 *      Author: user
 */

#include <sool/Sensors/ACS711/ACS711.h>
#include <sool/Common/Delay.h>
#include <sool/Maths/Scaler.h>
#include <sool/Peripherals/ADC/ADC_common.h>

static void		SOOL_ACS711_Reset(volatile SOOL_ACS711 *cs_ptr);
static uint8_t	SOOL_ACS711_DidFault(volatile SOOL_ACS711 *cs_ptr);
static int32_t	SOOL_ACS711_GetCurrent(volatile SOOL_ACS711 *cs_ptr);
static uint8_t 	SOOL_ACS711_InterruptHandler(volatile SOOL_ACS711 *cs_ptr);

// ------------------------------------------------------------------------

volatile SOOL_ACS711 SOOL_Sensors_ACS711_Init(uint8_t ADC_Channel, uint8_t ADC_SampleTime, volatile SOOL_ADC_DMA *adc_dma_ptr,
		GPIO_TypeDef* fault_port, uint16_t fault_pin, GPIO_TypeDef* reset_port, uint16_t reset_pin,
		int32_t current_minimum, int32_t current_maximum)
{

	/* New instance */
	volatile SOOL_ACS711 current_sensor;

	/* Configure GPIO analog input pin */
	GPIO_TypeDef* gpio_port;
	uint16_t gpio_pin;
	SOOL_ADC_Common_ConvertChannelToPortPin(ADC_Channel, &gpio_port, &gpio_pin);
	SOOL_PinConfig_AltFunction current_sensor_gpio = SOOL_Periph_GPIO_PinConfig_Initialize_AltFunction(gpio_port, gpio_pin, GPIO_Mode_AIN);

	/* Configure base `classes` */
	current_sensor.base_adc_channel = SOOL_Periph_ADC_InitializeChannel(ADC_Channel, ADC_SampleTime);
	current_sensor.base_fault = SOOL_Sensor_Button_Init(SOOL_Periph_GPIO_PinConfig_Initialize_Int(fault_port, fault_pin, GPIO_Mode_IN_FLOATING, EXTI_Trigger_Falling));
	current_sensor.base_reset = SOOL_Effector_PinSwitch_Init(SOOL_Periph_GPIO_PinConfig_Initialize_NoInt(reset_port, reset_pin, GPIO_Mode_Out_PP));
	current_sensor.base_adc_dma_ptr = adc_dma_ptr;

	/* Interface the created ADC Channel with the SOOL_ADC_DMA instance */
	adc_dma_ptr->AddChannel(adc_dma_ptr, &current_sensor.base_adc_channel);

	/* Turn on the sensor */
	current_sensor.base_reset.SetHigh(&current_sensor.base_reset);

	/* State structure configuration */
	current_sensor._state.overcurrent_occurred = 0;

	/* Setup structure configuration */
	current_sensor._setup.min_current = current_minimum;
	current_sensor._setup.max_current = current_maximum;

	/* Pointers */
	current_sensor.DidFault = SOOL_ACS711_DidFault;
	current_sensor.GetCurrent = SOOL_ACS711_GetCurrent;
	current_sensor.Reset = SOOL_ACS711_Reset;
	current_sensor._InterruptHandler = SOOL_ACS711_InterruptHandler;

	return (current_sensor);

}

// ------------------------------------------------------------------------

void SOOL_Sensors_ACS711_Startup(volatile SOOL_ACS711* cs_ptr) {

	/* Fault pin startup */
	cs_ptr->base_fault.base.EnableEXTI(&cs_ptr->base_fault.base);
	cs_ptr->base_fault.base.EnableNVIC(&cs_ptr->base_fault.base);

}

// ------------------------------------------------------------------------

static void SOOL_ACS711_Reset(volatile SOOL_ACS711 *cs_ptr) {
	cs_ptr->base_reset.SetLow(&cs_ptr->base_reset);
	SOOL_Common_DelayUs(100, SystemCoreClock); 											// FIXME: adjust the threshold
	cs_ptr->base_reset.SetHigh(&cs_ptr->base_reset);
}
// ------------------------------------------------------------------------
static uint8_t SOOL_ACS711_DidFault(volatile SOOL_ACS711 *cs_ptr) {
	uint8_t temp = cs_ptr->_state.overcurrent_occurred;
	cs_ptr->_state.overcurrent_occurred = 0;
	return (temp);
}
// ------------------------------------------------------------------------
static int32_t SOOL_ACS711_GetCurrent(volatile SOOL_ACS711 *cs_ptr) {
	uint16_t reading = cs_ptr->base_adc_dma_ptr->GetReading(cs_ptr->base_adc_dma_ptr, cs_ptr->base_adc_channel);
	return (SOOL_Maths_Scale(reading, 													/* ADC value */
							 cs_ptr->_setup.min_current, cs_ptr->_setup.max_current, 	/* physical sensor measurement range (expressed in AMPS) */
							 0, 4095));													/* ADC range */
}
// ------------------------------------------------------------------------
static uint8_t SOOL_ACS711_InterruptHandler(volatile SOOL_ACS711 *cs_ptr) {
	// active low state trigger
	cs_ptr->_state.overcurrent_occurred = 1;
	return (1);
}
// ------------------------------------------------------------------------
