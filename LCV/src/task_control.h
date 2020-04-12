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
 * \file task_control.h
 *
 * \brief Ventilator control task
 *
 */


#ifndef TASK_CONTROL_H_
#define TASK_CONTROL_H_

typedef struct
{
	uint8_t enable : 1;
	uint8_t ie_ratio_tenths;
	int32_t tidal_volume_ml;
	int32_t peep_cm_h20;
	int32_t pip_cm_h20;
	int32_t breath_per_min;
} lcv_parameters_t;

typedef struct  
{
	lcv_parameters_t setting_state;
	lcv_parameters_t current_state;
} lcv_state_t;

typedef struct
{
	int32_t peep_to_pip_rampup_ms;
	int32_t pip_hold_ms;
	int32_t pip_to_peep_rampdown_ms;
	int32_t peep_hold_ms;
	int32_t pressure_set_point_cm_h20;
	int32_t pressure_current_cm_h20;
} lcv_control_t;

#endif /* TASK_CONTROL_H_ */