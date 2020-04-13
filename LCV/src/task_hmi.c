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
#include "lib/adc_interface.h"
#include "task_control.h"

#include "task_hmi.h"

#define LCD_I2C_QUEUE_SIZE			(10)

#define LCD_SERCOM					SERCOM1
#define LCD_SERCOM_IRQn				SERCOM1_IRQn

// Task handle
static TaskHandle_t hmi_task_handle = NULL;
static TaskHandle_t lcd_i2c_task_handle = NULL;

static TimerHandle_t screen_update_handle = NULL;
static TimerHandle_t screen_change_handle = NULL;
static TimerHandle_t i2c_timeout_timer_handle = NULL;

static QueueHandle_t lcd_i2c_queue = NULL;
static struct i2c_master_module i2c_master_instance;

static bool display_main_page = true;

static SETTINGS_INPUT_STAGE stage = STAGE_NONE;
static lcv_parameters_t settings_input;
static const lcv_parameters_t lower_settings_range = {.enable = 0, .tidal_volume_ml = 100,
.peep_cm_h20 = 3, .pip_cm_h20 = 10, .breath_per_min = 6, .ie_ratio_tenths=5};

static const lcv_parameters_t upper_settings_range = {.enable = 0, .tidal_volume_ml = 2500,
.peep_cm_h20 = 20, .pip_cm_h20 = 35, .breath_per_min = 60, .ie_ratio_tenths=40};


static void handle_hmi_input(void)
{
	static bool last_button_status = false;
	// Check for stage change
	bool new_button_status = get_pushbutton_level();

	if(!last_button_status && new_button_status)
	{
		switch (stage)
		{
			case STAGE_NONE:
				stage = STAGE_BPM;
				break;

			case STAGE_BPM:
				stage = STAGE_PEEP;
				break;

			case STAGE_PEEP:
				stage = STAGE_PIP;
				break;

			case STAGE_PIP:
				stage = STAGE_IE;
				break;

			case STAGE_IE:
				// Save settings
				update_settings(&settings_input);
				stage = STAGE_NONE;
				break;
			
			default:
				stage = STAGE_NONE;
				break;
		}
	}

	// Handle the stage

	float knob_portion = get_input_potentiometer_portion();

	switch (stage)
	{
		case STAGE_NONE:
		break;

		case STAGE_BPM:
			settings_input.breath_per_min = (int32_t) lower_settings_range.breath_per_min +
			knob_portion * (upper_settings_range.breath_per_min - lower_settings_range.breath_per_min);
			break;

		case STAGE_PEEP:
			settings_input.peep_cm_h20 = (int32_t) lower_settings_range.peep_cm_h20 +
			knob_portion * (upper_settings_range.peep_cm_h20 - lower_settings_range.peep_cm_h20);
			break;

		case STAGE_PIP:
			settings_input.pip_cm_h20 = (int32_t) lower_settings_range.pip_cm_h20 +
			knob_portion * (upper_settings_range.pip_cm_h20 - lower_settings_range.pip_cm_h20);
			break;

		case STAGE_IE:
			settings_input.ie_ratio_tenths = (int32_t) lower_settings_range.ie_ratio_tenths +
			knob_portion * (upper_settings_range.ie_ratio_tenths - lower_settings_range.ie_ratio_tenths);
			break;
		
		default:
			stage = STAGE_NONE;
			break;
	}

	last_button_status = new_button_status;
}

static void vScreenChangeTimerCallback( TimerHandle_t xTimer )
{
	UNUSED(xTimer);
	display_main_page = !display_main_page;
}

static void vScreenRefreshTimerCallback( TimerHandle_t xTimer )
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

static void vI2CTimeoutTimerCallback( TimerHandle_t xTimer )
{
	UNUSED(xTimer);
	vTaskResume(lcd_i2c_task_handle);
}

void handle_i2c_write_complete(struct i2c_master_module *const module)
{
	 enum status_code status = i2c_master_get_job_status(module);

	xTaskResumeFromISR(lcd_i2c_task_handle);
}

static void lcd_i2c_hw_setup(void)
{
	struct i2c_master_config config_i2c_master;
	i2c_master_get_config_defaults(&config_i2c_master);
	config_i2c_master.generator_source = GCLK_GENERATOR_1;	// 8 MHz
	config_i2c_master.baud_rate = 45; // Set in # of kHz
	config_i2c_master.buffer_timeout = 65535;
	config_i2c_master.pinmux_pad0 = PIN_PA16C_SERCOM1_PAD0;
	config_i2c_master.pinmux_pad1 = PIN_PA17C_SERCOM1_PAD1;
	
	/* Initialize and enable device with config */
	while(i2c_master_init(&i2c_master_instance, LCD_SERCOM, &config_i2c_master) != STATUS_OK);

	// Uses FreeRTOS, so need to limit priority
	irq_register_handler(LCD_SERCOM_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
	i2c_master_register_callback(&i2c_master_instance, handle_i2c_write_complete, I2C_MASTER_CALLBACK_WRITE_COMPLETE);
	i2c_master_register_callback(&i2c_master_instance, handle_i2c_write_complete, I2C_MASTER_CALLBACK_ERROR);
	i2c_master_enable_callback(&i2c_master_instance, I2C_MASTER_CALLBACK_WRITE_COMPLETE);
	i2c_master_enable_callback(&i2c_master_instance, I2C_MASTER_CALLBACK_ERROR);

	i2c_master_enable(&i2c_master_instance);
}

static void hmi_task(void * pvParameters)
{
	UNUSED(pvParameters);

	lcd_i2c_hw_setup();
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

	const TickType_t xFrequency = pdMS_TO_TICKS(20);	// 50 Hz rate
	TickType_t xLastWakeTime = xTaskGetTickCount();
	
	for (;;)
	{
		// Ensure constant period, but don't use timer so that we have the defined priority of this task
		vTaskDelayUntil( &xLastWakeTime, xFrequency);

		handle_hmi_input();
		// Actual display write and screen changes happens in timers. Here we just update buffers
		update_main_buffer(&settings_input, stage);
		update_alarm_buffer();

	}
}

static void lcd_i2c_task(void * pvParameters)
{
	UNUSED(pvParameters);

	i2c_timeout_timer_handle = xTimerCreate("I2C_TIMEOUT",
		pdMS_TO_TICKS(30),
		pdFALSE,
		(void *) 0,
		vI2CTimeoutTimerCallback);

	i2c_transaction_t transaction;

	for (;;)
	{
		if(xQueueReceive(lcd_i2c_queue, &transaction,portMAX_DELAY) == pdTRUE)
		{
			// Send transaction
			i2c_master_write_packet_job(&i2c_master_instance, &transaction.packet);

			// Set up timeout timer
			xTimerReset(i2c_timeout_timer_handle, 0);

			vTaskSuspend(lcd_i2c_task_handle);
		}
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
	lcd_i2c_queue = xQueueCreate(LCD_I2C_QUEUE_SIZE, sizeof(i2c_transaction_t));

	xTaskCreate(hmi_task, (const char * const) "HMI",
		stack_depth_words, NULL, task_priority, &hmi_task_handle);

	xTaskCreate(lcd_i2c_task, (const char * const) "I2C",
		256, NULL, task_priority, &lcd_i2c_task_handle);
}

/*
*	\brief Checks is the system enable switch is on
*
*	\return True if enabled, false otherwise
*/
bool system_is_enabled(void)
{
	return (ioport_get_pin_level(INPUT_ENABLE_GPIO) == IOPORT_PIN_LEVEL_HIGH);
}

/*
*	\brief Checks the level of the pushbutton
*
*	\return True if high, false if low
*/
bool get_pushbutton_level(void)
{
	return ioport_get_pin_level(INPUT_PUSHBUTTON_GPIO);
}

void add_lcd_i2c_transaction_to_queue(i2c_transaction_t transaction)
{
	if(lcd_i2c_queue)
	{
		xQueueSend(lcd_i2c_queue, &transaction, 0);
	}
}