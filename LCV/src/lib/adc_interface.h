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
 * \file adc_interface.h
 *
 * \brief Analog to digital converter shared interface
 *
 */

#ifndef ADC_INTERFACE_H_
#define ADC_INTERFACE_H_

#define NUM_PRESSURE_SENSOR_CHANNELS		3

void adc_interface_init(void);
void adc_request_update(void);
float get_pressure_sensor_cmH2O(uint8_t channel);
float get_pressure_sensor_cmH2O_voted(void);
float get_input_potentiometer_portion(void);
float get_motor_temp_celsius(void);
float get_flow_slm(void);

#endif /* ADC_INTERFACE_H_ */