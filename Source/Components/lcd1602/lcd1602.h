
#ifndef LCD1602_H
#define LCD1602_H

#include "stm32f4xx_ll_gpio.h"
#include <stdint.h>

typedef struct {
    GPIO_TypeDef *rs_port;
    uint32_t      rs_pin;

    GPIO_TypeDef *en_port;
    uint32_t      en_pin;

    GPIO_TypeDef *d4_port;
    uint32_t      d4_pin;

    GPIO_TypeDef *d5_port;
    uint32_t      d5_pin;

    GPIO_TypeDef *d6_port;
    uint32_t      d6_pin;

    GPIO_TypeDef *d7_port;
    uint32_t      d7_pin;

    uint8_t       cols;
    uint8_t       rows;
} LCD1602_HandleTypeDef;

void LCD1602_Init(LCD1602_HandleTypeDef *lcd);
void LCD1602_Clear(LCD1602_HandleTypeDef *lcd);
void LCD1602_DisplayOn(LCD1602_HandleTypeDef *lcd);
void LCD1602_DisplayOff(LCD1602_HandleTypeDef *lcd);
void LCD1602_SetCursor(LCD1602_HandleTypeDef *lcd, uint8_t row, uint8_t col);
void LCD1602_Print(LCD1602_HandleTypeDef *lcd, const char *text);
void LCD1602_PrintLine(LCD1602_HandleTypeDef *lcd, uint8_t row, const char *text);
void LCD1602_PrintCentered(LCD1602_HandleTypeDef *lcd, uint8_t row, const char *text);

#endif /* LCD1602_H */
