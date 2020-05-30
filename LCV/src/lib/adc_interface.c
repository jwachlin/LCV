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

 #include "alarm_monitoring.h"

 #include "adc_interface.h"

 #define ADC_BUFFER_SIZE		(9)
 #define ADC_MAX				(4095.0)

 static struct adc_module adc_module_instance;
 static volatile uint16_t adc_buffer[ADC_BUFFER_SIZE];

 static volatile float pressure_raw_filt[3];
 static volatile uint16_t potentiometer_meas_raw;
 static volatile uint16_t motor_temp_meas_raw;
 static volatile uint16_t flow_meas_raw;

 static volatile bool setup = false;

 static void adc_cb(struct adc_module *const module)
 {
	if(adc_get_job_status(module, ADC_JOB_READ_BUFFER) == STATUS_OK)
	{
		// Motor first
		motor_temp_meas_raw = adc_buffer[0];
		// Control potentiometer
		potentiometer_meas_raw = adc_buffer[1];
		// Three pressure sensors in a raw
		pressure_raw_filt[0] = (0.9 * pressure_raw_filt[0]) + (0.1) * adc_buffer[2];
		pressure_raw_filt[1] = (0.9 * pressure_raw_filt[1]) + (0.1) * adc_buffer[3];
		pressure_raw_filt[2] = (0.9 * pressure_raw_filt[2]) + (0.1) * adc_buffer[4];
		// Flow sensor at ain[10]
		flow_meas_raw = adc_buffer[8];
	}
 }

 /*
 *	\brief Sets up ADC interface
 */
 void adc_interface_init(void)
 {
	struct adc_config config;

	adc_get_config_defaults(&config);
	config.positive_input = ADC_POSITIVE_INPUT_PIN2;
	config.negative_input = ADC_NEGATIVE_INPUT_GND;
	config.differential_mode = false;
	config.clock_source = GCLK_GENERATOR_1; // 8Mhz clock TODO is this fast enough?
	config.clock_prescaler = ADC_CLOCK_PRESCALER_DIV256;
	config.gain_factor = ADC_GAIN_FACTOR_1X;
	config.resolution = ADC_RESOLUTION_12BIT;
	config.reference = ADC_REFERENCE_AREFA; // 3.3V

	// Scan from 2 through 10
	config.pin_scan.offset_start_scan = 0;
	config.pin_scan.inputs_to_scan = 9;

	adc_init(&adc_module_instance, ADC, &config);
	adc_enable(&adc_module_instance);

	// Handle all conversions in callbacks
	adc_register_callback(&adc_module_instance, adc_cb, ADC_CALLBACK_READ_BUFFER);
	adc_enable_callback(&adc_module_instance, ADC_CALLBACK_READ_BUFFER);

	setup = true;

	// Start the conversion
	adc_request_update();
 }

 void adc_request_update(void)
 {
	// Trigger new measurement
	if(setup)
	{
		adc_read_buffer_job(&adc_module_instance, adc_buffer, ADC_BUFFER_SIZE);
	}
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
	// Pressure sensors output 0.5-4.5V corresponding to 0-5psig
	// Scaled down to 3.3V range with 10K/(10K+5.6K) divider
	if(channel >= NUM_PRESSURE_SENSOR_CHANNELS)
	{
		return 0.0;
	}
	float raw_adc =  pressure_raw_filt[channel];

	float pressure_voltage_scaled_up = ((raw_adc / ADC_MAX) * 3.3) * (15.6 / 10.0);

	float pressure_psi = 5.0 * (pressure_voltage_scaled_up - 0.5) / 4.0;

	float pressure_cmH2O = (70.307) * pressure_psi;
	return pressure_cmH2O;
 }

 /*
 *	\brief Gets pressure sensor data best estimate and checks for errors
 *
 *	\return The voted pressure in cm-H2O 
 */
 float get_pressure_sensor_cmH2O_voted(void)
 {
	// TODO for now, just use single pressure sensor
	set_alarm(ALARM_PRESSURE_SENSOR, false);

	int32_t i;
	// Get pressure
	float pressure_values[3];
	for(i=0; i<NUM_PRESSURE_SENSOR_CHANNELS; i++)
	{
		pressure_values[i] = get_pressure_sensor_cmH2O(i);
	}
	// Average, eliminate the furthest outlier, and average again to get actual, then check for sensor failure
	float avg_pressure = 0.0;
	for(i=0; i<NUM_PRESSURE_SENSOR_CHANNELS; i++)
	{
		avg_pressure += 0.33333 *pressure_values[i];
	}
	// Eliminate the furthest outlier of average
	uint8_t biggest_error_index = 0;
	float biggest_error = 0.0;
	for(i=0; i<NUM_PRESSURE_SENSOR_CHANNELS; i++)
	{
		float this_error = abs(avg_pressure - pressure_values[i]);
		if(this_error > biggest_error)
		{
			biggest_error = this_error;
			biggest_error_index = i;
		}
	}
	// Average again with closest two values
	avg_pressure = 0.0;
	for(i=0; i<NUM_PRESSURE_SENSOR_CHANNELS; i++)
	{
		if(i != biggest_error_index)
		{
			avg_pressure += 0.5 * pressure_values[i];
		}
	}
	// Check for sensor failure
	// TODO what is threshold?
	if(abs(pressure_values[biggest_error_index] - avg_pressure) > 0.2*avg_pressure)
	{
		set_alarm(ALARM_PRESSURE_SENSOR, true);
	}
	else
	{
		set_alarm(ALARM_PRESSURE_SENSOR, false);
	}
	return avg_pressure;
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
	return 0.0;
 }

 /*
 *	\brief Gets flow meter flow in standard liters per minute
 *
 *	\return The flow rate in slm
 */
 float get_flow_slm(void)
 {
	// Sensor outputs 0.5-4.5V which is -250 to +250 SLM
	// Scaled by 10K/(5.6K+10K) divider
	float flow_voltage_scaled_up = ((flow_meas_raw / ADC_MAX) * 3.3) * (1.56);

	float flow_slm = 250.0 * (flow_voltage_scaled_up - 2.5) / 2.0;
	return flow_slm;
 }