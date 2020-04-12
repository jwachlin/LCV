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
 * \file controller.c
 *
 * \brief Controller routines
 *
 */

 #include "../task_monitor.h"
 #include "../task_control.h"

 #include "alarm_monitoring.h"

 #include "controller.h"

 static float pidf_control(lcv_control_t * control, controller_param_t * params)
 {
	static float error_integral = 0.0;
	static float error_derivative = 0.0;
	static float last_error;

	float error = control->pressure_set_point_cm_h20 - control->pressure_current_cm_h20;

	float alpha = 0.7;
	error_derivative = alpha*(error-last_error) + (1.0 - alpha)*error_derivative;

	if(abs(error) < params->integral_enable_error_range)
	{
		error_integral += error;
		if(abs(error_integral * params->ki) > params->interal_antiwindup)
		{
			error_integral	= (error_integral/abs(error_integral)) * (params->interal_antiwindup) / params->ki;
		}
	}
	else
	{
		error_integral = 0.0;
	}

	float output = params->kf * control->pressure_set_point_cm_h20 +
					params->kp * error +
					params->ki * error_integral +
					params->kd * error_derivative;

	if(output > params->max_output)
	{
		output = params->max_output;
	}

	if(output < params->min_output)
	{
		output = params->min_output;
	}

	last_error = error;
	return output;
 }

 /*
 *	\brief Calculates the control profile given the input settings
 *
 *	\param state Pointer to the state structure holding current and set parameters
 *	\param control Pointer to the control structure defining the pressure profile
 */
 void calculate_lcv_control_params(lcv_state_t * state, lcv_control_t * control)
 {
	// Pressure control profile is piecewise linear
	/*
	*	PIP	         ________
	*			   /		  \
	*			 /			    \
	*	PEEP   /				  \____________
	*/
	float breath_cycle_total_time_ms = (60000.0) / state->setting_state.breath_per_min;
	float breath_cycle_total_time_ms_minus_ramps = breath_cycle_total_time_ms - (control->pip_to_peep_rampdown_ms + control->peep_to_pip_rampup_ms);

	if(breath_cycle_total_time_ms_minus_ramps < 0)
	{
		set_alarm(ALARM_P_RAMP_SETTINGS_INVALID, true);
		// TODO pick something reasonable?
	}
	else
	{
		set_alarm(ALARM_P_RAMP_SETTINGS_INVALID, false);
	}

	// I:E is a ratio, so 3:1 implies 4 parts needed. Split up what is left according to the I:E ratio
	float section_size_ms;
	if(state->setting_state.ie_ratio_tenths > 10)
	{
		// Typical ratios with more expiratory time
		section_size_ms = breath_cycle_total_time_ms_minus_ramps / ((state->setting_state.ie_ratio_tenths * 0.1) + 1.0);
		
		control->peep_hold_ms =	section_size_ms * (state->setting_state.ie_ratio_tenths * 0.1);
		control->pip_hold_ms = section_size_ms; // 1 section by definition
	}
	else
	{
		// Atypical inverse ratios with more inspiratory time
		float ratio_to_use = 1.0 / (state->setting_state.ie_ratio_tenths * 0.1);	// Ratio of 0.5:1 means 1:2, which has 3 sections
		section_size_ms = breath_cycle_total_time_ms_minus_ramps / (ratio_to_use + 1.0);

		control->pip_hold_ms =	section_size_ms * ratio_to_use;
		control->peep_hold_ms = section_size_ms; // 1 section by definition
	}
 }

 /*
 *	\brief Runs the "guidance" and control
 *
 *	First determines the pressure setpoint based on position in the profile. Then runs the PIDF
 *	pressure controller to reach that setpoint
 *
 *	\param state Pointer to the state structure holding current and set parameters
 *	\param control Pointer to the control structure defining the pressure profile
 *	\param params Pointer to the structure holding controller tuning parameters
 */
 float run_controller(lcv_state_t * state, lcv_control_t * control, controller_param_t * params)
 {
	static uint32_t last_time_ms = 0;
	uint32_t current_time_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;

	// First, determine what the new setpoint should be

	// Then, run the controller to track this setpoint

	float output = pidf_control(control, params);
	last_time_ms = current_time_ms;
	return output;
 }