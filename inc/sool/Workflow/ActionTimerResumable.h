/*
 * ActionTimerResumable.h
 *
 *  Created on: 15.10.2019
 *      Author: user
 */

#ifndef INC_SOOL_WORKFLOW_ACTIONTIMERRESUMABLE_H_
#define INC_SOOL_WORKFLOW_ACTIONTIMERRESUMABLE_H_

#include <stdint.h>
#include "sool/Memory/Vector/VectorUint32.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

struct _SOOL_ActionTimerResumablePrivateStruct {
	SOOL_Vector_Uint32 	start_v;
	SOOL_Vector_Uint32 	end_v;
	uint8_t 	active;
};

/* Forward declaration */
typedef struct _SOOL_ActionTimerResumableStruct SOOL_ActionTimerResumable;

struct _SOOL_ActionTimerResumableStruct {

	struct _SOOL_ActionTimerResumablePrivateStruct _timer;

	/**
	 * @brief Adds a new time stamp to the vector of start `times`.
	 * @param
	 * @param
	 * @note Numbers of start times must be equal to the number of end times.
	 */
	void 		(*AddStartTime)(SOOL_ActionTimerResumable*, const uint32_t);

	/**
	 * @brief Adds a new time stamp to the vector of end `times`.
	 * @param
	 * @param
	 * @note Numbers of start times must be equal to the number of end times.
	 */
	void 		(*AddEndTime)(SOOL_ActionTimerResumable*, const uint32_t);

	/**
	 * @brief Returns active flag which indicates that at least 1 start
	 * time stamp has been added.
	 * @param
	 * @return
	 */
	uint8_t		(*IsActive)(SOOL_ActionTimerResumable*);

	/**
	 * @brief Computes time difference according to an internal set of start and end time stamps.
	 * @param
	 * @return
	 * @note This is not a `getter` type function.
	 */
	uint32_t 	(*CalculateTimeDiff)(SOOL_ActionTimerResumable*);

	/**
	 * @brief Removes all time stamps stored in `start_v` and `end_v` vectors
	 * @param
	 */
	void 		(*Reset)(SOOL_ActionTimerResumable*);

};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

extern SOOL_ActionTimerResumable SOOL_Workflow_ActionTimerResumable_Init();

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#endif /* INC_SOOL_WORKFLOW_ACTIONTIMERRESUMABLE_H_ */
