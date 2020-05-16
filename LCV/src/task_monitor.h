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
 * \file task_monitor.h
 *
 * \brief Monitoring task
 *
 */

#ifndef TASK_MONITOR_H_
#define TASK_MONITOR_H_

#include "FreeRTOS.h"

#include "asf.h"

// Task priorities
#define taskMONITOR_TASK_PRIORITY		(tskIDLE_PRIORITY+3)
#define taskCONTROL_TASK_PRIORITY		(tskIDLE_PRIORITY+2)
#define taskSENSOR_TASK_PRIORITY		(tskIDLE_PRIORITY+2)
#define taskHMI_TASK_PRIORITY			(tskIDLE_PRIORITY+1)

// Task size allocation in words. Note 1 word = 4 bytes
#define taskMONITOR_TASK_STACK_SIZE		(256)
#define taskCONTROL_TASK_STACK_SIZE		(512)
#define taskSENSOR_TASK_STACK_SIZE		(256)
#define taskHMI_TASK_STACK_SIZE			(512)

void create_monitor_task(uint16_t stack_depth_words, unsigned portBASE_TYPE task_priority);
void create_control_task(uint16_t stack_depth_words, unsigned portBASE_TYPE task_priority);
void create_sensor_task(uint16_t stack_depth_words, unsigned portBASE_TYPE task_priority);
void create_hmi_task(uint16_t stack_depth_words, unsigned portBASE_TYPE task_priority);


#endif /* TASK_MONITOR_H_ */