/*
 * ActionTimer.h
 *
 *  Created on: 21.05.2019
 *      Author: user
 */

#ifndef INC_SOOL_WORKFLOW_ACTIONTIMER_H_
#define INC_SOOL_WORKFLOW_ACTIONTIMER_H_

#include <stdint.h>

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

struct _SOOL_ActionTimerPrivateStruct {
	uint32_t 	time_start;
	uint32_t 	time_end;
	uint32_t 	time_diff;
	uint8_t 	active;
	uint8_t 	finished;
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/* Forward declaration */
typedef struct _SOOL_ActionTimerStruct SOOL_ActionTimer;

struct _SOOL_ActionTimerStruct {
	struct _SOOL_ActionTimerPrivateStruct _timer;
	void 		(*SetStartTime)(SOOL_ActionTimer*, const uint32_t);
	void 		(*SetEndTime)(SOOL_ActionTimer*, const uint32_t);
	uint8_t		(*IsActive)(SOOL_ActionTimer*);
	uint32_t 	(*GetTimeDiff)(SOOL_ActionTimer*);
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

extern SOOL_ActionTimer SOOL_Workflow_ActionTimer_Init();

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#endif /* INC_SOOL_WORKFLOW_ACTIONTIMER_H_ */
