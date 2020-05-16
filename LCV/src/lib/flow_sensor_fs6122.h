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
 * \file flow_sensor_fs6122.h
 *
 * \brief Driver for interfacing with Siargo FS6122 Mass flow sensor
 *
 */


#ifndef FLOW_SENSOR_FS6122_H_
#define FLOW_SENSOR_FS6122_H_

#define FS6122_I2C_ADDRESS				(0x01)

typedef struct  
{
	int32_t flow_thousand_slpm;
	int32_t pressure_thousand_cmh20;
} siargo_fs6122_data_t;

void fs6122_init(void);
void reset_fs6122_read_pointer(void);
void request_fs6122_data(void);
void read_fs6122_data(siargo_fs6122_data_t * data);

#endif /* FLOW_SENSOR_FS6122_H_ */