/*
 * VL6180X.c
 *
 *  Created on: 08.10.2018
 *      Author: user
 */

#include "VL6180X.h"
#include "Systick_Timer.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// The Arduino two-wire interface uses a 7-bit number for the address, and sets the last bit correctly based on reads and writes
#define ADDRESS_DEFAULT 0b0101001

#define VL6180X_DEFAULT_ADDRESS	0x52



// RANGE_SCALER values for 1x, 2x, 3x scaling - see STSW-IMG003 core/src/vl6180x_api.c (ScalerLookUP[])
static uint16_t const VL6180X_ScalerValues[] = {0, 253, 127, 84};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint8_t 	address = VL6180X_DEFAULT_ADDRESS;
static uint8_t 	scaling = 0;
static uint8_t 	ptp_offset = 0;
static uint32_t io_timeout = 2; // number of tithings
static uint8_t 	did_timeout = 0;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void VL6180X_SetAddress(uint8_t new_addr)
{
	VL6180X_WriteReg(I2C_SLAVE__DEVICE_ADDRESS, new_addr & 0x7F);
	address = new_addr;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// Initialize sensor with settings from ST application note AN4545, section 9 -
// "Mandatory : private registers"
void VL6180X_Init()
{
  // Store part-to-part range offset so it can be adjusted if scaling is changed
  ptp_offset = VL6180X_ReadReg(SYSRANGE__PART_TO_PART_RANGE_OFFSET);

  if (VL6180X_ReadReg(SYSTEM__FRESH_OUT_OF_RESET) == 1)
  {
    scaling = 1;

    VL6180X_WriteReg(0x207, 0x01);
    VL6180X_WriteReg(0x208, 0x01);
    VL6180X_WriteReg(0x096, 0x00);
    VL6180X_WriteReg(0x097, 0xFD); // RANGE_SCALER = 253
    VL6180X_WriteReg(0x0E3, 0x00);
    VL6180X_WriteReg(0x0E4, 0x04);
    VL6180X_WriteReg(0x0E5, 0x02);
    VL6180X_WriteReg(0x0E6, 0x01);
    VL6180X_WriteReg(0x0E7, 0x03);
    VL6180X_WriteReg(0x0F5, 0x02);
    VL6180X_WriteReg(0x0D9, 0x05);
    VL6180X_WriteReg(0x0DB, 0xCE);
    VL6180X_WriteReg(0x0DC, 0x03);
    VL6180X_WriteReg(0x0DD, 0xF8);
    VL6180X_WriteReg(0x09F, 0x00);
    VL6180X_WriteReg(0x0A3, 0x3C);
    VL6180X_WriteReg(0x0B7, 0x00);
    VL6180X_WriteReg(0x0BB, 0x3C);
    VL6180X_WriteReg(0x0B2, 0x09);
    VL6180X_WriteReg(0x0CA, 0x09);
    VL6180X_WriteReg(0x198, 0x01);
    VL6180X_WriteReg(0x1B0, 0x17);
    VL6180X_WriteReg(0x1AD, 0x00);
    VL6180X_WriteReg(0x0FF, 0x05);
    VL6180X_WriteReg(0x100, 0x05);
    VL6180X_WriteReg(0x199, 0x05);
    VL6180X_WriteReg(0x1A6, 0x1B);
    VL6180X_WriteReg(0x1AC, 0x3E);
    VL6180X_WriteReg(0x1A7, 0x1F);
    VL6180X_WriteReg(0x030, 0x00);

    VL6180X_WriteReg(SYSTEM__FRESH_OUT_OF_RESET, 0);
  }
  else
  {
    // Sensor has already been initialized, so try to get scaling settings by
    // reading registers.

    uint16_t s = VL6180X_ReadReg16Bit(RANGE_SCALER);

    if      (s == VL6180X_ScalerValues[3]) 	{ scaling = 3; }
    else if (s == VL6180X_ScalerValues[2]) 	{ scaling = 2; }
    else                           			{ scaling = 1; }

    // Adjust the part-to-part range offset value read earlier to account for
    // existing scaling. If the sensor was already in 2x or 3x scaling mode,
    // precision will be lost calculating the original (1x) offset, but this can
    // be resolved by resetting the sensor and Arduino again.
    ptp_offset *= scaling;
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// Configure some settings for the sensor's default behavior from AN4545 -
// "Recommended : Public registers" and "Optional: Public registers"
//
// Note that this function does not set up GPIO1 as an interrupt output as
// suggested, though you can do so by calling:
// VL6180X_WriteReg(SYSTEM__MODE_GPIO1, 0x10);
void VL6180X_ConfigureDefault(void)
{
  // "Recommended : Public registers"

  // readout__averaging_sample_period = 48
  VL6180X_WriteReg(READOUT__AVERAGING_SAMPLE_PERIOD, 0x30);

  // sysals__analogue_gain_light = 6 (ALS gain = 1 nominal, actually 1.01 according to Table 14 in datasheet)
  VL6180X_WriteReg(SYSALS__ANALOGUE_GAIN, 0x46);

  // sysrange__vhv_repeat_rate = 255 (auto Very High Voltage temperature recalibration after every 255 range measurements)
  VL6180X_WriteReg(SYSRANGE__VHV_REPEAT_RATE, 0xFF);

  // sysals__integration_period = 99 (100 ms)
  // AN4545 incorrectly recommends writing to register 0x040; 0x63 should go in the lower byte, which is register 0x041.
  VL6180X_WriteReg16Bit(SYSALS__INTEGRATION_PERIOD, 0x0063);

  // sysrange__vhv_recalibrate = 1 (manually trigger a VHV recalibration)
  VL6180X_WriteReg(SYSRANGE__VHV_RECALIBRATE, 0x01);


  // "Optional: Public registers"

  // sysrange__intermeasurement_period = 9 (100 ms)
  VL6180X_WriteReg(SYSRANGE__INTERMEASUREMENT_PERIOD, 0x09);

  // sysals__intermeasurement_period = 49 (500 ms)
  VL6180X_WriteReg(SYSALS__INTERMEASUREMENT_PERIOD, 0x31);

  // als_int_mode = 4 (ALS new sample ready interrupt); range_int_mode = 4 (range new sample ready interrupt)
  VL6180X_WriteReg(SYSTEM__INTERRUPT_CONFIG_GPIO, 0x24);


  // Reset other settings to power-on defaults

  // sysrange__max_convergence_time = 49 (49 ms)
  VL6180X_WriteReg(SYSRANGE__MAX_CONVERGENCE_TIME, 0x31);

  // disable interleaved mode
  VL6180X_WriteReg(INTERLEAVED_MODE__ENABLE, 0);

  // reset range scaling factor to 1x
  VL6180X_SetScaling(1);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
// * * * * * * * * *  LOW-LEVEL FUNCTION THAT HAD TO BE ADJUSTED FOR STM32   * * * * * * * * * * *

// Writes an 8-bit register
void VL6180X_WriteReg(uint16_t reg, uint8_t value)
{

	I2C_GenerateStart_Blocking(I2C_PERIPH_ID);
	I2C_WriteSlave7BitAddress_Blocking(I2C_PERIPH_ID, VL6180X_DEFAULT_ADDRESS);
	I2C_SendData_Blocking(I2C_PERIPH_ID, (reg >> 8) & 0xff);	// reg high byte
	I2C_SendData_Blocking(I2C_PERIPH_ID, reg & 0xff);			// reg low byte
	I2C_SendData_Blocking(I2C_PERIPH_ID, value);				// value to send
	I2C_GenerateStop_Blocking(I2C_PERIPH_ID);

	/*
	I2C_GenerateStart_CheckFlag(I2C_PERIPH_ID);
	I2C_Send7BitAddress_CheckFlag(I2C_PERIPH_ID, address, I2C_Direction_Transmitter);
	I2C_SendData_CheckFlag(I2C_PERIPH_ID, (reg >> 8) & 0xff);	// reg high byte
	I2C_SendData_CheckFlag(I2C_PERIPH_ID, reg & 0xff);			// reg low byte
	I2C_SendData_CheckFlag(I2C_PERIPH_ID, value);				// value to send
	I2C_GenerateStop_CheckFlag(I2C_PERIPH_ID);
	*/

	/*// Arduino version
	Wire.beginTransmission(address);
	Wire.write((reg >> 8) & 0xff);  // reg high byte
	Wire.write(reg & 0xff);         // reg low byte
	Wire.write(value);
	last_status = Wire.endTransmission();
	 */
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
// * * * * * * * * *  LOW-LEVEL FUNCTION THAT HAD TO BE ADJUSTED FOR STM32   * * * * * * * * * * *


// Writes a 16-bit register
void VL6180X_WriteReg16Bit(uint16_t reg, uint16_t value)
{

	I2C_GenerateStart_Blocking(I2C_PERIPH_ID);
	I2C_WriteSlave7BitAddress_Blocking(I2C_PERIPH_ID, VL6180X_DEFAULT_ADDRESS);
	I2C_SendData_Blocking(I2C_PERIPH_ID, (reg >> 8) & 0xff);	// reg high byte
	I2C_SendData_Blocking(I2C_PERIPH_ID, reg & 0xff);			// reg low byte
	I2C_SendData_Blocking(I2C_PERIPH_ID, (value >> 8) & 0xff);	// value high byte
	I2C_SendData_Blocking(I2C_PERIPH_ID, value & 0xff);			// value low byte
	I2C_GenerateStop_Blocking(I2C_PERIPH_ID);


	/*
	I2C_GenerateStart_CheckFlag(I2C_PERIPH_ID);
	I2C_Send7BitAddress_CheckFlag(I2C_PERIPH_ID, address, I2C_Direction_Transmitter);
	I2C_SendData_CheckFlag(I2C_PERIPH_ID, (reg >> 8) & 0xff);	// reg high byte
	I2C_SendData_CheckFlag(I2C_PERIPH_ID, reg & 0xff);			// reg low byte
	I2C_SendData_CheckFlag(I2C_PERIPH_ID, (value >> 8) & 0xff);	// value high byte
	I2C_SendData_CheckFlag(I2C_PERIPH_ID, value & 0xff);		// value low byte
	I2C_GenerateStop_CheckFlag(I2C_PERIPH_ID);

	 */

	/*// Arduino version
	Wire.beginTransmission(address);
	Wire.write((reg >> 8) & 0xff);  // reg high byte
	Wire.write(reg & 0xff);         // reg low byte
	Wire.write((value >> 8) & 0xff);  // value high byte
	Wire.write(value & 0xff);         // value low byte
	last_status = Wire.endTransmission();
	*/
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
// * * * * * * * * *  LOW-LEVEL FUNCTION THAT HAD TO BE ADJUSTED FOR STM32   * * * * * * * * * * *


// Writes a 32-bit register
void VL6180X_WriteReg32Bit(uint16_t reg, uint32_t value)
{


	I2C_GenerateStart_Blocking(I2C_PERIPH_ID);
	I2C_WriteSlave7BitAddress_Blocking(I2C_PERIPH_ID, VL6180X_DEFAULT_ADDRESS);
	I2C_SendData_Blocking(I2C_PERIPH_ID, (reg >> 8) & 0xff);	// reg high byte
	I2C_SendData_Blocking(I2C_PERIPH_ID, reg & 0xff);			// reg low byte
	I2C_SendData_Blocking(I2C_PERIPH_ID, (value >> 24) & 0xff); // value highest byte
	I2C_SendData_Blocking(I2C_PERIPH_ID, (value >> 16) & 0xff);
	I2C_SendData_Blocking(I2C_PERIPH_ID, (value >> 8) & 0xff);
	I2C_SendData_Blocking(I2C_PERIPH_ID, value & 0xff);			// value lowest byte
	I2C_GenerateStop_Blocking(I2C_PERIPH_ID);


	/*
	I2C_GenerateStart_CheckFlag(I2C_PERIPH_ID);
	I2C_Send7BitAddress_CheckFlag(I2C_PERIPH_ID, address, I2C_Direction_Transmitter);
	I2C_SendData_CheckFlag(I2C_PERIPH_ID, (reg >> 8) & 0xff);	// reg high byte
	I2C_SendData_CheckFlag(I2C_PERIPH_ID, reg & 0xff);			// reg low byte
	I2C_SendData_CheckFlag(I2C_PERIPH_ID, (value >> 24) & 0xff);// value highest byte
	I2C_SendData_CheckFlag(I2C_PERIPH_ID, (value >> 16) & 0xff);
	I2C_SendData_CheckFlag(I2C_PERIPH_ID, (value >> 8)  & 0xff);
	I2C_SendData_CheckFlag(I2C_PERIPH_ID, value & 0xff);		// value lowest byte
	I2C_GenerateStop_CheckFlag(I2C_PERIPH_ID);
	 */

	/* // Arduino version
	Wire.beginTransmission(address);
	Wire.write((reg >> 8) & 0xff);  // reg high byte
	Wire.write(reg & 0xff);         // reg low byte
	Wire.write((value >> 24) & 0xff); // value highest byte
	Wire.write((value >> 16) & 0xff);
	Wire.write((value >> 8) & 0xff);
	Wire.write(value & 0xff);         // value lowest byte
	last_status = Wire.endTransmission();
	*/
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
// * * * * * * * * *  LOW-LEVEL FUNCTION THAT HAD TO BE ADJUSTED FOR STM32   * * * * * * * * * * *

// Reads an 8-bit register
uint8_t VL6180X_ReadReg(uint16_t reg)
{

	uint16_t data_buffer = 0;

	I2C_GenerateStart_Blocking(I2C_PERIPH_ID);
	I2C_WriteSlave7BitAddress_Blocking(I2C_PERIPH_ID, VL6180X_DEFAULT_ADDRESS);
	I2C_SendData_Blocking(I2C_PERIPH_ID, (reg >> 8) & 0xff);	// reg high byte
	I2C_SendData_Blocking(I2C_PERIPH_ID, reg & 0xff);			// reg low byte
	I2C_GenerateStop_Blocking(I2C_PERIPH_ID);

	I2C_InitializeReading_Blocking(I2C_PERIPH_ID, VL6180X_DEFAULT_ADDRESS);
	I2C_HandleReadingEvents_Blocking(I2C_PERIPH_ID, 1);
	I2C_ReadData_Blocking(I2C_PERIPH_ID, &data_buffer, 1);
	I2C_GenerateStop_Blocking(I2C_PERIPH_ID);


	/*
	// send an address of the register to read from
	I2C_GenerateStart_CheckFlag(I2C_PERIPH_ID);
	I2C_Send7BitAddress_CheckFlag(I2C_PERIPH_ID, address, I2C_Direction_Transmitter);
	I2C_SendData_CheckFlag(I2C_PERIPH_ID, (reg >> 8) & 0xff);	// reg high byte
	I2C_SendData_CheckFlag(I2C_PERIPH_ID, reg & 0xff);			// reg low byte
	I2C_GenerateStop_CheckFlag(I2C_PERIPH_ID);

	// receive data from slave
	I2C_GenerateStart_CheckFlag(I2C_PERIPH_ID);
	I2C_Send7BitAddress_CheckFlag(I2C_PERIPH_ID, address, I2C_Direction_Receiver);
	data_buffer = I2C_ReceiveData_CheckFlag(I2C_PERIPH_ID);
	I2C_GenerateStop_CheckFlag(I2C_PERIPH_ID);
	*/

	return (uint8_t)data_buffer;

	/* // Arduino version
	Wire.beginTransmission(address);
	Wire.write((reg >> 8) & 0xff);  // reg high byte
	Wire.write(reg & 0xff);         // reg low byte
	last_status = Wire.endTransmission();

	Wire.requestFrom(address, (uint8_t)1);
	value = Wire.read();
	Wire.endTransmission();
	return data_buffer;
	*/

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
// * * * * * * * * *  LOW-LEVEL FUNCTION THAT HAD TO BE ADJUSTED FOR STM32   * * * * * * * * * * *

// Reads a 16-bit register
uint16_t VL6180X_ReadReg16Bit(uint16_t reg)
{

	uint16_t data_buffer = 0;
	uint16_t temp_buffer[2] = {0,0};

	I2C_GenerateStart_Blocking(I2C_PERIPH_ID);
	I2C_WriteSlave7BitAddress_Blocking(I2C_PERIPH_ID, VL6180X_DEFAULT_ADDRESS);
	I2C_SendData_Blocking(I2C_PERIPH_ID, (reg >> 8) & 0xff);	// reg high byte
	I2C_SendData_Blocking(I2C_PERIPH_ID, reg & 0xff);			// reg low byte

	I2C_GenerateStop_Blocking(I2C_PERIPH_ID);
	I2C_InitializeReading_Blocking(I2C_PERIPH_ID, VL6180X_DEFAULT_ADDRESS);
	I2C_HandleReadingEvents_Blocking(I2C_PERIPH_ID, 2);
	I2C_ReadData_Blocking(I2C_PERIPH_ID, temp_buffer, 2);
	I2C_GenerateStop_Blocking(I2C_PERIPH_ID);

	data_buffer  = (uint8_t)temp_buffer[0] << 8;
	data_buffer |= (uint8_t)temp_buffer[1];

	/*
	// send an address of the register to read from
	I2C_GenerateStart_CheckFlag(I2C_PERIPH_ID);
	I2C_Send7BitAddress_CheckFlag(I2C_PERIPH_ID, address, I2C_Direction_Transmitter);
	I2C_SendData_CheckFlag(I2C_PERIPH_ID, (reg >> 8) & 0xff);	// reg high byte
	I2C_SendData_CheckFlag(I2C_PERIPH_ID, reg & 0xff);			// reg low byte
	I2C_GenerateStop_CheckFlag(I2C_PERIPH_ID);

	// receive data from slave
	I2C_GenerateStart_CheckFlag(I2C_PERIPH_ID);
	I2C_Send7BitAddress_CheckFlag(I2C_PERIPH_ID, address, I2C_Direction_Receiver);
	data_buffer  = I2C_ReceiveData_CheckFlag(I2C_PERIPH_ID) << 8;										// shift MSByte
	data_buffer |= I2C_ReceiveData_CheckFlag(I2C_PERIPH_ID);
	I2C_GenerateStop_CheckFlag(I2C_PERIPH_ID);
	*/

	return data_buffer;

	/* // Arduino version
	Wire.beginTransmission(address);
	Wire.write((reg >> 8) & 0xff);  // reg high byte
	Wire.write(reg & 0xff);         // reg low byte
	last_status = Wire.endTransmission();

	Wire.requestFrom(address, (uint8_t)2);
	value = (uint16_t)Wire.read() << 8; // value high byte
	value |= Wire.read();               // value low byte
	Wire.endTransmission();
	return value;
	*/

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
// * * * * * * * * *  LOW-LEVEL FUNCTION THAT HAD TO BE ADJUSTED FOR STM32   * * * * * * * * * * *

// Reads a 32-bit register
uint32_t VL6180X_ReadReg32Bit(uint16_t reg)
{

	uint32_t data_buffer = 0;
	uint16_t temp_buffer[4] = {0,0,0,0};

	I2C_GenerateStart_Blocking(I2C_PERIPH_ID);
	I2C_WriteSlave7BitAddress_Blocking(I2C_PERIPH_ID, VL6180X_DEFAULT_ADDRESS);
	I2C_SendData_Blocking(I2C_PERIPH_ID, (reg >> 8) & 0xff);	// reg high byte
	I2C_SendData_Blocking(I2C_PERIPH_ID, reg & 0xff);			// reg low byte

	I2C_GenerateStop_Blocking(I2C_PERIPH_ID);
	I2C_InitializeReading_Blocking(I2C_PERIPH_ID, VL6180X_DEFAULT_ADDRESS);
	I2C_HandleReadingEvents_Blocking(I2C_PERIPH_ID, 4);
	I2C_ReadData_Blocking(I2C_PERIPH_ID, temp_buffer, 4);
	I2C_GenerateStop_Blocking(I2C_PERIPH_ID);

	data_buffer  = (uint8_t)temp_buffer[0] << 24;
	data_buffer |= (uint8_t)temp_buffer[1] << 16;
	data_buffer |= (uint8_t)temp_buffer[2] << 8;
	data_buffer |= (uint8_t)temp_buffer[3];

	/*
	// send an address of the register to read from
	I2C_GenerateStart_CheckFlag(I2C_PERIPH_ID);
	I2C_Send7BitAddress_CheckFlag(I2C_PERIPH_ID, address, I2C_Direction_Transmitter);
	I2C_SendData_CheckFlag(I2C_PERIPH_ID, (reg >> 8) & 0xff);	// reg high byte
	I2C_SendData_CheckFlag(I2C_PERIPH_ID, reg & 0xff);			// reg low byte
	I2C_GenerateStop_CheckFlag(I2C_PERIPH_ID);

	// receive data from slave
	I2C_GenerateStart_CheckFlag(I2C_PERIPH_ID);
	I2C_Send7BitAddress_CheckFlag(I2C_PERIPH_ID, address, I2C_Direction_Receiver);
	data_buffer  = I2C_ReceiveData_CheckFlag(I2C_PERIPH_ID) << 24;	// get and shift MSByte
	data_buffer |= I2C_ReceiveData_CheckFlag(I2C_PERIPH_ID) << 16;
	data_buffer |= I2C_ReceiveData_CheckFlag(I2C_PERIPH_ID) << 8;
	data_buffer |= I2C_ReceiveData_CheckFlag(I2C_PERIPH_ID);		// get LSByte
	I2C_GenerateStop_CheckFlag(I2C_PERIPH_ID);
	*/

	return data_buffer;

	/* // Arduino version
	uint32_t value;

	Wire.beginTransmission(address);
	Wire.write((reg >> 8) & 0xff);  // reg high byte
	Wire.write(reg & 0xff);         // reg low byte
	last_status = Wire.endTransmission();

	Wire.requestFrom(address, (uint8_t)4);
	value = (uint32_t)Wire.read() << 24;  // value highest byte
	value |= (uint32_t)Wire.read() << 16;
	value |= (uint16_t)Wire.read() << 8;
	value |= Wire.read();                 // value lowest byte
	Wire.endTransmission();

	return value;
	*/
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// Set range scaling factor. The sensor uses 1x scaling by default, giving range
// measurements in units of mm. Increasing the scaling to 2x or 3x makes it give
// raw values in units of 2 mm or 3 mm instead. In other words, a bigger scaling
// factor increases the sensor's potential maximum range but reduces its
// resolution.

// Implemented using ST's VL6180X API as a reference (STSW-IMG003); see
// VL6180x_UpscaleSetScaling() in vl6180x_api.c.

void VL6180X_SetScaling(uint8_t new_scaling)
{
  uint8_t const DefaultCrosstalkValidHeight = 20; // default value of SYSRANGE__CROSSTALK_VALID_HEIGHT

  // do nothing if scaling value is invalid
  if (new_scaling < 1 || new_scaling > 3) { return; }

  scaling = new_scaling;
  VL6180X_WriteReg16Bit(RANGE_SCALER, VL6180X_ScalerValues[scaling]);

  // apply scaling on part-to-part offset
  VL6180X_WriteReg(SYSRANGE__PART_TO_PART_RANGE_OFFSET, ptp_offset / scaling);

  // apply scaling on CrossTalkValidHeight
  VL6180X_WriteReg(SYSRANGE__CROSSTALK_VALID_HEIGHT, DefaultCrosstalkValidHeight / scaling);

  // This function does not apply scaling to RANGE_IGNORE_VALID_HEIGHT.

  // enable early convergence estimate only at 1x scaling
  uint8_t rce = VL6180X_ReadReg(SYSRANGE__RANGE_CHECK_ENABLES);
  VL6180X_WriteReg(SYSRANGE__RANGE_CHECK_ENABLES, (rce & 0xFE) | (scaling == 1));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

inline uint8_t VL6180X_GetScaling(void) { return scaling; }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// Performs a single-shot ranging measurement
uint8_t VL6180X_ReadRangeSingle()
{
  VL6180X_WriteReg(SYSRANGE__START, 0x01);
  return VL6180X_ReadRangeContinuous();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

inline uint16_t	VL6180X_ReadRangeSingleMillimeters(void) {
	return (uint16_t)scaling * VL6180X_ReadRangeSingle();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// Performs a single-shot ambient light measurement
uint16_t VL6180X_ReadAmbientSingle()
{
  VL6180X_WriteReg(SYSALS__START, 0x01);
  return VL6180X_ReadAmbientContinuous();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// Starts continuous ranging measurements with the given period in ms
// (10 ms resolution; defaults to 100 ms if not specified).
//
// The period must be greater than the time it takes to perform a
// measurement. See section 2.4.4 ("Continuous mode limits") in the datasheet
// for details.
void VL6180X_StartRangeContinuous(uint16_t period)
{
  // default period value was 100
  int16_t period_reg = (int16_t)(period / 10) - 1;
  period_reg = VL6180X_Constrain(period_reg, 0, 254);

  VL6180X_WriteReg(SYSRANGE__INTERMEASUREMENT_PERIOD, period_reg);
  VL6180X_WriteReg(SYSRANGE__START, 0x03);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// Starts continuous ambient light measurements with the given period in ms
// (10 ms resolution; defaults to 500 ms if not specified).
//
// The period must be greater than the time it takes to perform a
// measurement. See section 2.4.4 ("Continuous mode limits") in the datasheet
// for details.
void VL6180X_StartAmbientContinuous(uint16_t period)
{
  // default period value was 500
  int16_t period_reg = (int16_t)(period / 10) - 1;
  period_reg = VL6180X_Constrain(period_reg, 0, 254);

  VL6180X_WriteReg(SYSALS__INTERMEASUREMENT_PERIOD, period_reg);
  VL6180X_WriteReg(SYSALS__START, 0x03);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// Starts continuous interleaved measurements with the given period in ms
// (10 ms resolution; defaults to 500 ms if not specified). In this mode, each
// ambient light measurement is immediately followed by a range measurement.
//
// The datasheet recommends using this mode instead of running "range and ALS
// continuous modes simultaneously (i.e. asynchronously)".
//
// The period must be greater than the time it takes to perform both
// measurements. See section 2.4.4 ("Continuous mode limits") in the datasheet
// for details.
void VL6180X_StartInterleavedContinuous(uint16_t period)
{
  // default period value was 500
  int16_t period_reg = (int16_t)(period / 10) - 1;
  period_reg = VL6180X_Constrain(period_reg, 0, 254);

  VL6180X_WriteReg(INTERLEAVED_MODE__ENABLE, 1);
  VL6180X_WriteReg(SYSALS__INTERMEASUREMENT_PERIOD, period_reg);
  VL6180X_WriteReg(SYSALS__START, 0x03);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// Stops continuous mode. This will actually start a single measurement of range
// and/or ambient light if continuous mode is not active, so it's a good idea to
// wait a few hundred ms after calling this function to let that complete
// before starting continuous mode again or taking a reading.
void VL6180X_StopContinuous()
{

  VL6180X_WriteReg(SYSRANGE__START, 0x01);
  VL6180X_WriteReg(SYSALS__START, 0x01);

  VL6180X_WriteReg(INTERLEAVED_MODE__ENABLE, 0);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// Returns a range reading when continuous mode is activated
// (readRangeSingle() also calls this function after starting a single-shot
// range measurement)
uint8_t VL6180X_ReadRangeContinuous()
{
  uint16_t counter = 0;
  while ((VL6180X_ReadReg(RESULT__INTERRUPT_STATUS_GPIO) & 0x04) == 0)
  {
  	counter++;
    if (io_timeout > 0 && counter > io_timeout)
    {
      did_timeout = 1;
      return 255;
    }
  }

  uint8_t range = VL6180X_ReadReg(RESULT__RANGE_VAL);
  VL6180X_WriteReg(SYSTEM__INTERRUPT_CLEAR, 0x01);

  return range;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

inline uint16_t VL6180X_ReadRangeContinuousMillimeters(void) {
	return (uint16_t)scaling * VL6180X_ReadRangeContinuous();
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// Returns an ambient light reading when continuous mode is activated
// (readAmbientSingle() also calls this function after starting a single-shot
// ambient light measurement)
uint16_t VL6180X_ReadAmbientContinuous()
{

  uint32_t time_start = SysTick_GetTithingsOfSec();
  while ((VL6180X_ReadReg(RESULT__INTERRUPT_STATUS_GPIO) & 0x20) == 0)
  {
    if (io_timeout > 0 && (SysTick_GetTithingsOfSec() - time_start) > io_timeout)
    {
      did_timeout = 1;
      return 0;
    }
  }

  uint16_t ambient = VL6180X_ReadReg16Bit(RESULT__ALS_VAL);
  VL6180X_WriteReg(SYSTEM__INTERRUPT_CLEAR, 0x02);

  return ambient;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

inline void VL6180X_SetTimeout(uint16_t timeout) {
	io_timeout = timeout;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

inline uint16_t VL6180X_GetTimeout(void) {
	return io_timeout;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// Did a timeout occur in one of the read functions since the last call to
// timeoutOccurred()?
uint8_t VL6180X_TimeoutOccurred()
{
	uint8_t tmp = did_timeout;
	did_timeout = 0;
	return tmp;
}
