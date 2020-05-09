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
 * \file fm25l16b.c
 *
 * \brief Interface to FRAM
 *
 */

 #include "../task_monitor.h"

 #include "spi_interface.h"
 #include "checksum.h"

 #include "fm25l16b.h"

 #define PARAMETER_STORAGE_ADDRESS			(0)
 #define STATE_STORAGE_ADDRESS				(500) // MUST not overlap
 #define ADDRESS_MASK						(0x7FF) // 11 bit addressing

 #define PARAMETER_STORAGE_READ_SIZE				(3+18+1)	// 3 byte header, 18 bytes of data + 1 byte crc8
 #define PARAMETER_STORAGE_WRITE_SIZE				(4+18+1)	// 4 byte header, 18 bytes of data + 1 byte crc8

 static struct spi_slave_inst fram_slave;

 static void parameter_load_cb(uint8_t * buff, uint32_t length)
 {
	if(length == PARAMETER_STORAGE_READ_SIZE)
	{
		uint8_t crc_read = *(buff + PARAMETER_STORAGE_READ_SIZE-1);
		uint8_t crc_calc = crc_8((buff+3), PARAMETER_STORAGE_READ_SIZE-4); // Ignore header
		if(crc_calc == crc_read)
		{
			// Unpack
			lcv_parameters_t params;
			params.enable = *(buff+3);
			params.ie_ratio_tenths = *(buff+4);
			memcpy(&params.tidal_volume_ml, (buff+5), 4);
			memcpy(&params.peep_cm_h20, (buff+9), 4);
			memcpy(&params.pip_cm_h20, (buff+13), 4);
			memcpy(&params.breath_per_min, (buff+17), 4);

			update_settings(&params);
		}
	}
 }

 static void parameter_save_cb(uint8_t * buff, uint32_t length)
 {
	// Do nothing
 }

 void fram_init(void)
 {
	spi_interface_init();

	struct spi_slave_inst_config slave_dev_config;
	/* Configure and initialize software device instance of peripheral slave */
	spi_slave_inst_get_config_defaults(&slave_dev_config);
	slave_dev_config.ss_pin = FRAM_CS_GPIO;
	spi_attach_slave(&fram_slave, &slave_dev_config);
 }

 bool fram_load_parameters_asynch(void)
 {
	uint8_t tx_buff[PARAMETER_STORAGE_READ_SIZE];
	spi_transaction_t transaction;

	// Data
	uint16_t address = (PARAMETER_STORAGE_ADDRESS) & ADDRESS_MASK;
	tx_buff[0] = FRAM_READ;
	tx_buff[1] = (address & 0xFF00) >> 8;
	tx_buff[2] = (address & 0x00FF);

	transaction.tx_buff = tx_buff;
	transaction.buffer_length = PARAMETER_STORAGE_READ_SIZE;
	transaction.cb = parameter_load_cb;
	transaction.slave_device = fram_slave;

	return spi_transact(transaction);
 }

 bool fram_save_parameters_asynch(lcv_parameters_t * param)
 {
	uint8_t tx_buff[PARAMETER_STORAGE_WRITE_SIZE];
	spi_transaction_t transaction;

	// Data
	uint16_t address = (PARAMETER_STORAGE_ADDRESS) & ADDRESS_MASK;
	tx_buff[0] = FRAM_WREN; // TODO can you send WREN right before WRITE?
	tx_buff[1] = FRAM_WRITE;
	tx_buff[2] = (address & 0xFF00) >> 8;
	tx_buff[3] = (address & 0x00FF);

	tx_buff[4] = param->enable;
	tx_buff[5] = param->ie_ratio_tenths;
	memcpy(&tx_buff[6], &param->tidal_volume_ml, 4);
	memcpy(&tx_buff[10], &param->peep_cm_h20, 4);
	memcpy(&tx_buff[14], &param->pip_cm_h20, 4);
	memcpy(&tx_buff[18], &param->breath_per_min, 4);
	// Calculate CRC8
	tx_buff[22] = crc_8(&tx_buff[4], PARAMETER_STORAGE_WRITE_SIZE-5); // Ignore header

	transaction.tx_buff = tx_buff;
	transaction.buffer_length = PARAMETER_STORAGE_WRITE_SIZE;
	transaction.cb = parameter_save_cb;
	transaction.slave_device = fram_slave;

	return spi_transact(transaction);
 }

 bool fram_load_states_asynch(lcv_parameters_t * state)
 {

 }

 bool fram_save_states_asynch(lcv_parameters_t * state)
 {

 }