/*
 * FreeRTOS Kernel V10.2.0
 * Copyright (C) 2019 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

/* 
	NOTE : Tasks run in system mode and the scheduler runs in Supervisor mode.
	The processor MUST be in supervisor mode when vTaskStartScheduler is 
	called.  The demo applications included in the FreeRTOS.org download switch
	to supervisor mode prior to main being called.  If you are not using one of
	these demo application projects then ensure Supervisor mode is used.
*/


/*
 * Creates all the demo application tasks, then starts the scheduler.  The WEB
 * documentation provides more details of the demo application tasks.
 * 
 * Main.c also creates a task called "Check".  This only executes every three 
 * seconds but has the highest priority so is guaranteed to get processor time.  
 * Its main function is to check that all the other tasks are still operational.
 * Each task (other than the "flash" tasks) maintains a unique count that is 
 * incremented each time the task successfully completes its function.  Should 
 * any error occur within such a task the count is permanently halted.  The 
 * check task inspects the count of each task to ensure it has changed since
 * the last time the check task executed.  If all the count variables have 
 * changed all the tasks are still executing error free, and the check task
 * toggles the onboard LED.  Should any task contain an error at any time 
 * the LED toggle rate will change from 3 seconds to 500ms.
 *
 */

/* Standard includes. */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "lpc21xx.h"
#include "task.h"
#include "queue.h"
/* Peripheral includes. */
#include "serial.h"
#include "GPIO.h"


/*-----------------------------------------------------------*/

/* Constants to setup I/O and processor. */
#define mainBUS_CLK_FULL	( ( unsigned char ) 0x01 )

/* Constants for the ComTest demo application tasks. */
#define mainCOM_TEST_BAUD_RATE	( ( unsigned long ) 115200 )

#define LOW_STATE					0
#define HIGH_STATE				1

#define No_BUTTON_DETECTED		0
#define BUTTON_1_DETECTED			1
#define BUTTON_2_DETECTED			2

#define SIMPLE_LOAD					10000
#define BUTTON_TASK1_DELAY			100
#define BUTTON_TASK2_DELAY			100
#define CONSUMER_TASK_DELAY			100
#define SEND_STR_TASK_DELAY			1000


TaskHandle_t xSendStr1Handle = NULL;
TaskHandle_t xvConsumerHandle = NULL;
TaskHandle_t xButton1Handle = NULL;
TaskHandle_t xButton2Handle = NULL;
/* Queue used to send and receive complete struct AMessage structures. */
QueueHandle_t xStructQueue = NULL;


pinState_t gl_buttonState1 , gl_buttonState2;
uint32_t gl_u32_counting_every_one_sec = 0;
uint32_t gl_u32_task_periodicity = 0;
uint8_t gl_u8_is_button_detected = No_BUTTON_DETECTED;


struct AMessage
{
   char ucMessageID;
   char ucData[ 20 ];
} xMessage1, xMessage2;




/*
 * Configure the processor for use with the Keil demo board.  This is very
 * minimal as most of the setup is managed by the settings in the project
 * file.
 */
static void prvSetupHardware( void );
/*-----------------------------------------------------------*/
void vSendStr1Code( void * pvParameters );
void vConsumerTaskCode( void * pvParameters );
void vButtonTask1Code( void * pvParameters );
void vButtonTask2Code( void * pvParameters );



/*
 * Application entry point:
 * Starts all the other tasks, then starts the scheduler. 
 */
int main( void )
{
	/* Setup the hardware for use with the Keil demo board. */
	prvSetupHardware();

	   xStructQueue = xQueueCreate(
                         /* The number of items the queue can hold. */
                         10,
                         /* Size of each item is big enough to hold the
                         whole structure. */
                         sizeof( struct AMessage * ) );
	
			xTaskCreate(
                    vSendStr1Code,       							/* Function that implements the task. */
                    "send_string1",          						/* Text name for the task. */
                    configMINIMAL_STACK_SIZE,     /* Stack size in words, not bytes. */
                    ( void * ) NULL,    					/* Parameter passed into the task. */
                    1,							/* Priority at which the task is created. */
                    &xSendStr1Handle );
										
			xTaskCreate(
                    vConsumerTaskCode,       							/* Function that implements the task. */
                    "send_string1",          						/* Text name for the task. */
                    configMINIMAL_STACK_SIZE,     /* Stack size in words, not bytes. */
                    ( void * ) NULL,    					/* Parameter passed into the task. */
                    1,							/* Priority at which the task is created. */
                    &xvConsumerHandle );
										
			xTaskCreate(
                    vButtonTask1Code,       							/* Function that implements the task. */
                    "Button1",          						/* Text name for the task. */
                    configMINIMAL_STACK_SIZE,     /* Stack size in words, not bytes. */
                    ( void * ) NULL,    					/* Parameter passed into the task. */
                    2,							/* Priority at which the task is created. */
                    &xButton1Handle );
			xTaskCreate(
                    vButtonTask2Code,       							/* Function that implements the task. */
                    "Button2",          						/* Text name for the task. */
                    configMINIMAL_STACK_SIZE,     /* Stack size in words, not bytes. */
                    ( void * ) NULL,    					/* Parameter passed into the task. */
                    1,							/* Priority at which the task is created. */
                    &xButton2Handle );									
    /* Create Tasks here */


	/* Now all the tasks have been started - start the scheduler.

	NOTE : Tasks run in system mode and the scheduler runs in Supervisor mode.
	The processor MUST be in supervisor mode when vTaskStartScheduler is 
	called.  The demo applications included in the FreeRTOS.org download switch
	to supervisor mode prior to main being called.  If you are not using one of
	these demo application projects then ensure Supervisor mode is used here. */
	vTaskStartScheduler();

	/* Should never reach here!  If you do then there was not enough heap
	available for the idle task to be created. */
	for( ;; );
}
/*-----------------------------------------------------------*/

/* Function to reset timer 1 */
void timer1Reset(void)
{
	T1TCR |= 0x2;
	T1TCR &= ~0x2;
}

/* Function to initialize and start timer 1 */
static void configTimer1(void)
{
	T1PR = 1000;
	T1TCR |= 0x1;
}

static void prvSetupHardware( void )
{
	/* Perform the hardware setup required.  This is minimal as most of the
	setup is managed by the settings in the project file. */

	/* Configure UART */
	xSerialPortInitMinimal(mainCOM_TEST_BAUD_RATE);
	
	
	
	/* Configure GPIO */
	GPIO_init();
	
	/* Config trace timer 1 and read T1TC to get current tick */
	configTimer1();

	/* Setup the peripheral bus to be the same as the PLL output. */
	VPBDIV = mainBUS_CLK_FULL;
}
/*-----------------------------------------------------------*/



/* Task to be created. */
void vSendStr1Code( void * pvParameters )
{
	uint32_t loc_u32_counter;
	struct AMessage *pxRxedPointer;
    /* The parameter value is expected to be 1 as 1 is passed in the
    pvParameters value in the call to xTaskCreate() below. 
    configASSERT( ( ( uint32_t ) pvParameters ) == 1 );*/

    for( ;; )
    {
			if( xStructQueue != NULL )
			{
					/* Receive a message from the created queue to hold complex struct AMessage
					structure.  Block for 10 ticks if a message is not immediately available.
					The value is read into a struct AMessage variable, so after calling
					xQueueReceive() xRxedStructure will hold a copy of xMessage. */
					if( xQueueReceive( xStructQueue,
														&( pxRxedPointer ),
														( TickType_t ) 10 ) == pdPASS )
					{
						/* *pxRxedPointer now points to xMessage. */
						
							for (loc_u32_counter = 0; loc_u32_counter < SIMPLE_LOAD; loc_u32_counter++ )
							{
								/* do nothing */
							}
							vSerialPutString((signed char *)pxRxedPointer->ucData , 20);						
						
					}
			}

			vTaskDelay(SEND_STR_TASK_DELAY);
    }
}

void vConsumerTaskCode( void * pvParameters )
{
	uint32_t loc_u32_counter;
	struct AMessage *pxMessage;
    /* The parameter value is expected to be 1 as 1 is passed in the
    pvParameters value in the call to xTaskCreate() below. 
    configASSERT( ( ( uint32_t ) pvParameters ) == 1 );*/

    for( ;; )
    {
			if( gl_u8_is_button_detected == BUTTON_1_DETECTED )
			{
				gl_u8_is_button_detected = No_BUTTON_DETECTED;
				if(xStructQueue != NULL)
				{
					xMessage1.ucMessageID = 0xF0;
					strcpy( xMessage1.ucData, "Hello" );
					pxMessage = &xMessage1;
					
					xQueueSend( xStructQueue, ( void * ) &pxMessage, ( TickType_t ) 0 );
				}
			}
			else if(gl_u8_is_button_detected == BUTTON_2_DETECTED)
			{
				gl_u8_is_button_detected = No_BUTTON_DETECTED;
				if(xStructQueue != NULL)
				{
					xMessage2.ucMessageID = 0xF8;
					strcpy( xMessage2.ucData, "Hi" );
					pxMessage = &xMessage2;
					
					xQueueSend( xStructQueue, ( void * ) &pxMessage, ( TickType_t ) 0 );
				}				
			}
			else 
			{
				/* do nothing */
			}

			vTaskDelay(CONSUMER_TASK_DELAY);
    }
}



void vButtonTask1Code( void * pvParameters )
{
	 struct AMessage *pxMessage;
	static uint8_t prevButtonState = HIGH_STATE , currentButtonState;
    /* The parameter value is expected to be 1 as 1 is passed in the
    pvParameters value in the call to xTaskCreate() below. 
    configASSERT( ( ( uint32_t ) pvParameters ) == 1 );*/

    for( ;; )
    {
			gl_buttonState1 = GPIO_read(PORT_0, PIN0);
			if (((gl_buttonState1 == LOW_STATE) && (prevButtonState == HIGH_STATE) ) || ( (gl_buttonState1 == HIGH_STATE) && (prevButtonState == LOW_STATE) )  )
			{
				gl_u8_is_button_detected = BUTTON_1_DETECTED;
				prevButtonState = gl_buttonState1;
			}
				
			vTaskDelay(BUTTON_TASK1_DELAY);
    }
}

void vButtonTask2Code( void * pvParameters )
{
	 struct AMessage *pxMessage;
	static uint8_t prevButtonState = HIGH_STATE , currentButtonState;
    /* The parameter value is expected to be 1 as 1 is passed in the
    pvParameters value in the call to xTaskCreate() below. 
    configASSERT( ( ( uint32_t ) pvParameters ) == 1 );*/

    for( ;; )
    {
			gl_buttonState2 = GPIO_read(PORT_0, PIN7);
			if (((gl_buttonState2 == LOW_STATE) && (prevButtonState == HIGH_STATE) ) || ( (gl_buttonState2 == HIGH_STATE) && (prevButtonState == LOW_STATE) )  )
			{
				gl_u8_is_button_detected = BUTTON_2_DETECTED;
				prevButtonState = gl_buttonState2;
				//while(GPIO_read(PORT_0, PIN0) == 0);
				
			}
				
			vTaskDelay(BUTTON_TASK2_DELAY);
    }
}