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
 * \file alarm_monitoring.h
 *
 * \brief Alarm status handling interface
 *
 */

#ifndef ALARM_MONITORING_H_
#define ALARM_MONITORING_H_

/*
*	\brief Enumeration of possible alarms
*/
typedef enum
{
	ALARM_FLOW_SENSOR = 0,
	ALARM_LCD = 1,
	ALARM_PRESSURE_SENSOR = 2,
	ALARM_MOTOR_ERROR = 3,
	ALARM_MOTOR_TEMP = 4,
	ALARM_SETTINGS_LOAD = 5
} ALARM_TYPE_INDEX;

void set_alarm(ALARM_TYPE_INDEX alarm_type, bool set);
bool check_alarm(ALARM_TYPE_INDEX alarm_type);
uint32_t get_alarm_bitfield(void);
bool any_alarms_set(void);

#endif /* ALARM_MONITORING_H_ */