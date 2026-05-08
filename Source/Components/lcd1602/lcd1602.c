/*
 * lcd1602.c
 *
 *  Created on: May 3, 2026
 *      Author: ASUS
 */

#include "lcd1602.h"
#include "delay.h"

#include <stddef.h>

#define LCD_DEFAULT_COLS              16U
#define LCD_DEFAULT_ROWS              2U

#define LCD_LOW                       0U
#define LCD_HIGH                      1U

#define LCD_CMD_CLEAR                 0x01U
#define LCD_CMD_HOME                  0x02U
#define LCD_CMD_ENTRY_MODE            0x06U
#define LCD_CMD_DISPLAY_OFF           0x08U
#define LCD_CMD_DISPLAY_ON            0x0CU
#define LCD_CMD_FUNCTION_4BIT_2LINE   0x28U
#define LCD_CMD_SET_DDRAM             0x80U

#define LCD_LINE_1_ADDR               0x00U
#define LCD_LINE_2_ADDR               0x40U

#define LCD_DELAY_POWER_ON_MS         50U
#define LCD_DELAY_CMD_MS              2U
#define LCD_DELAY_CLEAR_MS            5U
#define LCD_DELAY_ENABLE_US           50U

static uint8_t lcd_cols(LCD1602_HandleTypeDef *lcd);
static uint8_t lcd_rows(LCD1602_HandleTypeDef *lcd);
static uint8_t lcd_strlen_limit(const char *text, uint8_t max_len);
static void lcd_write_pin(GPIO_TypeDef *port, uint32_t pin, uint8_t state);
static void lcd_pulse_enable(LCD1602_HandleTypeDef *lcd);
static void lcd_write_4bits(LCD1602_HandleTypeDef *lcd, uint8_t data);
static void lcd_send(LCD1602_HandleTypeDef *lcd, uint8_t value, uint8_t rs_state);
static void lcd_command(LCD1602_HandleTypeDef *lcd, uint8_t command);
static void lcd_data(LCD1602_HandleTypeDef *lcd, uint8_t data);

// Initialize HD44780 LCD in 4-bit mode
void LCD1602_Init(LCD1602_HandleTypeDef *lcd) {
    if (lcd == NULL) {
        return;
    }

    DELAY_ms(LCD_DELAY_POWER_ON_MS);

    lcd_write_pin(lcd->rs_port, lcd->rs_pin, LCD_LOW);
    lcd_write_pin(lcd->en_port, lcd->en_pin, LCD_LOW);

    // Special startup sequence from the LCD datasheet to enter 4-bit mode
    lcd_write_4bits(lcd, 0x03U);
    DELAY_ms(5U);
    lcd_write_4bits(lcd, 0x03U);
    DELAY_us(150U);
    lcd_write_4bits(lcd, 0x03U);
    DELAY_us(150U);
    lcd_write_4bits(lcd, 0x02U);
    DELAY_us(150U);

    lcd_command(lcd, LCD_CMD_FUNCTION_4BIT_2LINE);
    lcd_command(lcd, LCD_CMD_DISPLAY_ON);
    lcd_command(lcd, LCD_CMD_CLEAR);
    lcd_command(lcd, LCD_CMD_ENTRY_MODE);
}

void LCD1602_Clear(LCD1602_HandleTypeDef *lcd) {
    if (lcd == NULL) {
        return;
    }

    lcd_command(lcd, LCD_CMD_CLEAR);
}

void LCD1602_DisplayOn(LCD1602_HandleTypeDef *lcd) {
    if (lcd == NULL) {
        return;
    }

    lcd_command(lcd, LCD_CMD_DISPLAY_ON);
}

void LCD1602_DisplayOff(LCD1602_HandleTypeDef *lcd) {
    if (lcd == NULL) {
        return;
    }

    lcd_command(lcd, LCD_CMD_DISPLAY_OFF);
}

void LCD1602_SetCursor(LCD1602_HandleTypeDef *lcd, uint8_t row, uint8_t col) {
    uint8_t addr;

    if (lcd == NULL) {
        return;
    }

    if (row >= lcd_rows(lcd)) {
        row = (uint8_t)(lcd_rows(lcd) - 1U);
    }

    if (col >= lcd_cols(lcd)) {
        col = (uint8_t)(lcd_cols(lcd) - 1U);
    }

    if (row == 0U) {
        addr = (uint8_t)(LCD_LINE_1_ADDR + col);
    }
    else {
        addr = (uint8_t)(LCD_LINE_2_ADDR + col);
    }

    lcd_command(lcd, (uint8_t)(LCD_CMD_SET_DDRAM | addr));
}

void LCD1602_Print(LCD1602_HandleTypeDef *lcd, const char *text) {
    if ((lcd == NULL) || (text == NULL)) {
        return;
    }

    while (*text != '\0') {
        lcd_data(lcd, (uint8_t)(*text));
        text++;
    }
}

void LCD1602_PrintLine(LCD1602_HandleTypeDef *lcd, uint8_t row, const char *text) {
    uint8_t col = 0U;
    uint8_t cols;

    if (lcd == NULL) {
        return;
    }

    cols = lcd_cols(lcd);
    LCD1602_SetCursor(lcd, row, 0U);

    // Print the text first, but stop at LCD width or string ending
    while ((col < cols) && (text != NULL) && (text[col] != '\0')) {
        lcd_data(lcd, (uint8_t)text[col]);
        col++;
    }

    // Fill the rest of the row with spaces to remove old characters
    while (col < cols) {
        lcd_data(lcd, (uint8_t)' ');
        col++;
    }
}

void LCD1602_PrintCentered(LCD1602_HandleTypeDef *lcd, uint8_t row, const char *text) {
    uint8_t len;
    uint8_t start_col;
    uint8_t i;

    if (lcd == NULL) {
        return;
    }

    len = lcd_strlen_limit(text, lcd_cols(lcd));
    start_col = (uint8_t)((lcd_cols(lcd) - len) / 2U);

    LCD1602_PrintLine(lcd, row, "");
    LCD1602_SetCursor(lcd, row, start_col);

    for (i = 0U; i < len; i++) {
        lcd_data(lcd, (uint8_t)text[i]);
    }
}

// Helper functions
static uint8_t lcd_cols(LCD1602_HandleTypeDef *lcd) {
    if ((lcd->cols == 0U) || (lcd->cols > 40U)) {
        return LCD_DEFAULT_COLS;
    }

    return lcd->cols;
}

static uint8_t lcd_rows(LCD1602_HandleTypeDef *lcd) {
    if ((lcd->rows == 0U) || (lcd->rows > 4U)) {
        return LCD_DEFAULT_ROWS;
    }

    return lcd->rows;
}

static uint8_t lcd_strlen_limit(const char *text, uint8_t max_len) {
    uint8_t len = 0U;

    if (text == NULL) {
        return 0U;
    }

    while ((len < max_len) && (text[len] != '\0')) {
        len++;
    }

    return len;
}

static void lcd_write_pin(GPIO_TypeDef *port, uint32_t pin, uint8_t state) {
    if (state == LCD_HIGH) {
        LL_GPIO_SetOutputPin(port, pin);
    }
    else {
        LL_GPIO_ResetOutputPin(port, pin);
    }
}

// EN pulse tells the LCD to read the data currently on D4-D7
static void lcd_pulse_enable(LCD1602_HandleTypeDef *lcd) {
    lcd_write_pin(lcd->en_port, lcd->en_pin, LCD_HIGH);
    DELAY_us(LCD_DELAY_ENABLE_US);

    lcd_write_pin(lcd->en_port, lcd->en_pin, LCD_LOW);
    DELAY_us(LCD_DELAY_ENABLE_US);
}

// LCD is in 4-bit mode, so only D4-D7 are written at one time
static void lcd_write_4bits(LCD1602_HandleTypeDef *lcd, uint8_t data) {
    data &= 0x0FU;

    lcd_write_pin(lcd->d4_port, lcd->d4_pin, (data & 0x01U) ? LCD_HIGH : LCD_LOW);
    lcd_write_pin(lcd->d5_port, lcd->d5_pin, (data & 0x02U) ? LCD_HIGH : LCD_LOW);
    lcd_write_pin(lcd->d6_port, lcd->d6_pin, (data & 0x04U) ? LCD_HIGH : LCD_LOW);
    lcd_write_pin(lcd->d7_port, lcd->d7_pin, (data & 0x08U) ? LCD_HIGH : LCD_LOW);

    DELAY_us(LCD_DELAY_ENABLE_US);
    lcd_pulse_enable(lcd);
}

// Send 8-bit command/data by splitting it into high 4 bits and low 4 bits
static void lcd_send(LCD1602_HandleTypeDef *lcd, uint8_t value, uint8_t rs_state) {
    lcd_write_pin(lcd->rs_port, lcd->rs_pin, rs_state);
    DELAY_us(LCD_DELAY_ENABLE_US);

    lcd_write_4bits(lcd, (uint8_t)(value >> 4U));
    lcd_write_4bits(lcd, (uint8_t)(value & 0x0FU));

    if ((value == LCD_CMD_CLEAR) || (value == LCD_CMD_HOME)) {
        DELAY_ms(LCD_DELAY_CLEAR_MS);
    }
    else {
        DELAY_ms(LCD_DELAY_CMD_MS);
    }
}

static void lcd_command(LCD1602_HandleTypeDef *lcd, uint8_t command) {
    lcd_send(lcd, command, LCD_LOW);
}

static void lcd_data(LCD1602_HandleTypeDef *lcd, uint8_t data) {
    lcd_send(lcd, data, LCD_HIGH);
}
