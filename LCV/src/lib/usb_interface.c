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
 * \file usb_interface.c
 *
 * \brief USB shared interface
 *
 */ 

 #include "../task_monitor.h"
 #include "../task_control.h"

 #include "usb_interface.h"

 static volatile bool authorize_cdc_transfer = false;

 void usb_interface_init(void)
 {
	udc_start();
 }

 void usb_transmit_control(lcv_control_t * control_params, float output)
 {
	int32_t i;
	if(authorize_cdc_transfer)
	{
		// send it
		uint8_t buffer[14];
		buffer[0] = USB_MAGIC_BYTE;
		memcpy(&buffer[1], &control_params->pressure_current_cm_h20, 4);
		memcpy(&buffer[5], &control_params->pressure_set_point_cm_h20, 4);
		memcpy(&buffer[9], &output, 4);
		buffer[13] = 0;
		for(i = 1; i < 13; i++)
		{
			buffer[13] += buffer[i];
		}

		
		if(udi_cdc_is_tx_ready() && udi_cdc_get_free_tx_buffer() >= 14)
		{
			udi_cdc_write_buf(buffer, 14);
		}
	}
 }

 bool my_callback_cdc_enable(void)
 {
	 authorize_cdc_transfer = true;
	 return true;
 }

 void my_callback_cdc_disable(void)
 {
	 authorize_cdc_transfer = false;
 }