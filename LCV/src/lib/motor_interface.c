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
 * \file motor_interface.c
 *
 * \brief Interfacing with blower motor driver
 *
 */

 #include "../task_monitor.h"

 #include "adc_interface.h"
 #include "alarm_monitoring.h"

 #include "motor_interface.h"

 static struct dac_module module;

 void init_motor_interface(void)
 {
	disable_motor();

	// Set up DAC
	struct dac_chan_config channel_config;
	dac_chan_get_config_defaults(&channel_config);

	struct dac_config config;
	dac_get_config_defaults(&config);
	config.left_adjust = false;
	config.voltage_pump_disable = false;
	config.reference = DAC_REFERENCE_AVCC;
	config.clock_source = GCLK_GENERATOR_1;	// 8 MHz

	dac_init(&module, DAC, &config);
	
	dac_chan_set_config(&module, DAC_CHANNEL_0, &channel_config);
	dac_chan_enable(&module, DAC_CHANNEL_0);

	dac_enable(&module);

	drive_motor(0.0);
 }

 void motor_status_monitor(void)
 {
	// TODO this does not work right now, even if motor working OK. Maybe too weak output?
	/*if(ioport_get_pin_level(MOTOR_READY_GPIO) == LOW)
	{
		set_alarm(ALARM_MOTOR_ERROR, true);
	}
	else
	{
		set_alarm(ALARM_MOTOR_ERROR, false);
	}*/
	set_alarm(ALARM_MOTOR_ERROR, false);

	if(get_motor_temp_celsius() > 100)
	{
		set_alarm(ALARM_MOTOR_TEMP, true);
	}
	else
	{
		set_alarm(ALARM_MOTOR_TEMP, false);
	}
 }

 void enable_motor(void)
 {
	ioport_set_pin_level(MOTOR_ENABLE_GPIO, MOTOR_ENABLE_ACTIVE_LEVEL);
 }

 void disable_motor(void)
 {
	ioport_set_pin_level(MOTOR_ENABLE_GPIO, !MOTOR_ENABLE_ACTIVE_LEVEL);
 }

 float drive_motor(float command)
 {
	static float last_command = 0.0;

	if(command< 0.0)
	{
		command = 0.00001;
	}
	if(command > 1.0)
	{
		command = 0.9999;
	}

	static float command_filt = 0.0;
	float alpha_down = 0.99;
	float alpha_up = 0.8;

	if(command >= last_command)
	{
		command_filt = alpha_up * command_filt + (1.0-alpha_up) * command;
	}
	else
	{
		command_filt = alpha_down * command_filt + (1.0-alpha_down) * command;
	}

	last_command = command_filt;

	uint16_t dac_out = (uint16_t) (command_filt * 1023.0);
	dac_out &= (0x3ff);
	dac_chan_write(&module, DAC_CHANNEL_0, dac_out);
	return command_filt;
 }