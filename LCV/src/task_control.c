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
#include "task_hmi.h"
#include "task_sensor.h"

#include "lib/alarm_monitoring.h"
#include "lib/adc_interface.h"
#include "lib/controller.h"
#include "lib/motor_interface.h"
#include "lib/fm25l16b.h"
#include "lib/usb_interface.h"

#include "task_control.h"

// Task handle
static TaskHandle_t control_task_handle = NULL;

static TimerHandle_t adc_timer_handle = NULL;

static lcv_state_t lcv_state;
static lcv_control_t lcv_control;

static volatile bool settings_changed = true;

/*
*	\brief Timer callback for requesting ADC read
*
*	\param xTimer The timer handle
*/
static void vADCRequestTriggerCallback( TimerHandle_t xTimer )
{
	UNUSED(xTimer);
	adc_request_update();
}

static void update_parameters_from_sensors(lcv_state_t * state, lcv_control_t * control)
{
	state->current_state.enable = system_is_enabled();
	state->setting_state.enable = state->current_state.enable;

	state->current_state.tidal_volume_ml = (int32_t) 1000 * get_tidal_volume_liter();
	state->setting_state.tidal_volume_ml = state->current_state.tidal_volume_ml;

	control->pressure_current_cm_h20 =  (int32_t) get_pressure_sensor_cmH2O_voted();

	motor_status_monitor();
}

static void control_task(void * pvParameters)
{
	UNUSED(pvParameters);

	// Set up FRAM and SPI
	fram_init();

	// Set default TODO what should these be?
	lcv_state.setting_state.enable = 0;
	lcv_state.setting_state.ie_ratio_tenths = 30;
	lcv_state.setting_state.peep_cm_h20 = 14;
	lcv_state.setting_state.pip_cm_h20 = 30;
	lcv_state.setting_state.breath_per_min = 20;

	// Load from FRAM asynchronously
	fram_load_parameters_asynch();
	vTaskDelay(pdMS_TO_TICKS(5));

	// Assume nothing until feedback
	lcv_state.current_state = lcv_state.setting_state;

	// Set initial control settings
	lcv_control.peep_to_pip_rampup_ms = 200;
	lcv_control.pip_to_peep_rampdown_ms = 200;

	calculate_lcv_control_params(&lcv_state, &lcv_control);

	adc_timer_handle = xTimerCreate("ADCTH",
		pdMS_TO_TICKS(2),
		pdTRUE,
		(void *) 0,
		vADCRequestTriggerCallback);

	if(adc_timer_handle)
	{
		xTimerStart(adc_timer_handle, 0);
	}

	const TickType_t xFrequency = pdMS_TO_TICKS(10);	// 100 Hz rate
	TickType_t xLastWakeTime = xTaskGetTickCount();

	controller_param_t control_params;
	control_params.kf = 0.05; 
	control_params.kp = 0.01; 
	control_params.kd = 0.0;
	control_params.ki = 0.0;
	control_params.integral_enable_error_range = 35.0;
	control_params.integral_antiwindup = 0.3;
	control_params.max_output = 1.0;
	control_params.min_output = 0.0;

	init_motor_interface();

	for (;;)
	{
		// Ensure constant period, but don't use timer so that we have the defined priority of this task
		vTaskDelayUntil( &xLastWakeTime, xFrequency);

		// Ensure at least control is not locked by feeding here
		wdt_reset_count();

		// Feed hardware watchdog
		ioport_set_pin_level(WATCHDOG_GPIO, !ioport_get_pin_level(WATCHDOG_GPIO));

		// Update sensor data if possible
		update_parameters_from_sensors(&lcv_state, &lcv_control);

		// Save if changed 
		if(settings_changed)
		{
			fram_save_parameters_asynch(&lcv_state.setting_state);
			settings_changed = false;
		}

		float motor_output = run_controller(&lcv_state, &lcv_control, &control_params);
		if(lcv_state.current_state.enable)
		{
			enable_motor();
			float output_sent = drive_motor(motor_output);
			usb_transmit_control(&lcv_control, output_sent);
		}
		else
		{
			disable_motor();
			drive_motor(0.0);
		}

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

/*
*	\brief Updates the current settings
*
*	\param new_settings Pointer to the new settings
*/
void update_settings(lcv_parameters_t * new_settings)
{
	// NOTE: may be called from ISR
	lcv_state.setting_state = *new_settings;

	settings_changed = true;

	calculate_lcv_control_params(&lcv_state, &lcv_control);
}