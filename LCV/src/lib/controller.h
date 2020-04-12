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
 * \file controller.h
 *
 * \brief Controller routines
 *
 */


#ifndef CONTROLLER_H_
#define CONTROLLER_H_

#include "../task_control.h"

typedef struct
{
	float kf;
	float kp;
	float ki;
	float kd;
	float interal_antiwindup;
	float integral_enable_error_range;
	float max_output;
	float min_output;
} controller_param_t;

void calculate_lcv_control_params(lcv_state_t * state, lcv_control_t * control);
float run_controller(lcv_state_t * state, lcv_control_t * control, controller_param_t * params);

#endif /* CONTROLLER_H_ */