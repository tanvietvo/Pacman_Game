/*
 * touch.h
 *
 *  Created on: Nov 5, 2025
 *      Author: tanvietvo
 */

#ifndef INC_TOUCH_H_
#define INC_TOUCH_H_

#include <stdint.h>

#define TOUCH_CLK_PORT        TOUCH_CLK_GPIO_Port   // PG8
#define TOUCH_CLK_PIN         TOUCH_CLK_Pin

#define TOUCH_MOSI_PORT       TOUCH_MOSI_GPIO_Port  // PC9
#define TOUCH_MOSI_PIN        TOUCH_MOSI_Pin

#define TOUCH_MISO_PORT       TOUCH_MISO_GPIO_Port  // PC12
#define TOUCH_MISO_PIN        TOUCH_MISO_Pin

#define TOUCH_CS_PORT         TOUCH_CS_GPIO_Port    // PG7
#define TOUCH_CS_PIN          TOUCH_CS_Pin

#define TOUCH_IRQ_PORT        TOUCH_IRQ_GPIO_Port   // PC8 (T_PEN)
#define TOUCH_IRQ_PIN         TOUCH_IRQ_Pin

#define CMD_READ_X            0xD0
#define CMD_READ_Y            0x90

void touch_init(void);
uint8_t touch_is_pressed(void);
uint8_t touch_read_raw_xy(uint16_t *x, uint16_t *y);
uint8_t touch_get_calibrated_xy(uint16_t *x, uint16_t *y);

#endif /* INC_TOUCH_H_ */
