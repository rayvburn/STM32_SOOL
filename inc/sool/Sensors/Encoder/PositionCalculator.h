/*
 * PositionCalculator.h
 *
 *  Created on: 05.02.2020
 *      Author: user
 */

#ifndef INC_SOOL_SENSORS_ENCODER_POSITIONCALCULATOR_H_
#define INC_SOOL_SENSORS_ENCODER_POSITIONCALCULATOR_H_

#include <stdint.h>

/**
 * @brief Implements a simple goal position calculator considering the current position
 * 	 and the `difference`. Does not handle cases where more than 65355 counts of a timer
 * 	 are required.
 * @note Timer counter register (TIM_CNT) is 16-bit
 * @param pos_current
 * @param pos_diff
 * @return
 */
extern uint16_t SOOL_Sensor_Encoder_PositionCalculator_ComputeGoal(uint16_t pos_current, int32_t pos_diff);

/**
 * @brief Calculates how many more pulses are required to reach the goal position.
 *   It assumes that the motor is rotating in the proper direction.
 * @param pos_current
 * @param goal
 * @return
 */
extern uint16_t SOOL_Sensor_Encoder_PositionCalculator_ComputeDiff(uint16_t pos_current, uint16_t pos_goal, uint8_t upcounting);


#endif /* INC_SOOL_SENSORS_ENCODER_POSITIONCALCULATOR_H_ */
