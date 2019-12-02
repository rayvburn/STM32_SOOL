/*
 * IRQ_I2C.h
 *
 *  Created on: 02.12.2019
 *      Author: user
 */

#ifndef INC_SOOL_IRQN_IRQ_I2C_H_
#define INC_SOOL_IRQN_IRQ_I2C_H_

// ====================================================================================
/* I2C_IRQ header file template */
// ====================================================================================

/* add header files */
//#include <.h>

/* add some Setter function to copy the objects here */
extern void SOOL_IRQn_I2C_SetObj (volatile void *obj_ptr);

/* IRQHandlers declaration */
extern void I2C1_IRQHandler();		/* I2C1 global interrupt */
extern void I2C2_IRQHandler();		/* I2C2 global interrupt */

#endif /* INC_SOOL_IRQN_IRQ_I2C_H_ */
