/*
 * bsp_lcd.c
 *
 *  Created on: May 3, 2026
 *      Author: ASUS
 */

#include "bsp_lcd.h"

#include "main.h"
#include "lcd1602.h"

static LCD1602_HandleTypeDef lcd1 = {
    .rs_port = GPIOB,
    .rs_pin  = LL_GPIO_PIN_8,

    .en_port = GPIOB,
    .en_pin  = LL_GPIO_PIN_9,

    .d4_port = GPIOB,
    .d4_pin  = LL_GPIO_PIN_12,

    .d5_port = GPIOB,
    .d5_pin  = LL_GPIO_PIN_13,

    .d6_port = GPIOB,
    .d6_pin  = LL_GPIO_PIN_14,

    .d7_port = GPIOB,
    .d7_pin  = LL_GPIO_PIN_15,

    .cols = 16U,
    .rows = 2U
};

void bsp_lcd_init(void) {
    LCD1602_Init(&lcd1);
}

void bsp_lcd_clear(void) {
    LCD1602_Clear(&lcd1);
}

void bsp_lcd_display_on(void) {
    LCD1602_DisplayOn(&lcd1);
}

void bsp_lcd_display_off(void) {
    LCD1602_DisplayOff(&lcd1);
}

void bsp_lcd_print(uint8_t row, uint8_t col, const char *text) {
    LCD1602_SetCursor(&lcd1, row, col);
    LCD1602_Print(&lcd1, text);
}

void bsp_lcd_print_line(uint8_t row, const char *text) {
    LCD1602_PrintLine(&lcd1, row, text);
}

void bsp_lcd_print_centered(uint8_t row, const char *text) {
    LCD1602_PrintCentered(&lcd1, row, text);
}
