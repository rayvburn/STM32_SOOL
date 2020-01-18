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
	int32_t					offset;
	int32_t					max_current;	// scaling factor
	int32_t					min_current;
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

struct _SOOL_ACS711StateStruct {
	uint8_t 				overcurrent_occurred;
	uint8_t					reset_started;
	uint32_t				reset_start_time;
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

	/**
	 * @brief Evaluates the overcurrent flag
	 * @param cs_ptr
	 * @return
	 */
	uint8_t		(*DidFault)(volatile SOOL_ACS711 *cs_ptr);

	/**
	 * @brief Resets the sensor
	 * @param cs_ptr
	 * @param state: ENABLE (power on again) or DISABLE (switch off the sensor)
	 */
	void		(*Reset)(volatile SOOL_ACS711 *cs_ptr, FunctionalState state);

	/**
	 * @brief Evaluates if the time required for sensor power
	 * to be cut off has elapsed. If true the sensor
	 * can be re-enabled @ref Reset(ENABLE)
	 * @param cs_ptr
	 * @return
	 */
	uint8_t		(*DidPowerOff)(volatile SOOL_ACS711 *cs_ptr);

	/**
	 * @brief Retrieves the ADC reading and scales it so the output is expressed
	 * in human-readable units (the same unit as the current range passed to the
	 * constructor).
	 * @details One probably will need to incorporate an averaging filter to acquire
	 * a meaningful data as the sensor is quite noisy (averaging 100 samples at 4 A
	 * gives +-75 mA accuracy) and seems not to be really linear (offset calibrated
	 * and error at 4 A is about 0,7 A, i.e. sensor reads 3,3 A).
	 * @param cs_ptr
	 * @return
	 */
	int32_t		(*GetCurrent)(volatile SOOL_ACS711 *cs_ptr);

	/**
	 * @brief Ran in the proper ISR
	 * @param cs_ptr
	 * @return
	 */
	uint8_t 	(*_InterruptHandler)(volatile SOOL_ACS711 *cs_ptr);

};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/**
 * @brief ACS711 current sensor controller
 * @param ADC_Channel
 * @param ADC_SampleTime
 * @param adc_dma_ptr
 * @param fault_port
 * @param fault_pin
 * @param reset_port
 * @param reset_pin
 * @param current_minimum
 * @param current_maximum: defines a maximum current (physical value in amps/milliamps)
 * @param offset: set as an average value returned by the @ref GetCurrent method while the `offset` is still set to 0 (i.e. calibrate the sensor)
 * @return
 * @note To reset the sensor the AOI403 P-MOSFET can be used
 * @note Requires SysTick timer (SOOL) to be enabled
 */
extern volatile SOOL_ACS711 SOOL_Sensors_ACS711_Init(uint8_t ADC_Channel, uint8_t ADC_SampleTime, volatile SOOL_ADC_DMA *adc_dma_ptr,
		GPIO_TypeDef* fault_port, uint16_t fault_pin, GPIO_TypeDef* reset_port, uint16_t reset_pin,
		int32_t current_minimum, int32_t current_maximum, int32_t offset);

/**
 * @brief Startup routine
 * @note Does not handle ADC startup
 * @note Remember to put interrupt handler (fault pin) into the proper EXTI interrupt handler
 * @param cs_ptr
 */
extern void SOOL_Sensors_ACS711_Startup(volatile SOOL_ACS711* cs_ptr);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// Example: https://gitlab.com/frb-pow/010FallingBulletLifterMCU/blob/analog_basic_devel/src/main.c

#endif /* INC_SOOL_SENSORS_ACS711_ACS711_H_ */
