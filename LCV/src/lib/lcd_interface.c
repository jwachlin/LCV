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

 #include "../task_monitor.h"

 #include "lcd_interface.h"

 #define LCD_SERCOM					SERCOM1
 #define SCREEN_BUFFER_SIZE			(80)

 static struct i2c_master_module i2c_master_instance;
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
	struct i2c_master_config config_i2c_master;
	i2c_master_get_config_defaults(&config_i2c_master);
	config_i2c_master.baud_rate = 45; // Set in # of kHz
	config_i2c_master.buffer_timeout = 65535;
	config_i2c_master.pinmux_pad0 = PIN_PA16C_SERCOM1_PAD0;
	config_i2c_master.pinmux_pad1 = PIN_PA17C_SERCOM1_PAD1;
	
	/* Initialize and enable device with config */
	while(i2c_master_init(&i2c_master_instance, LCD_SERCOM, &config_i2c_master) != STATUS_OK);
	i2c_master_enable(&i2c_master_instance);

	// Turn on screen
	static uint8_t on_screen_buffer[2] = {LCD_PREFIX, LCD_COMMAND_DISPLAY_ON};
	power_on_packet.address = LCD_I2C_ADDRESS;
	power_on_packet.data = on_screen_buffer;
	power_on_packet.data_length = 2;
	power_on_packet.high_speed = false;
	power_on_packet.ten_bit_address = false;
	i2c_master_write_packet_job(&i2c_master_instance, &power_on_packet);
	
	set_backlight(2);

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

	// First set cursor to start
	static uint8_t cursor_set[3] = {LCD_PREFIX, LCD_COMMAND_SET_CURSOR, 0x00};
	cursor_set_packet.address = LCD_I2C_ADDRESS;
	cursor_set_packet.data = cursor_set;
	cursor_set_packet.data_length = 3;
	cursor_set_packet.high_speed = false;
	cursor_set_packet.ten_bit_address = false;
	i2c_master_write_packet_job(&i2c_master_instance, &cursor_set_packet);

	// Send first and third lines
	screen_buffer_first_half_packet.address = LCD_I2C_ADDRESS;
	screen_buffer_first_half_packet.data = lines_1_3_buffer;
	screen_buffer_first_half_packet.data_length = 40;
	screen_buffer_first_half_packet.high_speed = false;
	screen_buffer_first_half_packet.ten_bit_address = false;
	i2c_master_write_packet_job(&i2c_master_instance, &screen_buffer_first_half_packet);

	// Send second and fourth lines
	screen_buffer_second_half_packet.address = LCD_I2C_ADDRESS;
	screen_buffer_second_half_packet.data = lines_2_4_buffer;
	screen_buffer_second_half_packet.data_length = 40;
	screen_buffer_second_half_packet.high_speed = false;
	screen_buffer_second_half_packet.ten_bit_address = false;
	i2c_master_write_packet_job(&i2c_master_instance, &screen_buffer_second_half_packet);
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
	i2c_master_write_packet_job(&i2c_master_instance, &contrast_packet);
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
	i2c_master_write_packet_job(&i2c_master_instance, &backlight_packet);
	return true;
}
