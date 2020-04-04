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
 * \file alarm_monitoring.c
 *
 * \brief Alarm status handling interface
 *
 */

#include "task_monitor.h"

#include "alarm_monitoring.h"

static volatile uint32_t alarm_bitfield = 0;

/*
*	\brief Sets an alarm status
*
*	\param alarm_type The alarm to set
*	\param set If true, alarm is set; if false, alarm is cleared
*/
void set_alarm(ALARM_TYPE_INDEX alarm_type, bool set)
{
	if((uint32_t) alarm_type > 31)
	{
		return;
	}

	if(set)
	{
		alarm_bitfield |= (1 << (uint32_t) alarm_type);
	}
	else
	{
		alarm_bitfield &= ~(1 << (uint32_t) alarm_type);
	}
	
}

/*
*	\brief Checks an alarm status
*
*	\param alarm_type The alarm to check
*
*	\return True if the alarm is set, false if not
*/
bool check_alarm(ALARM_TYPE_INDEX alarm_type)
{
	if((uint32_t) alarm_type > 31)
	{
		return false;
	}

	return (alarm_bitfield & (1 << (uint32_t) alarm_type));
}

/*
*	\brief Gets the entire alarm bitfield
*
*	\return The bitfield
*/
uint32_t get_alarm_bitfield(void)
{
	return alarm_bitfield;
}

/*
*	\brief Checks if any alarm is set
*
*	\return True if any alarm is set
*/
bool any_alarms_set(void)
{
	return (alarm_bitfield != 0);
}