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
 * \file fm25l16b.h
 *
 * \brief Interface to FRAM
 *
 */


#ifndef FM25L16B_H_
#define FM25L16B_H_

#include "../task_control.h"

#define FRAM_MEMORY_SIZE_BYTES					(2048)

#define FRAM_WREN								(0x06)
#define FRAM_WRDI								(0x04)
#define FRAM_RDSR								(0x05)
#define FRAM_WRSR								(0x01)
#define FRAM_READ								(0x03)
#define FRAM_WRITE								(0x02)

void fram_init(void);
bool fram_load_parameters_asynch(void);
bool fram_save_parameters_asynch(lcv_parameters_t * param);
bool fram_load_states_asynch(lcv_parameters_t * state);
bool fram_save_states_asynch(lcv_parameters_t * state);

#endif /* FM25L16B_H_ */