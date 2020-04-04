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
 * \file lcd_interface.h
 *
 * \brief Driver for interfacing with the NHD-0420D3Z-FL-GBW-V3 LCD 
 *
 */

#ifndef LCD_INTERFACE_H_
#define LCD_INTERFACE_H_

/*
*	\brief Enumeration of screen buffers
*/
typedef enum
{
	MAIN_SCREEN = 0,
	ALARM_SCREEN = 1
} SCREEN_TYPE;

#define LCD_I2C_ADDRESS                     (0x28) // not counting R/W bit

#define LCD_WAKEUP_TIME_MS                  (150)

/* LCD supports 0x20 to 0x7F ASCII characters */
#define LCD_PREFIX                          (0xFE)

#define LCD_COMMAND_DISPLAY_ON              (0x41)
#define LCD_COMMAND_DISPLAY_OFF             (0x42)
#define LCD_COMMAND_SET_CURSOR              (0x45)
#define LCD_COMMAND_CURSOR_HOME             (0x46)
#define LCD_COMMAND_UNDERLINE_CURSOR_ON     (0x47)
#define LCD_COMMAND_UNDERLINE_CURSOR_OFF    (0x48)
#define LCD_COMMAND_MOVE_CURSOR_LEFT        (0x49)
#define LCD_COMMAND_MOVE_CURSOR_RIGHT       (0x4A)
#define LCD_COMMAND_BLINK_CURSOR_ON         (0x4B)
#define LCD_COMMAND_BLINK_CURSOR_OFF        (0x4C)
#define LCD_COMMAND_BACKSPACE               (0x4E)
#define LCD_COMMAND_CLEAR_SPACE             (0x51)
#define LCD_COMMAND_SET_CONTRAST            (0x52)
#define LCD_COMMAND_SET_BRIGHTNESS          (0x53)
#define LCD_COMMAND_LOAD_CUSTOM_CHAR        (0x54)
#define LCD_COMMAND_MOVE_DISP_LEFT          (0x55)
#define LCD_COMMAND_MOVE_DISP_RIGHT         (0x56)
#define LCD_COMMAND_RS232_BAUDRATE          (0x61)
#define LCD_COMMAND_I2C_ADDRESS             (0x62)
#define LCD_COMMAND_DISP_FIRMWARE_VERSION   (0x70)
#define LCD_COMMAND_DISP_RS232_BAUDRATE     (0x71)
#define LCD_COMMAND_DISP_I2C_ADDRESS        (0x72)

bool lcd_init(void);
bool set_string(uint8_t row, uint8_t column, char * c, uint8_t length, SCREEN_TYPE screen);
bool set_character(uint8_t row, uint8_t column, char * c, SCREEN_TYPE screen);
bool set_character_index(uint8_t panel_index, char * c, SCREEN_TYPE screen);
bool send_buffer(SCREEN_TYPE screen);
bool set_contrast(uint8_t level);
bool set_backlight(uint8_t level);

#endif /* LCD_INTERFACE_H_ */