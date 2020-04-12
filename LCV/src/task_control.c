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
 * \file task_control.c
 *
 * \brief Ventilator control task
 *
 */

#include "task_monitor.h"

#include "lib/alarm_monitoring.h"
#include "lib/adc_interface.h"

#include "task_control.h"

// Task handle
static TaskHandle_t control_task_handle = NULL;

static lcv_state_t lcv_state;
static lcv_control_t lcv_control;

static void calculate_lcv_control_params(lcv_state_t * state, lcv_control_t * control)
{
	// Pressure control profile is piecewise linear
	/*
	*	PIP	         ________
	*			   /		  \
	*			 /			    \
	*	PEEP   /				  \____________
	*/
	float breath_cycle_total_time_ms = (60000.0) / state->setting_state.breath_per_min;
	float breath_cycle_total_time_ms_minus_ramps = breath_cycle_total_time_ms - (control->pip_to_peep_rampdown_ms + control->peep_to_pip_rampup_ms);

	if(breath_cycle_total_time_ms_minus_ramps < 0)
	{
		set_alarm(ALARM_P_RAMP_SETTINGS_INVALID, true);
		// TODO pick something reasonable?
	}
	else
	{
		set_alarm(ALARM_P_RAMP_SETTINGS_INVALID, false);
	}

	// I:E is a ratio, so 3:1 implies 4 parts needed. Split up what is left according to the I:E ratio
	float section_size_ms;
	if(state->setting_state.ie_ratio_tenths > 10)
	{
		// Typical ratios with more expiratory time
		section_size_ms = breath_cycle_total_time_ms_minus_ramps / ((state->setting_state.ie_ratio_tenths * 0.1) + 1.0);
		
		control->peep_hold_ms =	section_size_ms * (state->setting_state.ie_ratio_tenths * 0.1);
		control->pip_hold_ms = section_size_ms; // 1 section by definition
	}
	else
	{
		// Atypical inverse ratios with more inspiratory time
		float ratio_to_use = 1.0 / (state->setting_state.ie_ratio_tenths * 0.1);	// Ratio of 0.5:1 means 1:2, which has 3 sections
		section_size_ms = breath_cycle_total_time_ms_minus_ramps / (ratio_to_use + 1.0);

		control->pip_hold_ms =	section_size_ms * ratio_to_use;
		control->peep_hold_ms = section_size_ms; // 1 section by definition
	}
}

static void update_parameters_from_sensors(lcv_state_t * state, lcv_control_t * control)
{
	control->pressure_current_cm_h20 =  (int32_t) get_pressure_sensor_cmH2O_voted();


}

static void control_task(void * pvParameters)
{
	UNUSED(pvParameters);

	// Set default TODO what should these be?
	lcv_state.setting_state.enable = 0;
	lcv_state.setting_state.ie_ratio_tenths = 30;
	lcv_state.setting_state.peep_cm_h20 = 14;
	lcv_state.setting_state.pip_cm_h20 = 30;
	lcv_state.setting_state.breath_per_min = 20;

	// Assume nothing until feedback
	lcv_state.current_state = lcv_state.setting_state;

	// Set initial control settings
	lcv_control.peep_to_pip_rampup_ms = 1000;
	lcv_control.pip_to_peep_rampdown_ms = 1000;

	calculate_lcv_control_params(&lcv_state, &lcv_control);

	const TickType_t xFrequency = pdMS_TO_TICKS(10);	// 100 Hz rate
	TickType_t xLastWakeTime = xTaskGetTickCount();

	for (;;)
	{
		// Ensure constant period, but don't use timer so that we have the defined priority of this task
		vTaskDelayUntil( &xLastWakeTime, xFrequency);

		// Update sensor data if possible
		update_parameters_from_sensors(&lcv_state, &lcv_control);



	}
}

/*
*	\brief Creates the control task
*
*	\param stack_depth_words The depth of the stack in words
*	\param task_priority The task priority
*/

void create_control_task(uint16_t stack_depth_words, unsigned portBASE_TYPE task_priority)
{
	xTaskCreate(control_task, (const char * const) "CONTROL",
	stack_depth_words, NULL, task_priority, &control_task_handle);
}

/*
*	\brief Gets the current settings
*
*	\return The current settings
*/
lcv_parameters_t get_current_settings(void)
{
	return lcv_state.setting_state;
}