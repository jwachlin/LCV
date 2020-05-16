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
 * \file flow_sensor_fs6122.c
 *
 * \brief Driver for interfacing with Siargo FS6122 Mass flow sensor
 *
 */

 #include "../task_monitor.h"

 #include "flow_sensor_fs6122.h"
 #include <string.h>

 #define FLOW_METER_SERCOM			SERCOM3

 static struct i2c_master_module i2c_master_instance;

 static volatile siargo_fs6122_data_t current_data;
 static volatile uint8_t read_slm_buffer[8];
 static struct i2c_master_packet slm_read_packet;
 static struct i2c_master_packet slm_write_packet;

 /*
 *	\brief Callback to handle the measurements from the flow sensor
 *
 *	\param module Pointer to I2C master module
 */
 static void flow_sensor_slm_callback(struct i2c_master_module *const module)
 {
	 // WARNING: ISR context
	 current_data.flow_thousand_slpm = (read_slm_buffer[0] * 16777216) + (read_slm_buffer[1] * 65536) + (read_slm_buffer[2] * 256) + read_slm_buffer[3];
	 current_data.pressure_thousand_cmh20 = (read_slm_buffer[4] * 16777216) + (read_slm_buffer[5] * 65536) + (read_slm_buffer[6] * 256) + read_slm_buffer[7];
 }

 void fs6122_init(void)
 {
	struct i2c_master_config config_i2c_master;
	i2c_master_get_config_defaults(&config_i2c_master);
	config_i2c_master.generator_source = GCLK_GENERATOR_1;	// 8 MHz
	config_i2c_master.baud_rate = 80;
	config_i2c_master.buffer_timeout = 65535;
	config_i2c_master.pinmux_pad0 = PIN_PA22C_SERCOM3_PAD0;
	config_i2c_master.pinmux_pad1 = PIN_PA23C_SERCOM3_PAD1;
	
	/* Initialize and enable device with config */
	while(i2c_master_init(&i2c_master_instance, FLOW_METER_SERCOM, &config_i2c_master) != STATUS_OK);
	i2c_master_enable(&i2c_master_instance);

	// Set up I2C callback
	i2c_master_register_callback(&i2c_master_instance, flow_sensor_slm_callback, I2C_MASTER_CALLBACK_READ_COMPLETE);
	i2c_master_enable_callback(&i2c_master_instance, I2C_MASTER_CALLBACK_READ_COMPLETE);

	// request read for next cycle
	static uint8_t flow_request_to_send = 0x84;
	// First have to request read, delay 2ms, and then read
	slm_write_packet.address = FS6122_I2C_ADDRESS;
	slm_write_packet.data = &flow_request_to_send;
	slm_write_packet.data_length = 1;
	slm_write_packet.high_speed = false;
	slm_write_packet.ten_bit_address = false;
	i2c_master_write_packet_job(&i2c_master_instance, &slm_write_packet);
 }

 void reset_fs6122_read_pointer(void)
 {
	// request read for next cycle
	static uint8_t flow_request_to_send = 0x84;
	// First have to request read, delay 2ms, and then read
	slm_write_packet.address = FS6122_I2C_ADDRESS;
	slm_write_packet.data = &flow_request_to_send;
	slm_write_packet.data_length = 1;
	slm_write_packet.high_speed = false;
	slm_write_packet.ten_bit_address = false;
	i2c_master_write_packet_job(&i2c_master_instance, &slm_write_packet);
 }
 
 void request_fs6122_data(void)
 {
	slm_read_packet.address = FS6122_I2C_ADDRESS;
	slm_read_packet.data = read_slm_buffer;
	slm_read_packet.data_length = 8;
	slm_read_packet.high_speed = false;
	slm_read_packet.ten_bit_address = false;
	i2c_master_read_packet_job(&i2c_master_instance, &slm_read_packet);
 }

 void read_fs6122_data(siargo_fs6122_data_t * data)
 {
	*data = current_data;
 }