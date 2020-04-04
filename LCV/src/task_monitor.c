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
 * \file task_monitor.c
 *
 * \brief Monitoring task
 *
 */

#include "task_monitor.h"

// Task handle
static TaskHandle_t monitor_task_handle = NULL;

static void monitor_task(void * pvParameters)
{
	UNUSED(pvParameters);
	
	for (;;)
	{
		vTaskDelay(pdMS_TO_TICKS(1000)); // TODO do something here
	}
}

/*
*	\brief Creates the monitor task
*
*	\param stack_depth_words The depth of the stack in words
*	\param task_priority The task priority
*/
void create_monitor_task(uint16_t stack_depth_words, unsigned portBASE_TYPE task_priority)
{
	xTaskCreate(monitor_task, (const char * const) "MONITOR",
		stack_depth_words, NULL, task_priority, &monitor_task_handle);
}