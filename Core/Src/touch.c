/*
 * touch.c
 *
 *  Created on: Nov 5, 2025
 *      Author: tanvietvo
 */

#include "touch.h"
#include "spi.h"

extern SPI_HandleTypeDef TOUCH_SPI_HANDLE;

#define RAW_X_MIN 300
#define RAW_X_MAX 3800
#define RAW_Y_MIN 250
#define RAW_Y_MAX 3750

#define SCREEN_X_MAX 240
#define SCREEN_Y_MAX 320

static uint16_t map(uint16_t val, uint16_t min_in, uint16_t max_in, uint16_t min_out, uint16_t max_out)
{
    if (val <= min_in)
    	return min_out;
    if (val >= max_in)
    	return max_out;
    return (uint16_t)((double)(val - min_in) / (double)(max_in - min_in) * (double)(max_out - min_out) + (double)min_out);
}

static uint8_t touch_spi_sendrecv(uint8_t data)
{
    uint8_t rx_data = 0;
    HAL_SPI_TransmitReceive(&TOUCH_SPI_HANDLE, &data, &rx_data, 1, 100);
    return rx_data;
}


static uint16_t touch_read_raw(uint8_t cmd)
{
    uint16_t raw_val;
    uint8_t data_high, data_low;

    HAL_GPIO_WritePin(TOUCH_CS_PORT, TOUCH_CS_PIN, GPIO_PIN_RESET);

    touch_spi_sendrecv(cmd);
    data_high = touch_spi_sendrecv(0x00);
    data_low = touch_spi_sendrecv(0x00);

    HAL_GPIO_WritePin(TOUCH_CS_PORT, TOUCH_CS_PIN, GPIO_PIN_SET);

    // Data 12 bit (XPT2046) in center
    raw_val = (data_high << 8) | data_low;
    raw_val = raw_val >> 3;

    return raw_val; // 0-4095
}

uint8_t touch_is_pressed()
{
    // IRQ (PEN) low when lcd touched
    return (HAL_GPIO_ReadPin(TOUCH_IRQ_PORT, TOUCH_IRQ_PIN) == GPIO_PIN_RESET);
}

uint8_t touch_read_raw_xy(uint16_t *x, uint16_t *y)
{
    if (!touch_is_pressed())
        return 0;

    *x = touch_read_raw(CMD_READ_X);
    *y = touch_read_raw(CMD_READ_Y);

    return 1;
}

uint8_t touch_get_calibrated_xy(uint16_t *x, uint16_t *y)
{
    uint16_t raw_x, raw_y;

    if (touch_read_raw_xy(&raw_x, &raw_y) == 0)
        return 0;

    // Map (300-3800) to (0-240)
    *x = map(raw_x, RAW_X_MIN, RAW_X_MAX, 0, SCREEN_X_MAX);

    // Map (250-3750) (0-320)
    *y = map(raw_y, RAW_Y_MIN, RAW_Y_MAX, 0, SCREEN_Y_MAX);

    // uint16_t temp_y = map(raw_x, RAW_X_MIN, RAW_X_MAX, 0, SCREEN_Y_MAX);
    // uint16_t temp_x = map(raw_y, RAW_Y_MIN, RAW_Y_MAX, 0, SCREEN_X_MAX);
    // *x = temp_x;
    // *y = temp_y;

    return 1;
}
