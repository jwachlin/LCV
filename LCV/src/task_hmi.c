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
 * \file task_hmi.c
 *
 * \brief Human-machine interfacing task
 *
 */

#include "task_monitor.h"
#include "lib/lcd_interface.h"
#include "lib/alarm_monitoring.h"

#include "task_hmi.h"

// Task handle
static TaskHandle_t hmi_task_handle = NULL;

static TimerHandle_t screen_update_handle = NULL;
static TimerHandle_t screen_change_handle = NULL;

static bool display_main_page = true;

void vScreenChangeTimerCallback( TimerHandle_t xTimer )
{
	UNUSED(xTimer);
	display_main_page = !display_main_page;
}

void vScreenRefreshTimerCallback( TimerHandle_t xTimer )
{
	UNUSED(xTimer);
	
	// Don't display alarm page if no alarms
	if(!display_main_page)
	{
		if(!any_alarms_set())
		{
			display_main_page = true;
		}
	}

	if(display_main_page)
	{
		send_buffer(MAIN_SCREEN);
	}
	else
	{
		send_buffer(ALARM_SCREEN);
	}
}

static void hmi_task(void * pvParameters)
{
	lcd_init();

	screen_update_handle = xTimerCreate("SCREEN_TIM",
				pdMS_TO_TICKS(30),
				pdTRUE,
				(void *) 0,
				vScreenRefreshTimerCallback);
	if(screen_update_handle)
	{
		xTimerStart(screen_update_handle, 0);
	}

	screen_change_handle = xTimerCreate("SCREEN_CHG",
		pdMS_TO_TICKS(2000),
		pdTRUE,
		(void *) 0,
		vScreenChangeTimerCallback);
	if(screen_change_handle)
	{
		xTimerStart(screen_change_handle, 0);
	}

	UNUSED(pvParameters);
	
	for (;;)
	{
		vTaskDelay(pdMS_TO_TICKS(1000)); // TODO do something here
	}
}

/*
*	\brief Creates the human-machine-interface task
*
*	\param stack_depth_words The depth of the stack in words
*	\param task_priority The task priority
*/

void create_hmi_task(uint16_t stack_depth_words, unsigned portBASE_TYPE task_priority)
{
	xTaskCreate(hmi_task, (const char * const) "HMI",
		stack_depth_words, NULL, task_priority, &hmi_task_handle);
}