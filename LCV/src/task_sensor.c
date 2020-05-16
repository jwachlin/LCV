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
 * \file task_sensor.c
 *
 * \brief Sensor interfacing task
 *
 */

#include "task_monitor.h"

#include "lib/flow_sensor_fs6122.h"
#include "lib/adc_interface.h"

#include "task_sensor.h"

#define TIDAL_VOLUME_PERIOD_MS		(10)

// Task handle
static TaskHandle_t sensor_task_handle = NULL;

// Tidal volume estimator
static TimerHandle_t volume_estimator_handle = NULL;
static TimerHandle_t fs6122_read_handle = NULL;

static volatile float recent_tidal_volume_liter = 0.0;

/*
*	\brief Timer callback for sending I2C command to reset internal address pointer
*
*	\param xTimer The timer handle
*/
static void vFS6122ReadTimerCallback( TimerHandle_t xTimer )
{
	reset_fs6122_read_pointer();
}

/*
*	\brief Timer callback for tidal volume estimation
*
*	\param xTimer The timer handle
*/
static void vTidalVolumeTimerCallback( TimerHandle_t xTimer )
{
	UNUSED(xTimer);

	static float flow_volume = 0.0;
	static float tidal_volume = 0.0;
	static float filtered_rate = 0.0;
	static float last_filtered_rate = 0.0;
	static bool rising = true;
	static uint32_t last_time = 0;

	uint32_t current_time = xTaskGetTickCount();

	siargo_fs6122_data_t data;
	read_fs6122_data(&data);
	float flow_slm = data.flow_thousand_slpm * 0.001;

	float alpha = 0.7;
	filtered_rate = (alpha)*filtered_rate + (1.0-alpha)*flow_slm;
	float dt = 0.001 * (current_time - last_time); // Time in seconds
	flow_volume += flow_slm * (1.0/60.0) * dt;  // flow change in liters
	tidal_volume += abs(flow_slm) * (1.0/60.0) *dt * 0.5;	// total tidal flow change

	if(rising && filtered_rate > 0.0 && last_filtered_rate <= 0.0)
	{
		rising = false;
	}
	else if(!rising && filtered_rate < 0.0 && last_filtered_rate >= 0.0)
	{
		recent_tidal_volume_liter = tidal_volume;
		// Reset
		flow_volume = 0.0;
		tidal_volume = 0.0;
	}

	last_filtered_rate = filtered_rate;
	last_time = current_time;

	request_fs6122_data();
	xTimerReset(fs6122_read_handle, 0);
}

/*
*	\brief Sets up sensor interface hardware
*
*	Sets up ADC for communication with pressure sensors and flow meter
*/
static void sensor_hw_init(void)
{
	fs6122_init();

	adc_interface_init();
}

/*
*	\brief The sensor task
*/
static void sensor_task(void * pvParameters)
{
	UNUSED(pvParameters);

	sensor_hw_init();

	volume_estimator_handle = xTimerCreate("TIDALV",
		pdMS_TO_TICKS(TIDAL_VOLUME_PERIOD_MS),
		pdTRUE,
		(void *) 0,
		vTidalVolumeTimerCallback);

	if(volume_estimator_handle)
	{
		xTimerStart(volume_estimator_handle, 0);
	}

	fs6122_read_handle = xTimerCreate("FLOWS",
		pdMS_TO_TICKS(TIDAL_VOLUME_PERIOD_MS/2), // half time
		pdFALSE,
		(void *) 0,
		vFS6122ReadTimerCallback);
	
	for (;;)
	{
		vTaskDelay(pdMS_TO_TICKS(1000)); // TODO do something here
	}
}

/*
*	\brief Creates the sensor task
*
*	\param stack_depth_words The depth of the stack in words
*	\param task_priority The task priority
*/
void create_sensor_task(uint16_t stack_depth_words, unsigned portBASE_TYPE task_priority)
{
	xTaskCreate(sensor_task, (const char * const) "SENSOR",
		stack_depth_words, NULL, task_priority, &sensor_task_handle);
}

/*
*	\brief Gets the estimated tidal volume in liters
*/
float get_tidal_volume_liter(void)
{
	return recent_tidal_volume_liter;
}