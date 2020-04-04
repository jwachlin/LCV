/*MIT License

Copyright (c) 2020 jwachlin

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/

/**
 * \file main.c
 *
 * \brief Firmware for low-cost ventilator
 *
 */


#include <asf.h>
#include "task_monitor.h"

int main (void)
{
	system_init();
	delay_init();
	
	// Set up application tasks.
	create_monitor_task(taskMONITOR_TASK_STACK_SIZE, taskMONITOR_TASK_PRIORITY);
	create_control_task(taskCONTROL_TASK_STACK_SIZE, taskCONTROL_TASK_PRIORITY);
	create_sensor_task(taskSENSOR_TASK_STACK_SIZE, taskSENSOR_TASK_PRIORITY);
	create_hmi_task(taskHMI_TASK_STACK_SIZE, taskHMI_TASK_PRIORITY);

	vTaskStartScheduler();
	
	// Should never get here, FreeRTOS tasks should have begun
	for (;;)
	{
	}
	
	return 0;
}

/******* FreeRTOS User-Defined Hooks *******/

void vApplicationMallocFailedHook(void);

void vApplicationMallocFailedHook(void)
{
	/* Only called if configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h */
	taskDISABLE_INTERRUPTS();
	for (;;)
	{
	}
}

void vApplicationTickHook(void);

void vApplicationTickHook(void)
{
	/* This function will be called by each tick interrupt if
	configUSE_TICK_HOOK is set to 1 in FreeRTOSConfig.h */
}

void vApplicationStackOverflowHook(void);

void vApplicationStackOverflowHook(void)
{
	/* Only called if configCHECK_FOR_STACK_OVERFLOW is not set to 0 in FreeRTOSConfig.h */
	taskDISABLE_INTERRUPTS();
	for (;;)
	{
	}
}