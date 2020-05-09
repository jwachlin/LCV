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
 * \file spi_interface.c
 *
 * \brief Interface to non-blocking SPI
 *
 */

 #include "../task_monitor.h"
 #include <string.h>

 #include "spi_interface.h"
 
 #define MAX_BUFFER_SIZE		(64)

 #define SPI_SERCOM				SERCOM0

 static struct spi_module spi_master_instance;

 static volatile uint8_t tx_buffer[MAX_BUFFER_SIZE];
 static volatile uint8_t rx_buffer[MAX_BUFFER_SIZE];

 static volatile spi_transaction_t current_transaction;

 static void callback_spi_master( struct spi_module *const module)
 {
	 spi_select_slave(module, &current_transaction.slave_device, false);
	 // callback
	 current_transaction.cb(rx_buffer, current_transaction.buffer_length);
 }

 void spi_interface_init(void)
 {
	struct spi_config config_spi_master;
	/* Configure, initialize and enable SERCOM SPI module */
	spi_get_config_defaults(&config_spi_master);
	config_spi_master.mux_setting = SPI_SIGNAL_MUX_SETTING_C; // MOSI [0], SCK [1], MISO [2]
	config_spi_master.pinmux_pad0 = FRAM_MOSI_PINMUX;
	config_spi_master.pinmux_pad1 = FRAM_SCK_PINMUX;
	config_spi_master.pinmux_pad2 = FRAM_MISO_PINMUX;
	config_spi_master.pinmux_pad3 = PINMUX_DEFAULT;
	config_spi_master.generator_source = GCLK_GENERATOR_3; // 12MHz
	config_spi_master.mode_specific.master.baudrate = 100000; // 6MHz, FRAM can handle 20 MHz
	spi_init(&spi_master_instance, SPI_SERCOM, &config_spi_master);
	spi_enable(&spi_master_instance);

	// Set up callbacks
	spi_register_callback(&spi_master_instance, callback_spi_master,SPI_CALLBACK_BUFFER_TRANSCEIVED);
	spi_enable_callback(&spi_master_instance, SPI_CALLBACK_BUFFER_TRANSCEIVED);
 }

 bool spi_transact(spi_transaction_t transaction)
 {
	if(current_transaction.buffer_length > MAX_BUFFER_SIZE )
	{
		return false;
	}

	current_transaction = transaction;
	memcpy(tx_buffer, current_transaction.tx_buff, current_transaction.buffer_length);

	spi_select_slave(&spi_master_instance, &current_transaction.slave_device, true);
	spi_transceive_buffer_job(&spi_master_instance, tx_buffer, rx_buffer, current_transaction.buffer_length);
	return true;
 }