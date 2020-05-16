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
 * \file lcd_interface.c
 *
 * \brief Driver for interfacing with the NHD-0420D3Z-FL-GBW-V3 LCD 
 *
 */

  #include <string.h>

 #include "../task_monitor.h"
 #include "../task_hmi.h"
 #include "../task_control.h"

 #include "alarm_monitoring.h"

 #include "lcd_interface.h"

 #define SCREEN_BUFFER_SIZE			(80)

 static struct i2c_master_packet screen_buffer_first_half_packet;
 static struct i2c_master_packet screen_buffer_second_half_packet;
 static struct i2c_master_packet cursor_set_packet;
 static struct i2c_master_packet power_on_packet;
 static struct i2c_master_packet contrast_packet;
 static struct i2c_master_packet backlight_packet;

 static char alarm_screen_buffer[SCREEN_BUFFER_SIZE] = {0};
 static char main_screen_buffer[SCREEN_BUFFER_SIZE] = {0};

 static char * intro_screen = "Low Cost Ventilator";

 bool lcd_init(void)
 {
	// Turn on screen
	static uint8_t on_screen_buffer[2] = {LCD_PREFIX, LCD_COMMAND_DISPLAY_ON};
	power_on_packet.address = LCD_I2C_ADDRESS;
	power_on_packet.data = on_screen_buffer;
	power_on_packet.data_length = 2;
	power_on_packet.high_speed = false;
	power_on_packet.ten_bit_address = false;
	i2c_transaction_t transaction;
	transaction.packet = power_on_packet;
	add_lcd_i2c_transaction_to_queue(transaction);
	
	set_backlight(8);

	set_contrast(40);

	// Set up initial screen
	memcpy(main_screen_buffer, intro_screen, strlen(intro_screen));
	send_buffer(MAIN_SCREEN);
 }

bool set_string(uint8_t row, uint8_t column, char * c, uint8_t length, SCREEN_TYPE screen)
{
	bool success = true;
	if(row == 0 || row > 4 || column == 0 || column > 20)
	{
		success = false;
		return success;
	}

	for(int i = 0; i < length; i++)
	{
		set_character(row, column, (c+i), screen);
		column++;
		if(column > 20)
		{
			column = 1;
			row+=1;
		}
	}
	return success;
}

bool set_character(uint8_t row, uint8_t column, char * c, SCREEN_TYPE screen)
{
	 /*
                    Column 1    Column 20
        Line 1      0x00        0x13
        Line 2      0x40        0x53
        Line 3      0x14        0x27
        Line 4      0x54        0x67
    */
    bool success = true;
    if(row == 0 || row > 4 || column == 0 || column > 20)
    {
       success = false;
       return success;
    }
    // Calculate index
    uint8_t index = (column-1)*20 + (row-1);
    return set_character_index(index, c, screen);
}

bool set_character_index(uint8_t panel_index, char * c, SCREEN_TYPE screen)
{
	if(panel_index > 80 || panel_index < 1)
	{
		return false;
	}
	if(screen = MAIN_SCREEN)
	{
		memcpy(&main_screen_buffer[panel_index-1], c, 1);
		return true;
	}
	else if(screen == ALARM_SCREEN)
	{
		memcpy(&alarm_screen_buffer[panel_index-1], c, 1);
		return true;
	}
	return false;
}

bool send_buffer(SCREEN_TYPE screen)
{
	/*
                    Column 1    Column 20
        Line 1      0x00        0x13
        Line 2      0x40        0x53
        Line 3      0x14        0x27
        Line 4      0x54        0x67
    */
	// reorganize to this format
	static uint8_t lines_1_3_buffer[40];
	static uint8_t lines_2_4_buffer[40];

	if(screen == MAIN_SCREEN)
	{
		memcpy(&lines_1_3_buffer[0], &main_screen_buffer[0], 20);
		memcpy(&lines_1_3_buffer[20], &main_screen_buffer[40], 20);
		memcpy(&lines_2_4_buffer[0], &main_screen_buffer[20], 20);
		memcpy(&lines_2_4_buffer[20], &main_screen_buffer[60], 20);
	}
	else if(screen == ALARM_SCREEN)
	{
		memcpy(&lines_1_3_buffer[0], &alarm_screen_buffer[0], 20);
		memcpy(&lines_1_3_buffer[20], &alarm_screen_buffer[40], 20);
		memcpy(&lines_2_4_buffer[0], &alarm_screen_buffer[20], 20);
		memcpy(&lines_2_4_buffer[20], &alarm_screen_buffer[60], 20);
	}
	else
	{
		return false;
	}

	// Clear any trailing 0s from string creation as those are special characters on the LCD
	for(int32_t i = 0; i < 40; i++)
	{
		if(lines_1_3_buffer[i] < 0x07)
		{
			lines_1_3_buffer[i] = 0x20; // ASCII space
		}
		if(lines_2_4_buffer[i] < 0x07)
		{
			lines_2_4_buffer[i] = 0x20; // ASCII space
		}
	}

	i2c_transaction_t transaction;

	// First set cursor to start
	static uint8_t cursor_set[3] = {LCD_PREFIX, LCD_COMMAND_SET_CURSOR, 0x00};
	cursor_set_packet.address = LCD_I2C_ADDRESS;
	cursor_set_packet.data = cursor_set;
	cursor_set_packet.data_length = 3;
	cursor_set_packet.high_speed = false;
	cursor_set_packet.ten_bit_address = false;
	transaction.packet = cursor_set_packet;
	add_lcd_i2c_transaction_to_queue(transaction);

	// Send first and third lines
	screen_buffer_first_half_packet.address = LCD_I2C_ADDRESS;
	screen_buffer_first_half_packet.data = lines_1_3_buffer;
	screen_buffer_first_half_packet.data_length = 40;
	screen_buffer_first_half_packet.high_speed = false;
	screen_buffer_first_half_packet.ten_bit_address = false;
	transaction.packet = screen_buffer_first_half_packet;
	add_lcd_i2c_transaction_to_queue(transaction);

	// Send second and fourth lines
	screen_buffer_second_half_packet.address = LCD_I2C_ADDRESS;
	screen_buffer_second_half_packet.data = lines_2_4_buffer;
	screen_buffer_second_half_packet.data_length = 40;
	screen_buffer_second_half_packet.high_speed = false;
	screen_buffer_second_half_packet.ten_bit_address = false;
	transaction.packet = screen_buffer_second_half_packet;
	add_lcd_i2c_transaction_to_queue(transaction);
}

bool set_contrast(uint8_t level)
{
	if(level < 1 || level > 50)
	{
		return false;
	}
	static uint8_t data_to_send[3];
	data_to_send[0] = LCD_PREFIX;
	data_to_send[1] = LCD_COMMAND_SET_CONTRAST;
	data_to_send[2] = level;
	contrast_packet.address = LCD_I2C_ADDRESS;
	contrast_packet.data = data_to_send;
	contrast_packet.data_length = 3;
	contrast_packet.high_speed = false;
	contrast_packet.ten_bit_address = false;
	i2c_transaction_t transaction;
	transaction.packet = contrast_packet;
	add_lcd_i2c_transaction_to_queue(transaction);
	return true;
}

bool set_backlight(uint8_t level)
{
	if(level < 1 || level > 8)
	{
		return false;
	}
	static uint8_t data_to_send[3];
	data_to_send[0] = LCD_PREFIX;
	data_to_send[1] = LCD_COMMAND_SET_BRIGHTNESS;
	data_to_send[2] = level;
	backlight_packet.address = LCD_I2C_ADDRESS;
	backlight_packet.data = data_to_send;
	backlight_packet.data_length = 3;
	backlight_packet.high_speed = false;
	backlight_packet.ten_bit_address = false;
	i2c_transaction_t transaction;
	transaction.packet = backlight_packet;
	add_lcd_i2c_transaction_to_queue(transaction);
	return true;
}

void update_main_buffer(lcv_parameters_t * new_settings,  SETTINGS_INPUT_STAGE stage)
{
	lcv_parameters_t current_settings = get_current_settings();

	for(int32_t i = 0; i < SCREEN_BUFFER_SIZE; i++)
	{
		main_screen_buffer[i] = 0x20; // ASCII space
	}

	// Update info
	// NOTE: snprintf here doesn't support floats
	if(current_settings.enable)
	{
		snprintf(&main_screen_buffer[0],9,"VENT:ON");
	}
	else
	{
		snprintf(&main_screen_buffer[0],9,"VENT:OFF");
	}

	snprintf(&main_screen_buffer[10],10, "V:%iml", current_settings.tidal_volume_ml);

	snprintf(&main_screen_buffer[20],13, "PEEP:%icmH20", current_settings.peep_cm_h20);

	uint8_t current_inspiratory_ones = current_settings.ie_ratio_tenths / 10;
	uint8_t current_inspiratory_tenths = current_settings.ie_ratio_tenths - (10*current_inspiratory_ones);
	snprintf(&main_screen_buffer[32],9, "IE:%i.%i:1", current_inspiratory_ones, current_inspiratory_tenths);

	snprintf(&main_screen_buffer[40],13, "PIP:%icmH20", current_settings.pip_cm_h20);

	snprintf(&main_screen_buffer[52],7, "BPM:%i", current_settings.breath_per_min);

	// Fill in settings input display
	uint8_t setting_inspiratory_ones = new_settings->ie_ratio_tenths / 10;
	uint8_t setting_inspiratory_tenths = new_settings->ie_ratio_tenths - (10*setting_inspiratory_ones);

	switch (stage)
	{
		case STAGE_NONE:
			break;

		case STAGE_BPM:
			sprintf(&main_screen_buffer[60], "SET BPM:%i", new_settings->breath_per_min);
			break;

		case STAGE_PEEP:
			sprintf(&main_screen_buffer[60], "SET PEEP:%icmH20", new_settings->peep_cm_h20);
			break;

		case STAGE_PIP:
			sprintf(&main_screen_buffer[60], "SET PIP:%icmH20", new_settings->pip_cm_h20);
			break;

		case STAGE_IE:
			sprintf(&main_screen_buffer[60], "SET I:E: %i.%i:1", setting_inspiratory_ones, setting_inspiratory_tenths);
			break;
		
		default:
			break;
	}
}

void update_alarm_buffer(void)
{
	for(int32_t i = 0; i < SCREEN_BUFFER_SIZE; i++)
	{
		alarm_screen_buffer[i] = 0x20; // ASCII space
	}

	snprintf(&alarm_screen_buffer[0],9,"ERRORS:");

	if(check_alarm(ALARM_FLOW_SENSOR))
	{
		snprintf(&alarm_screen_buffer[10],10,"FLOW");
	}

	if(check_alarm(ALARM_PRESSURE_SENSOR))
	{
		snprintf(&alarm_screen_buffer[20],10,"PRES SNS");
	}

	if(check_alarm(ALARM_MOTOR_ERROR))
	{
		snprintf(&alarm_screen_buffer[30],10,"MOT FAIL");
	}

	if(check_alarm(ALARM_MOTOR_TEMP))
	{
		snprintf(&alarm_screen_buffer[40],10,"MOT TEMP");
	}

	if(check_alarm(ALARM_SETTINGS_LOAD))
	{
		snprintf(&alarm_screen_buffer[50],10,"SETT LOAD");
	}

	if(check_alarm(ALARM_P_RAMP_SETTINGS_INVALID))
	{
		snprintf(&alarm_screen_buffer[60],10,"P RISE");
	}
}