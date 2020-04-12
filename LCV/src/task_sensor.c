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

#include "lib/flow_sensor_sfm3300.h"
#include "lib/adc_interface.h"

#include "task_sensor.h"

#define FLOW_METER_SERCOM			SERCOM3

// Task handle
static TaskHandle_t sensor_task_handle = NULL;

static struct i2c_master_module i2c_master_instance;


/*
*	\brief Sets up sensor interface hardware
*
*	Sets up ADC for communication with pressure sensors and flow meter
*/
static void sensor_hw_init(void)
{
	struct i2c_master_config config_i2c_master;
	i2c_master_get_config_defaults(&config_i2c_master);
	config_i2c_master.baud_rate = I2C_MASTER_BAUD_RATE_100KHZ;
	config_i2c_master.buffer_timeout = 65535; 
	config_i2c_master.pinmux_pad0 = PIN_PA22C_SERCOM3_PAD0;
	config_i2c_master.pinmux_pad1 = PIN_PA23C_SERCOM3_PAD1;
	
	/* Initialize and enable device with config */
	while(i2c_master_init(&i2c_master_instance, FLOW_METER_SERCOM, &config_i2c_master) != STATUS_OK);
	i2c_master_enable(&i2c_master_instance);

	adc_interface_init();
}

/*
*	\brief The sensor task
*/
static void sensor_task(void * pvParameters)
{
	UNUSED(pvParameters);

	sensor_hw_init();
	
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