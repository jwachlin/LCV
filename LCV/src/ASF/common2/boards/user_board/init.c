/**
 * \file
 *
 * \brief User board initialization template
 *
 */
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */

#include <asf.h>
#include <board.h>
#include <conf_board.h>

#if defined(__GNUC__)
void board_init(void) WEAK __attribute__((alias("system_board_init")));
#elif defined(__ICCARM__)
void board_init(void);
#  pragma weak board_init=system_board_init
#endif

void system_board_init(void)
{
	ioport_init();
	
	// Set up motor
	ioport_set_pin_dir(MOTOR_ENABLE_GPIO, IOPORT_DIR_OUTPUT);
	ioport_set_pin_level(MOTOR_ENABLE_GPIO, !MOTOR_ENABLE_ACTIVE_LEVEL);
	ioport_set_pin_dir(MOTOR_READY_GPIO, IOPORT_DIR_INPUT);
	ioport_set_pin_mode(MOTOR_READY_GPIO, IOPORT_MODE_PULLDOWN);
	ioport_set_pin_mode(MOTOR_SPEED_GPIO, MOTOR_SPEED_GPIO_FLAGS);
	ioport_disable_pin(MOTOR_SPEED_GPIO);
	ioport_set_pin_mode(MOTOR_NTC_GPIO, MOTOR_NTC_GPIO_FLAGS);
	ioport_disable_pin(MOTOR_NTC_GPIO);
	
	// Control inputs
	ioport_set_pin_mode(INPUT_POTENTIOMETER_GPIO, INPUT_POTENTIOMETER_GPIO_FLAGS);
	ioport_disable_pin(INPUT_POTENTIOMETER_GPIO);
	ioport_set_pin_dir(INPUT_PUSHBUTTON_GPIO, IOPORT_DIR_INPUT);
	ioport_set_pin_mode(INPUT_PUSHBUTTON_GPIO, IOPORT_MODE_PULLDOWN);
	ioport_set_pin_dir(INPUT_ENABLE_GPIO, IOPORT_DIR_INPUT);
	ioport_set_pin_mode(INPUT_ENABLE_GPIO, IOPORT_MODE_PULLDOWN);
	
	// Pressure sensors
	ioport_set_pin_mode(PRESSURE_SENSOR_0_GPIO, PRESSURE_SENSOR_0_GPIO_FLAGS);
	ioport_disable_pin(PRESSURE_SENSOR_0_GPIO);
	ioport_set_pin_mode(PRESSURE_SENSOR_1_GPIO, PRESSURE_SENSOR_1_GPIO_FLAGS);
	ioport_disable_pin(PRESSURE_SENSOR_1_GPIO);
	ioport_set_pin_mode(PRESSURE_SENSOR_2_GPIO, PRESSURE_SENSOR_2_GPIO_FLAGS);
	ioport_disable_pin(PRESSURE_SENSOR_2_GPIO);
	
	// Flow meter
	ioport_set_pin_dir(FLOW_SENSOR_POWER_GPIO, IOPORT_DIR_OUTPUT);
	ioport_set_pin_level(FLOW_SENSOR_POWER_GPIO, !FLOW_SENSOR_POWER_ACTIVE_LEVEL);
	ioport_set_pin_mode(FLOW_SENSOR_SDA_GPIO, FLOW_SENSOR_SDA_GPIO_FLAGS);
	ioport_disable_pin(FLOW_SENSOR_SDA_GPIO);
	ioport_set_pin_mode(FLOW_SENSOR_SCL_GPIO, FLOW_SENSOR_SCL_GPIO_FLAGS);
	ioport_disable_pin(FLOW_SENSOR_SCL_GPIO);
	ioport_set_pin_mode(FLOW_SENSOR_ANALOG_GPIO, FLOW_SENSOR_ANALOG_GPIO_FLAGS);
	ioport_disable_pin(FLOW_SENSOR_ANALOG_GPIO);
	
	// Screen
	ioport_set_pin_dir(LCD_POWER_GPIO, IOPORT_DIR_OUTPUT);
	ioport_set_pin_level(LCD_POWER_GPIO, !LCD_POWER_ACTIVE_LEVEL);
	ioport_set_pin_mode(LCD_SDA_GPIO, LCD_SDA_GPIO_FLAGS);
	ioport_disable_pin(LCD_SDA_GPIO);
	ioport_set_pin_mode(LCD_SCL_GPIO, LCD_SCL_GPIO_FLAGS);
	ioport_disable_pin(LCD_SCL_GPIO);
	
	// FRAM interface
	ioport_set_pin_dir(FRAM_CS_GPIO, IOPORT_DIR_OUTPUT);
	ioport_set_pin_level(FRAM_CS_GPIO, !FRAM_CS_SELECT_LEVEL);
	ioport_set_pin_mode(FRAM_SCK_GPIO, FRAM_SCK_GPIO_FLAGS);
	ioport_disable_pin(FRAM_SCK_GPIO);
	ioport_set_pin_mode(FRAM_MISO_GPIO, FRAM_MISO_GPIO_FLAGS);
	ioport_disable_pin(FRAM_MISO_GPIO);
	ioport_set_pin_mode(FRAM_MOSI_GPIO, FRAM_MOSI_GPIO_FLAGS);
	ioport_disable_pin(FRAM_MOSI_GPIO);
	
	// Alarm pins
	ioport_set_pin_dir(BUZZER_GPIO, IOPORT_DIR_OUTPUT);
	ioport_set_pin_level(BUZZER_GPIO, !BUZZER_GPIO_ACTIVE_LEVEL);
	ioport_set_pin_dir(WATCHDOG_GPIO, IOPORT_DIR_OUTPUT);
	ioport_set_pin_level(WATCHDOG_GPIO, !WATCHDOG_GPIO_ACTIVE_LEVEL);

	// Power monitoring
	ioport_set_pin_dir(POWER_MONITOR_GPIO, IOPORT_DIR_INPUT);
	ioport_set_pin_mode(POWER_MONITOR_GPIO, IOPORT_MODE_PULLDOWN);
	
	// Vref
	ioport_set_pin_mode(VREFA_GPIO, VREFA_GPIO_FLAGS);
	ioport_disable_pin(VREFA_GPIO);
}