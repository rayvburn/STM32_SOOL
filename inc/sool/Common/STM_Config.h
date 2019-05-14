/*
 * StmConfig.h
 *
 *  Created on: 03.10.2018
 *      Author: user
 */

#ifndef COMMON_STMCONFIG_H_
#define COMMON_STMCONFIG_H_

#include "stm32f10x.h"
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/** STM Clock configuration TEMPLATE
 * with default settings (72 MHz for F103C8T6) */
extern void SOOL_Common_ClockConfigDefault();

/** JTAG Remapping - fully disable JTAG, leave SWD enabled */
extern void SOOL_Common_DisableJTAG();

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#endif /* COMMON_STMCONFIG_H_ */
