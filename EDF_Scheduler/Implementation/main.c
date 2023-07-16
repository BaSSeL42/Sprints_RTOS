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

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "lpc21xx.h"
#include "semphr.h"
/* Peripheral includes. */
#include "serial.h"
#include "GPIO.h"

#include "Botton.h"

#define BOTTON_1									1
#define BOTTON_2									2
#define BOTTON_1_PERIOD									50
#define BOTTON_2_PERIOD									50
#define Periodic_Transmitter_PERIOD						100
#define Uart_Receiver_PERIOD						20
#define Load_1_Simulation_PERIOD						10
#define Load_2_Simulation_PERIOD						100


#define FAILING_EDGE			2
#define RISING_EDGE				1

#define PRESSED						1
#define NOT_PRESSED				0
#define SEND_STRING				1


unsigned char		botton_1_state_for_consumer_task		= NOT_PRESSED;
unsigned char		botton_2_state_for_consumer_task		= NOT_PRESSED;


static signed char botton_1_rising_string[] = "Botton 1 RISING EDGE\n";
static signed char botton_1_failing_string[] = "Botton 1 FAILING EDGE\n";
static unsigned char botton_1_rising_string_size = 9 + 13;
static unsigned char botton_1_failing_string_size = 9 + 14;
static signed char botton_2_rising_string[] = "Botton 2 RISING EDGE\n";
static signed char botton_2_failing_string[] = "Botton 2 FAILING EDGE\n";
static unsigned char botton_2_rising_string_size = 9 + 13;
static unsigned char botton_2_failing_string_size = 9 + 14;

static signed char string_1[]="Thank\n";
static unsigned char string_1_size = 7;
unsigned char		botton_1_state		= PRESSED;
unsigned char		botton_1_prv_state		= PRESSED;
unsigned char		botton_2_state		= PRESSED;
unsigned char		botton_2_prv_state		= PRESSED;
static unsigned char   consumer_task_data [4];


/*-----------------------------------------------------------*/

/* Constants to setup I/O and processor. */
#define mainBUS_CLK_FULL	( ( unsigned char ) 0x01 )

/* Constants for the ComTest demo application tasks. */
#define mainCOM_TEST_BAUD_RATE	( ( unsigned long ) 115200 )




/*******************************************************************************************************/

/*
 * Configure the processor for use with the Keil demo board.  This is very
 * minimal as most of the setup is managed by the settings in the project
 * file.
 */


static void prvSetupHardware( void );
/*-----------------------------------------------------------*/
 pinState_t buttonInput;


/*
void vApplicationTickHook( void )
{
	GPIO_write(PORT_0, PIN1, PIN_IS_HIGH);
	GPIO_write(PORT_0, PIN1, PIN_IS_LOW);
}
*/

/*
void vApplicationIdleHook( void )
{
	GPIO_write(PORT_0, PIN2, PIN_IS_HIGH);
	
}
*/
TaskHandle_t Button_1_Monitor_Handler = NULL;
TaskHandle_t Button_2_Monitor_Handler = NULL;
TaskHandle_t Periodic_Transmitter_Handler = NULL;
TaskHandle_t Uart_Receiver_Handler = NULL;
TaskHandle_t Load_1_Simulation_Handler = NULL;
TaskHandle_t Load_2_Simulation_Handler = NULL;
SemaphoreHandle_t Semaphore_Handler;


void Button_1_Monitor( void * pvParameters )
{
			TickType_t xLastWakeTime;
			xLastWakeTime = xTaskGetTickCount();
	
		vTaskSetApplicationTaskTag(NULL, (void *) PIN4);
		
    for( ;; )
    {
			botton_1_state = botton_read(BOTTON_1);					// read botton status
			if(botton_1_state != botton_1_prv_state)
				{
					//botton_1_state_for_consumer_task = CHANGE_EDGE;
					if(botton_1_state == PRESSED)
					{
						consumer_task_data[0] = RISING_EDGE;
					}
					else{
						consumer_task_data[0] = FAILING_EDGE;
					}
						
					//consumer_task_data
				}
				else
				{
					// do nothing
				}
			botton_1_prv_state = botton_1_state;
					
			vTaskDelayUntil(&xLastWakeTime, BOTTON_1_PERIOD);
				
				GPIO_write(PORT_0, PIN8, PIN_IS_LOW);
				
    }
}


void Button_2_Monitor( void * pvParameters )
{
			TickType_t xLastWakeTime;
			xLastWakeTime = xTaskGetTickCount();
	
		vTaskSetApplicationTaskTag(NULL, (void *) PIN5);
	
    for( ;; )
    {
			botton_2_state = botton_read(BOTTON_2);					// read botton status
			if(botton_2_state != botton_2_prv_state)
				{
					if(botton_2_state == PRESSED)
					{
						consumer_task_data[1] = RISING_EDGE;
					}
					else{
						consumer_task_data[1] = FAILING_EDGE;
					}
				}
				else
				{
					// do nothing
				}
			botton_2_prv_state = botton_2_state;
			vTaskDelayUntil(&xLastWakeTime, BOTTON_2_PERIOD);
				
				GPIO_write(PORT_0, PIN8, PIN_IS_LOW);
    }
}




void Periodic_Transmitter( void * pvParameters )
{
			TickType_t xLastWakeTime;
			xLastWakeTime = xTaskGetTickCount();
	
		vTaskSetApplicationTaskTag(NULL, (void *) PIN3);

    //static unsigned char semaphore_state = pdFALSE ;
		for(;;)
		{
			consumer_task_data[2] = SEND_STRING;
			/*semaphore_state = xSemaphoreTake( Semaphore_Handler, ( TickType_t ) 0 );
			if(semaphore_state == pdTRUE)
			{
				vSerialPutString(string_1, string_1_size);
				xSemaphoreGive( Semaphore_Handler);
			}*/
		
			vTaskDelayUntil(&xLastWakeTime, Periodic_Transmitter_PERIOD);
			
			GPIO_write(PORT_0, PIN8, PIN_IS_LOW);
		
    }
}



void Uart_Receiver( void * pvParameters )
{
	
		static unsigned char semaphore_state = pdFALSE ;
	
			TickType_t xLastWakeTime;
			xLastWakeTime = xTaskGetTickCount();
	
	vTaskSetApplicationTaskTag(NULL, (void *) PIN2);
	
    for( ;; )
    {
			if(consumer_task_data[0] == RISING_EDGE)
			{
				semaphore_state = xSemaphoreTake( Semaphore_Handler, ( TickType_t ) 0 );
				if(semaphore_state == pdTRUE)
				{
					vSerialPutString(botton_1_rising_string, botton_1_rising_string_size);
					xSemaphoreGive( Semaphore_Handler);
				}
				consumer_task_data[0] = 0;
			}
			else if(consumer_task_data[0] == FAILING_EDGE)
			{
				semaphore_state = xSemaphoreTake( Semaphore_Handler, ( TickType_t ) 0 );
				if(semaphore_state == pdTRUE)
				{
					vSerialPutString(botton_1_failing_string, botton_1_failing_string_size);
					xSemaphoreGive( Semaphore_Handler);
				}
				consumer_task_data[0] = 0;
			}
			
			if(consumer_task_data[1] == RISING_EDGE)
			{
				semaphore_state = xSemaphoreTake( Semaphore_Handler, ( TickType_t ) 0 );
				if(semaphore_state == pdTRUE)
				{
					vSerialPutString(botton_2_rising_string, botton_2_rising_string_size);
					xSemaphoreGive( Semaphore_Handler);
				}
				consumer_task_data[1] = 0;
			}
			else if(consumer_task_data[1] == FAILING_EDGE)
			{
				semaphore_state = xSemaphoreTake( Semaphore_Handler, ( TickType_t ) 0 );
				if(semaphore_state == pdTRUE)
				{
					vSerialPutString(botton_2_failing_string, botton_2_failing_string_size);
					xSemaphoreGive( Semaphore_Handler);
				}
				consumer_task_data[1] = 0;
			}
			
			if(consumer_task_data[2] == SEND_STRING)
			{
				semaphore_state = xSemaphoreTake( Semaphore_Handler, ( TickType_t ) 0 );
				if(semaphore_state == pdTRUE)
				{
					vSerialPutString(string_1, string_1_size);
					xSemaphoreGive( Semaphore_Handler);
				}
				consumer_task_data[2] = 0;
			}

			vTaskDelayUntil(&xLastWakeTime, Uart_Receiver_PERIOD);
			
			GPIO_write(PORT_0, PIN8, PIN_IS_LOW);

    }
}

// 9990
void Load_1_Simulation( void * pvParameters )
{
			TickType_t xLastWakeTime;
			xLastWakeTime = xTaskGetTickCount();
	
		vTaskSetApplicationTaskTag(NULL, (void *) PIN6);
    for( ;; )
    {
			 uint32_t i = 0;
			for(i = 0 ; i < 37000 ; i++)
			{
				// heavy load for 5 ms
			}

			vTaskDelayUntil(&xLastWakeTime, Load_1_Simulation_PERIOD);
			
			GPIO_write(PORT_0, PIN8, PIN_IS_LOW);

    }
}


// 23000
void Load_2_Simulation( void * pvParameters )
{
			TickType_t xLastWakeTime;
			xLastWakeTime = xTaskGetTickCount();
	
	 vTaskSetApplicationTaskTag(NULL, (void *) PIN7);
    for( ;; )
    {
			uint32_t i = 0;
			for(i = 0 ; i < 87000 ; i++)
			{
				// heavy load for 12 ms
			}
			vTaskDelayUntil(&xLastWakeTime, Load_2_Simulation_PERIOD);
			
			//toggling idlehook
			GPIO_write(PORT_0, PIN8, PIN_IS_LOW);
    }
		
}

/* System tick */
void vApplicationTickHook(void)
 {  
		GPIO_write(PORT_0, PIN9, PIN_IS_HIGH);
	 
		GPIO_write(PORT_0, PIN9, PIN_IS_LOW);
 }
 
 void vApplicationIdleHook(void)
{
GPIO_write(PORT_0, PIN8, PIN_IS_HIGH);
}

/*
 * Application entry point:
 * Starts all the other tasks, then starts the scheduler. 
 */
int main( void )
{
	/* Setup the hardware for use with the Keil demo board. */
	prvSetupHardware();

	Semaphore_Handler = xSemaphoreCreateMutex();
	xSemaphoreGive( Semaphore_Handler );
    /* Create Tasks here */
	/* todo: Create the task, storing the handle. */
		  xTaskPeriodicCreate(
                    Button_1_Monitor,       			/* Function that implements the task. */
                    "Task_Name",       		/* Text name for the task. */
                    configMINIMAL_STACK_SIZE,      				  		/* Stack size in words, not bytes. */
                    ( void * ) 0,       		/* Parameter passed into the task. */
                    1,                  		/* Priority at which the task is created. */
                    &Button_1_Monitor_Handler, 					/* Used to pass out the created task's handle. */
										50 );   								// Setting Deadline
			
			xTaskPeriodicCreate(
                    Button_2_Monitor,       			/* Function that implements the task. */
                    "Task_Name",       		/* Text name for the task. */
                    configMINIMAL_STACK_SIZE,      				  		/* Stack size in words, not bytes. */
                    ( void * ) 0,       		/* Parameter passed into the task. */
                    2,                  		/* Priority at which the task is created. */
                    &Button_2_Monitor_Handler, 					/* Used to pass out the created task's handle. */
										50 );   								// Setting Deadline

			xTaskPeriodicCreate(
                    Periodic_Transmitter,       			/* Function that implements the task. */
                    "Task_Name",       		/* Text name for the task. */
                    configMINIMAL_STACK_SIZE,      				  		/* Stack size in words, not bytes. */
                    ( void * ) 0,       		/* Parameter passed into the task. */
                    2,                  		/* Priority at which the task is created. */
                    &Periodic_Transmitter_Handler, 					/* Used to pass out the created task's handle. */
										100 );   								// Setting Deadline		

			xTaskPeriodicCreate(
                    Uart_Receiver,       			/* Function that implements the task. */
                    "Task_Name",       		/* Text name for the task. */
                    configMINIMAL_STACK_SIZE,      				  		/* Stack size in words, not bytes. */
                    ( void * ) 0,       		/* Parameter passed into the task. */
                    2,                  		/* Priority at which the task is created. */
                    &Uart_Receiver_Handler, 					/* Used to pass out the created task's handle. */
										20 );   								// Setting Deadline		


			xTaskPeriodicCreate(
                    Load_1_Simulation,       			/* Function that implements the task. */
                    "Task_Name",       		/* Text name for the task. */
                    configMINIMAL_STACK_SIZE,      				  		/* Stack size in words, not bytes. */
                    ( void * ) 0,       		/* Parameter passed into the task. */
                    2,                  		/* Priority at which the task is created. */
                    &Load_1_Simulation_Handler, 					/* Used to pass out the created task's handle. */
										10 );   								// Setting Deadline

			xTaskPeriodicCreate(
                    Load_2_Simulation,       			/* Function that implements the task. */
                    "Task_Name",       		/* Text name for the task. */
                    configMINIMAL_STACK_SIZE,      				  		/* Stack size in words, not bytes. */
                    ( void * ) 0,       		/* Parameter passed into the task. */
                    2,                  		/* Priority at which the task is created. */
                    &Load_2_Simulation_Handler, 					/* Used to pass out the created task's handle. */
										100 );   								// Setting Deadline


										
									
										
										
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
