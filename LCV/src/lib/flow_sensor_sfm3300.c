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
 * \file flow_sensor_sfm3300.c
 *
 * \brief Driver for interfacing with flow sensor
 *
 */

#include "../task_monitor.h"

#include "flow_sensor_sfm3300.h"

//CRC
#define POLYNOMIAL 0x131 //P(x)=x^8+x^5+x^4+1 = 100110001

static struct i2c_master_packet slm_read_packet;
static struct i2c_master_packet slm_write_packet;
static volatile uint8_t read_slm_buffer[3];

static bool flow_sensor_crc(uint8_t * data, uint8_t num_bytes, uint8_t checksum)
{
	 uint8_t crc = 0;
	 uint8_t byteCtr;
	 //calculates 8-Bit checksum with given polynomial
	 for (byteCtr = 0; byteCtr < num_bytes; ++byteCtr)
	 { crc ^= *(data+byteCtr);
		 for (uint8_t bit = 8; bit > 0; --bit)
		 { if (crc & 0x80) crc = (crc << 1) ^ POLYNOMIAL;
			 else crc = (crc << 1);
		 }
	 }
	 if (crc != checksum) return false;
	 else return true;
}

static void flow_sensor_slm_callback(struct i2c_master_module *const module)
{
	uint8_t read_crc = read_slm_buffer[2];
	if(!flow_sensor_crc(read_slm_buffer, 2, read_crc))
	{
		return;
	}

	uint32_t raw_rate = read_slm_buffer[1] | (read_slm_buffer[0] << 8);
	float flow_rate = (float) ((int32_t) raw_rate - (int32_t) SFM3300_OFFSET_FLOW) / SFM3300_SCALE_FACTOR_FLOW;
	// TODO do something with it
}

void flow_sensor_init(struct i2c_master_module * i2c_mod)
{
	// Set up I2C callback
	i2c_master_register_callback(i2c_mod, flow_sensor_slm_callback, I2C_MASTER_CALLBACK_READ_COMPLETE);
	i2c_master_enable_callback(i2c_mod, I2C_MASTER_CALLBACK_READ_COMPLETE);
	// TODO set priority to be FreeRTOS compatible?
}

void flow_sensor_power_on(void)
{
	ioport_set_pin_level(FLOW_SENSOR_POWER_GPIO, FLOW_SENSOR_POWER_ACTIVE_LEVEL);
}

void flow_sensor_power_off(void)
{
	ioport_set_pin_level(FLOW_SENSOR_POWER_GPIO, !FLOW_SENSOR_POWER_ACTIVE_LEVEL);
}

void flow_sensor_request_flow_slm(struct i2c_master_module * i2c_mod)
{
	// Note: Delay is inherent between these, so must not call faster than 500Hz

	slm_read_packet.address = SFM3300_I2C_ADDRESS;
	slm_read_packet.data = read_slm_buffer;
	slm_read_packet.data_length = 3;
	slm_read_packet.high_speed = false;
	slm_read_packet.ten_bit_address = false;
	i2c_master_read_packet_job(i2c_mod, &slm_read_packet);

	static uint8_t flow_request_to_send[2] = {SFM3300_MEAS_CMD_BYTE_1, SFM3300_MEAS_CMD_BYTE_2};
	// First have to request read, delay 2ms, and then read
	slm_write_packet.address = SFM3300_I2C_ADDRESS;
	slm_write_packet.data = &flow_request_to_send[0];
	slm_write_packet.data_length = 2;
	slm_write_packet.high_speed = false;
	slm_write_packet.ten_bit_address = false;
	i2c_master_write_packet_job(i2c_mod, &slm_write_packet);
}