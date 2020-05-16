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
 
 static uint32_t calculate_new_setpoint(uint32_t stage_start_time_ms, uint32_t current_time_ms, lcv_state_t * state, lcv_control_t * control)
 {
	int32_t time_into_profile = current_time_ms - stage_start_time_ms;
	uint32_t new_state_start = stage_start_time_ms;
	// In PEEP to PIP stage?
	if(time_into_profile < control->peep_to_pip_rampup_ms)
	{
		// Linear ramp up
		control->pressure_set_point_cm_h20 = state->setting_state.peep_cm_h20;
		float section_factor = ((float) (time_into_profile) / (float) control->peep_to_pip_rampup_ms);
		control->pressure_set_point_cm_h20 += (int32_t)  (section_factor * (state->setting_state.pip_cm_h20 - state->setting_state.peep_cm_h20));
	}
	else if(time_into_profile < (control->peep_to_pip_rampup_ms + control->pip_hold_ms))
	{
		control->pressure_set_point_cm_h20 = state->setting_state.pip_cm_h20;
	}
	else if(time_into_profile < (control->peep_to_pip_rampup_ms + control->pip_hold_ms + control->pip_to_peep_rampdown_ms))
	{
		// Linear ramp down
		control->pressure_set_point_cm_h20 = state->setting_state.pip_cm_h20;
		
		float section_dt = time_into_profile - (control->peep_to_pip_rampup_ms + control->pip_hold_ms);
		control->pressure_set_point_cm_h20 += (section_dt / (float) control->pip_to_peep_rampdown_ms) * (state->setting_state.peep_cm_h20 - state->setting_state.pip_cm_h20);
	}
	else if(time_into_profile < (control->peep_to_pip_rampup_ms + control->pip_hold_ms + control->pip_to_peep_rampdown_ms + control->peep_hold_ms))
	{
		control->pressure_set_point_cm_h20 = state->setting_state.peep_cm_h20;
	}
	else
	{
		// Time over this setpoint, return new transition time, keep at PEEP
		control->pressure_set_point_cm_h20 = state->setting_state.peep_cm_h20;
		new_state_start = stage_start_time_ms + (control->peep_to_pip_rampup_ms + control->pip_hold_ms + control->pip_to_peep_rampdown_ms + control->peep_hold_ms);
	}
	return new_state_start;
 }

 /*
 *	\brief Performs PIDF control
 *
 *	Has integral error range and anti-windup
 *	Has derivative filtering
 *	Note: this is a tracking controller, so "derivative" can somewhat abruptly change, need to be careful
 *
 *	\param control Pointer to the control structure defining the pressure profile
 *	\param params Pointer to the structure holding controller tuning parameters
 */
 static float pidf_control(lcv_control_t * control, controller_param_t * params)
 {
	static float error_integral = 0.0;
	static float error_derivative = 0.0;
	static float last_error = 0.0;

	float error = control->pressure_set_point_cm_h20 - control->pressure_current_cm_h20;

	float alpha = 0.7;
	error_derivative = alpha*(error-last_error) + (1.0 - alpha)*error_derivative;

	if(abs(error) < params->integral_enable_error_range)
	{
		error_integral += error;
		if(abs(error_integral * params->ki) > params->integral_antiwindup)
		{
			error_integral	= (error_integral/abs(error_integral)) * (params->integral_antiwindup) / params->ki;
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
	static bool was_enabled = false;
	static uint32_t start_of_current_profile_time_ms = 0;
	uint32_t current_time_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;

	if(!was_enabled && state->current_state.enable)
	{
		start_of_current_profile_time_ms = current_time_ms;
	}

	// First, determine what the new setpoint should be
	// Updates profile if enters a new profile
	start_of_current_profile_time_ms = calculate_new_setpoint(start_of_current_profile_time_ms, current_time_ms, state, control);

	// Then, run the controller to track this setpoint
	float output = pidf_control(control, params);
	was_enabled = (state->current_state.enable > 0);
	return output;
 }