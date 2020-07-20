/**
 * Program skeleton for the course "Programming embedded systems"
 *
 * Lab 1: the elevator control system
 */

/**
 * The planner module, which is responsible for consuming
 * pin/key events, and for deciding where the elevator
 * should go next
 */

#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>

#include "global.h"
#include "planner.h"
#include "assert.h"
#include "pin_listener.h"
#define MOTOR_UPWARD   (TIM3->CCR1)
#define MOTOR_DOWNWARD (TIM3->CCR2)
#define MOTOR_STOPPED  (!MOTOR_UPWARD && !MOTOR_DOWNWARD)

static void plannerTask(void *params) {
  PinEvent p=0;
	PinEvent temp;
	PinEvent pnew;
	
	PinEvent ex[]={0,0,0,0,0,0,0,0,0,0};
	int i=1;
	int f = 1;
	int d = 0;
	int q[3] = { -1, -1, -1 };
	int position = 0;
	int j=1;
	int toAdd,k, af = 1,l=0;
	int x = 0;
	int w=1;
	portTickType timeArrivedAtFloor = xTaskGetTickCount();
	
	for(;;)
		{
	if(xQueueReceive(pinEventQueue,&p,10/portTICK_RATE_MS)==pdPASS)
	{	
		if(p == ARRIVED_AT_FLOOR)
		{	printf("Arrived at Floor\n");
				af = 1;
				timeArrivedAtFloor = xTaskGetTickCount();
		}
		if(p == STOP_PRESSED)
		{			printf("\n Stop button pressed\n");
					setCarMotorStopped(1);
		}
		if (p == STOP_RELEASED)
		{
					printf("\n Stop button released\n");
					setCarMotorStopped(0);
		}
		if(p == LEFT_FLOOR)
		{	printf("Left Floor\n");
					af = 0;
					l = 0;
		}
		
		if(p ==DOORS_CLOSED)
		{		printf("Door closed\n");
					d = 1;
					
		}
		if(p== DOORS_OPENING)
		{			printf("Door opening \n");
					d = 0;
		}
		if(((p==TO_FLOOR_1))||((p==TO_FLOOR_2))||((p==TO_FLOOR_3)))
					{
							toAdd = 1;
							for (k=j;k<10;k++) {
								if (ex[k] == p) {
									toAdd = 0;
									break;
								}
							}
							if (toAdd) {
								ex[i]=p;
								i++;
							}
		
				}
		temp=ex[j];
		if (af && d && (xTaskGetTickCount() - timeArrivedAtFloor) * portTICK_RATE_MS >= 1000 && !l) 
			{
		if(((temp==TO_FLOOR_1)||(temp==TO_FLOOR_2)||(temp==TO_FLOOR_3))&&( MOTOR_STOPPED))
			{
									pnew=temp;
									j++;
							if(pnew==TO_FLOOR_1)
								{
									setCarTargetPosition(0);
								}
							else if(pnew==TO_FLOOR_2)
								{
									setCarTargetPosition(400);
								}
							else if(pnew==TO_FLOOR_3)
								{
									setCarTargetPosition(800);
								}

								l=1;
		}
	}
		 
}
}

	vTaskDelay(portMAX_DELAY);
}


void setupPlanner(unsigned portBASE_TYPE uxPriority) {
  xTaskCreate(plannerTask, "planner", 100, NULL, uxPriority, NULL);
}
