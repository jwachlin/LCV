/**
 * \file
 *
 * \brief User board definition template
 *
 */

 /* This file is intended to contain definitions and configuration details for
 * features and devices that are available on the board, e.g., frequency and
 * startup time for an external crystal, external memory devices, LED and USART
 * pins.
 */
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */

#ifndef USER_BOARD_H
#define USER_BOARD_H

#include <conf_board.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \ingroup group_common_boards
 * \defgroup user_board_group User board
 *
 * @{
 */

void system_board_init(void);

/** Name string macro */
#define BOARD_NAME                "USER_BOARD"

// Motor pin definitions
#define MOTOR_ENABLE_GPIO			IOPORT_CREATE_PIN(IOPORT_PORTA, 7)	// TODO actually set these pins
#define MOTOR_ENABLE_ACTIVE_LEVEL	IOPORT_PIN_LEVEL_HIGH
#define MOTOR_NTC_GPIO				IOPORT_CREATE_PIN(IOPORT_PORTB, 8)
#define MOTOR_NTC_GPIO_FLAGS		(IOPORT_MODE_MUX_B)
#define MOTOR_READY_GPIO			IOPORT_CREATE_PIN(IOPORT_PORTA, 7)	// TODO actually set these pins
#define MOTOR_SPEED_GPIO			IOPORT_CREATE_PIN(IOPORT_PORTA, 2)
#define MOTOR_SPEED_GPIO_FLAGS		(IOPORT_MODE_MUX_B)

// Control input
#define INPUT_POTENTIOMETER_GPIO			IOPORT_CREATE_PIN(IOPORT_PORTB,9)
#define INPUT_POTENTIOMETER_GPIO_FLAGS		(IOPORT_MODE_MUX_B)
#define INPUT_PUSHBUTTON_GPIO				IOPORT_CREATE_PIN(IOPORT_PORTA, 7)	// TODO actually set these pins
#define INPUT_ENABLE_GPIO					IOPORT_CREATE_PIN(IOPORT_PORTA, 7)	// TODO actually set these pins

// Pressure sensors
#define PRESSURE_SENSOR_0_GPIO				IOPORT_CREATE_PIN(IOPORT_PORTA,4)
#define PRESSURE_SENSOR_0_GPIO_FLAGS		(IOPORT_MODE_MUX_B)
#define PRESSURE_SENSOR_1_GPIO				IOPORT_CREATE_PIN(IOPORT_PORTA,5)
#define PRESSURE_SENSOR_1_GPIO_FLAGS		(IOPORT_MODE_MUX_B)
#define PRESSURE_SENSOR_2_GPIO				IOPORT_CREATE_PIN(IOPORT_PORTA,6)
#define PRESSURE_SENSOR_2_GPIO_FLAGS		(IOPORT_MODE_MUX_B)

// Flow meter pins
#define FLOW_SENSOR_POWER_GPIO				IOPORT_CREATE_PIN(IOPORT_PORTA, 7)	// TODO actually set these pins
#define FLOW_SENSOR_POWER_ACTIVE_LEVEL		IOPORT_PIN_LEVEL_HIGH
#define FLOW_SENSOR_SDA_GPIO				IOPORT_CREATE_PIN(IOPORT_PORTA, 22)
#define FLOW_SENSOR_SDA_GPIO_FLAGS			(IOPORT_MODE_MUX_C)
#define FLOW_SENSOR_SCL_GPIO				IOPORT_CREATE_PIN(IOPORT_PORTA, 23)
#define FLOW_SENSOR_SCL_GPIO_FLAGS			(IOPORT_MODE_MUX_C)

// LCD screen pins
#define LCD_POWER_GPIO				IOPORT_CREATE_PIN(IOPORT_PORTA, 7)	// TODO actually set these pins
#define LCD_POWER_ACTIVE_LEVEL		IOPORT_PIN_LEVEL_HIGH
#define LCD_SDA_GPIO				IOPORT_CREATE_PIN(IOPORT_PORTA, 16)
#define LCD_SDA_GPIO_FLAGS			(IOPORT_MODE_MUX_C)
#define LCD_SCL_GPIO				IOPORT_CREATE_PIN(IOPORT_PORTA, 17)
#define LCD_SCL_GPIO_FLAGS			(IOPORT_MODE_MUX_C)

// FRAM pins
#define FRAM_SCK_GPIO				IOPORT_CREATE_PIN(IOPORT_PORTA, 9)
#define FRAM_SCK_GPIO_FLAGS			(IOPORT_MODE_MUX_C)
#define FRAM_MISO_GPIO				IOPORT_CREATE_PIN(IOPORT_PORTA, 10)
#define FRAM_MISO_GPIO_FLAGS		(IOPORT_MODE_MUX_C)
#define FRAM_MOSI_GPIO				IOPORT_CREATE_PIN(IOPORT_PORTA, 8)
#define FRAM_MOSI_GPIO_FLAGS		(IOPORT_MODE_MUX_C)
#define FRAM_CS_GPIO				IOPORT_CREATE_PIN(IOPORT_PORTA, 11)
#define FRAM_CS_SELECT_LEVEL		IOPORT_PIN_LEVEL_LOW

// Alarms
#define BUZZER_GPIO						IOPORT_CREATE_PIN(IOPORT_PORTA, 7)	// TODO actually set these pins
#define BUZZER_GPIO_ACTIVE_LEVEL		IOPORT_PIN_LEVEL_HIGH
#define WATCHDOG_GPIO					IOPORT_CREATE_PIN(IOPORT_PORTA, 7)	// TODO actually set these pins
#define WATCHDOG_GPIO_ACTIVE_LEVEL		IOPORT_PIN_LEVEL_HIGH

/** @} */

#ifdef __cplusplus
}
#endif

#endif // USER_BOARD_H
