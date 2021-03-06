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
 * \file task_hmi.h
 *
 * \brief Human-machine interfacing task
 *
 */


#ifndef TASK_HMI_H_
#define TASK_HMI_H_

typedef enum
{
	STAGE_NONE=0,
	STAGE_BPM=1,
	STAGE_PEEP=2,
	STAGE_PIP=3,
	STAGE_IE=4
} SETTINGS_INPUT_STAGE;

typedef struct 
{
	struct i2c_master_packet packet;
} i2c_transaction_t;

bool system_is_enabled(void);
bool get_pushbutton_level(void);
void add_lcd_i2c_transaction_to_queue(i2c_transaction_t transaction);

#endif /* TASK_HMI_H_ */