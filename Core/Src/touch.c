/*
 * touch.c
 *
 *  Created on: Nov 5, 2025
 *      Author: tanvietvo
 */

#include "touch.h"
#include "lcd.h"

#define RAW_X_MIN 400
#define RAW_X_MAX 3800
#define RAW_Y_MIN 320
#define RAW_Y_MAX 3800

#define SCREEN_X_MAX (lcddev.width)
#define SCREEN_Y_MAX (lcddev.height)

void touch_init(void)
{
    if (!(CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk))
    {
        CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
        DWT->CYCCNT = 0;
        DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    }
}

static void touch_delay_us(uint32_t us)
{
    uint32_t start = DWT->CYCCNT;
    uint32_t cycles = (HAL_RCC_GetHCLKFreq() / 1000000) * us;
    while ((DWT->CYCCNT - start) < cycles);
}

static uint8_t touch_spi_sendrecv(uint8_t data)
{
    uint8_t rx_data = 0;

    for (int i = 0; i < 8; i++)
    {
        // Write at Falling Edge)
        HAL_GPIO_WritePin(TOUCH_CLK_PORT, TOUCH_CLK_PIN, GPIO_PIN_RESET);
        if (data & 0x80)
            HAL_GPIO_WritePin(TOUCH_MOSI_PORT, TOUCH_MOSI_PIN, GPIO_PIN_SET);
        else
            HAL_GPIO_WritePin(TOUCH_MOSI_PORT, TOUCH_MOSI_PIN, GPIO_PIN_RESET);

        data <<= 1;
        touch_delay_us(1);

        // Read at Rising Edge)
        HAL_GPIO_WritePin(TOUCH_CLK_PORT, TOUCH_CLK_PIN, GPIO_PIN_SET);
        touch_delay_us(1);

        rx_data <<= 1;
        if (HAL_GPIO_ReadPin(TOUCH_MISO_PORT, TOUCH_MISO_PIN))
            rx_data |= 1;
    }
    return rx_data;
}

static uint16_t touch_read_raw(uint8_t cmd)
{
    uint16_t raw_val;
    uint8_t data_high, data_low;

    HAL_GPIO_WritePin(TOUCH_CS_PORT, TOUCH_CS_PIN, GPIO_PIN_RESET); // Pull CS down

    touch_spi_sendrecv(cmd);

    data_high = touch_spi_sendrecv(0x00);
    data_low = touch_spi_sendrecv(0x00);

    HAL_GPIO_WritePin(TOUCH_CS_PORT, TOUCH_CS_PIN, GPIO_PIN_SET); // Pull CS up

    raw_val = ((uint16_t)data_high << 8) | (uint16_t)data_low;
    raw_val = raw_val >> 3;

    return raw_val; // Value in range (0-4095)
}

static uint16_t map(uint16_t val, uint16_t min_in, uint16_t max_in, uint16_t min_out, uint16_t max_out)
{
    if (val <= min_in)
    	return min_out;
    if (val >= max_in)
    	return max_out;
    return (uint16_t)(((uint32_t)(val - min_in) * (uint32_t)(max_out - min_out)) / (uint32_t)(max_in - min_in) + min_out);
}

uint8_t touch_is_pressed(void)
{
    return (HAL_GPIO_ReadPin(TOUCH_IRQ_PORT, TOUCH_IRQ_PIN) == GPIO_PIN_RESET);
}

uint8_t touch_read_raw_xy(uint16_t *x, uint16_t *y)
{
    if (!touch_is_pressed())
        return 0;

    *x = touch_read_raw(CMD_READ_X);
    *y = touch_read_raw(CMD_READ_Y);

    if (*x < 100 || *x > 4000 || *y < 100 || *y > 4000)
        return 0;

    return 1;
}

uint8_t touch_get_calibrated_xy(uint16_t *x, uint16_t *y)
{
    uint16_t raw_x, raw_y;

    if (touch_read_raw_xy(&raw_x, &raw_y) == 0)
        return 0;

    *x = map(raw_x, RAW_X_MIN, RAW_X_MAX, 0, SCREEN_X_MAX);
    *y = map(raw_y, RAW_Y_MIN, RAW_Y_MAX, 0, SCREEN_Y_MAX);

    return 1;
}
