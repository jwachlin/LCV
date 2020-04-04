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
 * \file adc_interface.c
 *
 * \brief Analog to digital converter shared interface
 *
 */

 #include "../task_monitor.h"

 #include "adc_interface.h"

 #define ADC_BUFFER_SIZE		10
 #define ADC_MAX				(65535.0)

 static struct adc_module adc_module_instance;
 static volatile uint16_t adc_buffer[ADC_BUFFER_SIZE];

 static uint16_t pressure_raw_int[3];
 static uint16_t potentiometer_meas_raw;
 static uint16_t motor_temp_meas_raw;

 static void adc_cb(struct adc_module *const module)
 {
	if(adc_get_job_status(module, ADC_JOB_READ_BUFFER) == STATUS_OK)
	{
		// Motor first
		memcpy(&motor_temp_meas_raw, &adc_buffer[0], 2);
		// Control potentiometer
		memcpy(&potentiometer_meas_raw, &adc_buffer[2], 2);
		// Three pressure sensors in a raw
		// TODO be sure indexing is correct
		memcpy(&pressure_raw_int[0], &adc_buffer[4], 2);
		memcpy(&pressure_raw_int[1], &adc_buffer[6], 2);
		memcpy(&pressure_raw_int[2], &adc_buffer[8], 2);
	}
	// Trigger new measurement
	adc_read_buffer_job(&adc_module_instance, adc_buffer, ADC_BUFFER_SIZE);
 }

 /*
 *	\brief Sets up ADC interface
 */
 void adc_interface_init(void)
 {
	struct adc_config config;

	adc_module_instance.reference = ADC_REFERENCE_AREFA; // 3.3V

	adc_get_config_defaults(&config);
	config.positive_input = ADC_POSITIVE_INPUT_PIN2;
	config.negative_input = ADC_NEGATIVE_INPUT_GND;
	config.differential_mode = false;
	config.clock_source = GCLK_GENERATOR_1; // 8Mhz clock
	config.clock_prescaler = ADC_CLOCK_PRESCALER_DIV4;
	config.gain_factor = ADC_GAIN_FACTOR_1X;
	config.resolution = ADC_RESOLUTION_16BIT;

	// Scan from 2 through 6
	config.pin_scan.offset_start_scan = 2;
	config.pin_scan.inputs_to_scan = 5;

	adc_init(&adc_module_instance, ADC, &config);
	adc_enable(&adc_module_instance);

	// Handle all conversions in callbacks
	adc_register_callback(&adc_module_instance, adc_cb, ADC_CALLBACK_READ_BUFFER);
	adc_enable_callback(&adc_module_instance, ADC_CALLBACK_READ_BUFFER);

	// Start the conversion
	adc_read_buffer_job(&adc_module_instance, adc_buffer, ADC_BUFFER_SIZE);
 }

 /*
 *	\brief Gets pressure sensor data
 *
 *	\param channel The sensor channel
 *	
 *	\return The pressure from the channel in cm-H2O if channel valid or 0 otherwise
 */
 float get_pressure_sensor_cmH2O(uint8_t channel)
 {
	if(channel >= NUM_PRESSURE_SENSOR_CHANNELS)
	{
		return 0.0;
	}
	uint16_t raw_adc =  pressure_raw_int[channel];

	float adc_portion_fsr = (raw_adc / ADC_MAX) * (PRESSURE_SENSOR_VOLTAGE / 3.3);
	float pressure_cmH2O = (70.307) * adc_portion_fsr * PRESSURE_SENSOR_FSR_PSI;
	return pressure_cmH2O;
 }

 /*
 *	\brief Gets portion of full scale from potentiometer input
 *
 *	\return The portion from 0.0 to 1.0
 */
 float get_input_potentiometer_portion(void)
 {
	return (potentiometer_meas_raw / ADC_MAX);
 }

 /*
 *	\brief Gets motor temperature
 *
 *	\return The temperature in Celsius
 */
 float get_motor_temp_celsius(void)
 {
	// TODO what is scale?
 }