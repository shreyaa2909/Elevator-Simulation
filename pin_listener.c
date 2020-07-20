/**
 * Program skeleton for the course "Programming embedded systems"
 *
 * Lab 1: the elevator control system
 */

/**
 * Functions listening for changes of specified pins
 */

#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include "pin_listener.h"
#include "assert.h"

#define DEBOUNCE_PERIOD (20 / portTICK_RATE_MS)         
u8 previous_state=0;
static void pollPin(PinListener *listener, xQueueHandle pinEventQueue) {
//
//0 - falling
//1 - pressed
//2 - rising
//3 - released
switch (listener->status) {
	
	case 0:
		 if (GPIO_ReadInputDataBit(listener->gpio,listener->pin))
		 {
			 listener->status=1;
			 printf("pressed\n");
		 }
		 break;
	case 1: 
		 if(GPIO_ReadInputDataBit(listener->gpio,listener->pin))
		 {
			 listener->status=2;
			 //printf("rising\n");
			 xQueueSendToBack(pinEventQueue, &listener->risingEvent, 0);
		
		 }
		 else
			{
				listener->status = 3;
			}
		 break;
	 case 2:
		 if(!GPIO_ReadInputDataBit(listener->gpio,listener->pin))
		 {
			 listener->status=3;
			 printf("released\n");
		 }
		 break;
	case 3:
		 if(!GPIO_ReadInputDataBit(listener->gpio,listener->pin))
			
		 {
			 listener->status=0;
			 //printf("falling\n");
			 xQueueSendToBack(pinEventQueue, &listener->fallingEvent,0);
			}
		 else
		 {
				listener->status = 1;
		 }

		 break;
	 }

}

static void pollPinsTask(void *params) {
  PinListenerSet listeners = *((PinListenerSet*)params);
  portTickType xLastWakeTime;
  int i;

  xLastWakeTime = xTaskGetTickCount();
	
  for (;;) {
    for (i = 0; i < listeners.num; ++i)
	
	  pollPin(listeners.listeners + i, listeners.pinEventQueue);
    
	vTaskDelayUntil(&xLastWakeTime, listeners.pollingPeriod);
  }
}

void setupPinListeners(PinListenerSet *listenerSet) {
  portBASE_TYPE res;

  res = xTaskCreate(pollPinsTask, "pin polling",
                    100, (void*)listenerSet,
					listenerSet->uxPriority, NULL);
  assert(res == pdTRUE);
}