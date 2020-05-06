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
 * \file flow_sensor_sfm3300.h
 *
 * \brief Driver for interfacing with flow sensor
 *
 */


#ifndef FLOW_SENSOR_SFM3300_H_
#define FLOW_SENSOR_SFM3300_H_

#define SFM3300_I2C_ADDRESS             (0x40)

#define SFM3300_MEAS_CMD_BYTE_1         (0x10)
#define SFM3300_MEAS_CMD_BYTE_2         (0x00)
#define SFM3300_READ_SN_BYTE_1          (0x31)
#define SFM3300_READ_SN_BYTE_2          (0xAE)
#define SFM3300_SOFT_RESET_BYTE_1       (0x20)
#define SFM3300_SOFT_RESET_BYTE_2       (0x00)

#define SFM3300_OFFSET_FLOW             (32768)
#define SFM3300_SCALE_FACTOR_FLOW       (120)       // in 1/slm

void flow_sensor_init(struct i2c_master_module * i2c_mod);
void flow_sensor_request_flow_slm(struct i2c_master_module * i2c_mod);

#endif /* FLOW_SENSOR_SFM3300_H_ */