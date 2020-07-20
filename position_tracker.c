/**
 * Program skeleton for the course "Programming embedded systems"
 *
 * Lab 1: the elevator control system
 */
/**
 * Class for keeping track of the car position.
 */
#include "FreeRTOS.h"
#include "task.h"

#include "position_tracker.h"
#include "semphr.h"
#include "assert.h"
	
	int cstate=0;
	int pstate=0;
	int inn=0;
static void positionTrackerTask(void *params) {
	PositionTracker *tracker = ((PositionTracker*)params);
	portTickType xLastWakeTime;
  xLastWakeTime = xTaskGetTickCount();
	pstate=GPIO_ReadInputDataBit(tracker->gpio,tracker->pin);
//	setCarTargetPosition(50);	
	for(;;)
	{		xSemaphoreTake(tracker->lock,portMAX_DELAY);
		if(inn)	
			pstate=cstate;
		cstate=GPIO_ReadInputDataBit(tracker->gpio,tracker->pin );
	
			if(cstate>pstate)
			{
				if(tracker->direction == Up)
				{ 
					tracker->position = tracker->position + 1;
					printf("Going up\n");
				}
				else if(tracker->direction == Down)
				{
					tracker->position = tracker->position - 1;
					printf("Going down\n");
				}
			}
			
			
			xSemaphoreGive(tracker->lock);
			inn=1;
	
	

  vTaskDelayUntil(&xLastWakeTime, tracker->pollingPeriod);
		}
}
void setupPositionTracker(PositionTracker *tracker,
                          GPIO_TypeDef * gpio, u16 pin,
						  portTickType pollingPeriod,
						  unsigned portBASE_TYPE uxPriority) {
  portBASE_TYPE res;

  tracker->position = 0;
  tracker->lock = xSemaphoreCreateMutex();
  assert(tracker->lock != NULL);
  tracker->direction = Unknown;
  tracker->gpio = gpio;
  tracker->pin = pin;
  tracker->pollingPeriod = pollingPeriod;

  res = xTaskCreate(positionTrackerTask, "position tracker",
                    80, (void*)tracker, uxPriority, NULL);
  assert(res == pdTRUE);
}
void setDirection(PositionTracker *tracker, Direction dir) {

  xSemaphoreTake(tracker->lock,portMAX_DELAY);
	tracker->direction = dir;
	xSemaphoreGive(tracker->lock);
	
	

}
s32 getPosition(PositionTracker *tracker) {
	vs32 pos;
	xSemaphoreTake(tracker->lock,portMAX_DELAY);
	pos=tracker->position;
	xSemaphoreGive(tracker->lock);
  // ...

  return pos;

}