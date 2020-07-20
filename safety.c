/**
 * Program skeleton for the course "Programming embedded systems"
 *
 * Lab 1: the elevator control system
 */

/**
 * This file defines the safety module, which observes the running
 * elevator system and is able to stop the elevator in critical
 * situations
 */

#include "FreeRTOS.h"
#include "task.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_gpio.h"
#include <stdio.h>

#include "global.h"
#include "assert.h"

#define POLL_TIME (10 / portTICK_RATE_MS)
#define c1  GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_0)
#define c2  GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_1)
#define c3  GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_2)
#define MOTOR_UPWARD   (TIM3->CCR1)
#define MOTOR_DOWNWARD (TIM3->CCR2)
#define MOTOR_STOPPED  (!MOTOR_UPWARD && !MOTOR_DOWNWARD)

#define STOP_PRESSED  GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_3)
#define AT_FLOOR      GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_7)
#define DOORS_CLOSED  GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_8)

s32 lastpos;
s32 presentpos,diff,position;
#define f1  0
#define f2  400 
#define f3  800
static portTickType xLastWakeTime;
static int getCurrentDirection() {
	if (MOTOR_STOPPED)
		return 0;
	if (MOTOR_UPWARD)
		return 1;
	return -1;
}

static void check(u8 assertion, char *name) {
  if (!assertion) {
    printf("SAFETY REQUIREMENT %s VIOLATED: STOPPING ELEVATOR\n", name);
    for (;;) {
			
	  setCarMotorStopped(1);
  	  vTaskDelayUntil(&xLastWakeTime, POLL_TIME);
    }
  }
}

static void safetyTask(void *params) {
  s16 timeSinceStopPressed = -1;
	s16 timeSinceAtFloor = -1;
	s16 timeSinceatfloorandclosed = -1;
	int x = 0, y = 1, z=1;
	int s = 1,n=0,af=-1;
	int direct = getCurrentDirection(); 
	portTickType xLastWakeTime1 = xTaskGetTickCount(); 
  xLastWakeTime = xTaskGetTickCount();
	//GPIO_WriteBit(GPIOC, GPIO_Pin_7,Bit_SET);
  for (;;) {
    // Environment assumption 1: the doors can only be opened if
	//                           the elevator is at a floor and
    //                           the motor is not active

check((AT_FLOOR && MOTOR_STOPPED) || DOORS_CLOSED,"env1");

	// environment assumption 2:The elevator moves at a maximum speed of 50cm/s by checking the position of the car.

	presentpos=getCarPosition();
	diff=presentpos-lastpos;	
	lastpos=presentpos;

 check((abs(diff)<50),"env2");

 // environment assumption 3:If the ground floor is put at 0cm in an absolute coordinate system, the second floor is at 400cm and the third floor at 800cm (the at-floor sensor reports a floor with a threshold of +-0.5cm)

	position = getCarPosition();
		check(!MOTOR_STOPPED || STOP_PRESSED || !AT_FLOOR ||(AT_FLOOR && MOTOR_STOPPED &&((position >= (0 - 1) && position <= (0 + 1)) ||
			(position >= (400 - 1) && position <= (400 + 1)) || (position >= (800 - 1) && position <= (800 + 1)))), "env3");
	
// environment assumption 4:if we call from any floor while elevator is moving, it dosents stops

if ((c1 || c2 || c3) && !MOTOR_STOPPED && !s) {
			check(direct == getCurrentDirection(), "env4");
		} else if (s && !MOTOR_STOPPED) {
			s = 0;
			direct = getCurrentDirection();
		} else if (!s && MOTOR_STOPPED) {
			s = 1;
		}


    //System requirement 1: if the stop button is pressed, the motor is
	//                       stopped within 1s

	if (STOP_PRESSED) {
	  if (timeSinceStopPressed < 0)
	    timeSinceStopPressed = 0;
      else
	    timeSinceStopPressed += POLL_TIME;

      check(timeSinceStopPressed * portTICK_RATE_MS <= 1000 || MOTOR_STOPPED,
	        "req1");
	} else {
	  timeSinceStopPressed = -1;
	}

    // System requirement 2: the motor signals for upwards and downwards
	//                       movement are not active at the same time

    check(!MOTOR_UPWARD || !MOTOR_DOWNWARD,"req2");

	// safety requirement 3:The elevator may not pass the end positions, that is, go through the roof or the floor
		position = getCarPosition();
		check(position >= 0 && position <= 800, "req3");

	// safety requirement 4:A moving elevator halts only if the stop button is pressed or the elevator has arrived at a floor
	if (STOP_PRESSED) 
		{
		x = 1;
		}
	else if (!MOTOR_STOPPED) 
		{
		x = 0;
		}
	check(!MOTOR_STOPPED || (MOTOR_STOPPED && AT_FLOOR) || (MOTOR_STOPPED && x), "req4");

	// safety requirement 5:Once the elevator has stopped at a floor, it will wait for at least 1s before it continues to another floor
	if (!MOTOR_STOPPED && y == 1) 
		{
		y = 0;
		 if (z || (xTaskGetTickCount() * portTICK_RATE_MS) - (xLastWakeTime1 * portTICK_RATE_MS) >= 1000) 
			 {
				check((z || (xTaskGetTickCount() * portTICK_RATE_MS) - (xLastWakeTime1 * portTICK_RATE_MS) >= 1000), "req5");
			} 
		else 
			{
			check((z || (xTaskGetTickCount() * portTICK_RATE_MS) - (xLastWakeTime1 * portTICK_RATE_MS) >= 1000), "req5");
			}
		z = 0;
		}
	else if (MOTOR_STOPPED && AT_FLOOR && y == 0) 
		{
			y = 1;
			xLastWakeTime1 = xTaskGetTickCount();
		}
	// safety requirement 6: elevator only stops when floors are called
	if (MOTOR_STOPPED)
			{
		n = 1;
		if (c1) {
			af = f1;
		} else if (c2) {
			af = f2;
		} else if (c3) {
			af = f3;
		}
		if (AT_FLOOR && n == 0)
		{
		check(af == -1 || (getCarPosition() >= af - 0.5 && getCarPosition() <= af + 0.5), "req6");
		}
	} else {
		n = 0;
	}
		
		
	vTaskDelayUntil(&xLastWakeTime, POLL_TIME);
  }

}

void setupSafety(unsigned portBASE_TYPE uxPriority) {
  xTaskCreate(safetyTask, "safety", 100, NULL, uxPriority, NULL);
}
