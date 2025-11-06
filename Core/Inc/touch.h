/*
 * touch.h
 *
 *  Created on: Nov 5, 2025
 *      Author: tanvietvo
 */

#ifndef INC_TOUCH_H_
#define INC_TOUCH_H_

#include <stdint.h>

#define TOUCH_SPI_HANDLE      hspi2
#define TOUCH_CS_PORT         TOUCH_CS_GPIO_Port
#define TOUCH_CS_PIN          TOUCH_CS_Pin
#define TOUCH_IRQ_PORT        TOUCH_IRQ_GPIO_Port
#define TOUCH_IRQ_PIN         TOUCH_IRQ_Pin

// Hằng số cho XPT2046
#define CMD_READ_X            0xD0
#define CMD_READ_Y            0x90

uint8_t touch_is_pressed();

// Hàm đọc tọa độ GỐC (0-4095)
uint8_t touch_read_raw_xy(uint16_t *x, uint16_t *y);

// Hàm đọc tọa độ đã hiệu chỉnh (0-240, 0-320)
uint8_t touch_get_calibrated_xy(uint16_t *x, uint16_t *y);

#endif /* INC_TOUCH_H_ */
