/*
 * MAX7219_symbols.h
 *
 *  Created on: 20.08.2019
 *      Author: user
 */

#ifndef INC_SOOL_IC_MAX7219_MAX7219_SYMBOLS_H_
#define INC_SOOL_IC_MAX7219_MAX7219_SYMBOLS_H_

///// Symbols are expressed as 16-bit values with 4 LSB zeroed.
///// This provides less operations needed to perform before
///// sending a value to MAX7219.
//#define MAX7219_SYMBOL_0		0x7E
//#define MAX7219_SYMBOL_1		0x30
//#define MAX7219_SYMBOL_2		0x6D
//#define MAX7219_SYMBOL_3		0x79
//#define MAX7219_SYMBOL_4		0x33
//#define MAX7219_SYMBOL_5		0x5B
//#define MAX7219_SYMBOL_6		0x5F
//#define MAX7219_SYMBOL_7		0x70
//#define MAX7219_SYMBOL_8		0x7F
//#define MAX7219_SYMBOL_9		0x7B
//#define MAX7219_SYMBOL_LINE		0xA0
//#define MAX7219_SYMBOL_BLANK 	0x


//	case('0'):
//			data = MAX7219_SYMBOL_0;
//			break;
//	case('1'):
//			data = MAX7219_SYMBOL_1;
//			break;
//	case('2'):
//			data = MAX7219_SYMBOL_2;
//			break;
//	case('3'):
//			data = MAX7219_SYMBOL_3;
//			break;
//	case('4'):
//			data = MAX7219_SYMBOL_4;
//			break;
//	case('5'):
//			data = MAX7219_SYMBOL_5;
//			break;
//	case('6'):
//			data = MAX7219_SYMBOL_6;
//			break;
//	case('7'):
//			data = MAX7219_SYMBOL_7;
//			break;
//	case('8'):
//			data = MAX7219_SYMBOL_8;
//			break;
//	case('9'):
//			data = MAX7219_SYMBOL_9;
//			break;

// see Table 5. Code B Font in the Maxim's datasheet
// bits D6-D4 are set to 0
//#define MAX7219_SYMBOL_0		0x00
//#define MAX7219_SYMBOL_1		0x01
//#define MAX7219_SYMBOL_2		0x02
//#define MAX7219_SYMBOL_3		0x03
//#define MAX7219_SYMBOL_4		0x04
//#define MAX7219_SYMBOL_5		0x05
//#define MAX7219_SYMBOL_6		0x06
//#define MAX7219_SYMBOL_7		0x07
//#define MAX7219_SYMBOL_8		0x08
//#define MAX7219_SYMBOL_9		0x09
#define MAX7219_SYMBOL_LINE		0x0A
#define MAX7219_SYMBOL_BLANK 	0x0F

#endif /* INC_SOOL_IC_MAX7219_MAX7219_SYMBOLS_H_ */
