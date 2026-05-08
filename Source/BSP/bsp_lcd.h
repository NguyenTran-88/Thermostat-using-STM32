
#ifndef BSP_LCD_H
#define BSP_LCD_H

#include <stdint.h>

void bsp_lcd_init(void);

void bsp_lcd_clear(void);
void bsp_lcd_display_on(void);
void bsp_lcd_display_off(void);

void bsp_lcd_print(uint8_t row, uint8_t col, const char *text);
void bsp_lcd_print_line(uint8_t row, const char *text);
void bsp_lcd_print_centered(uint8_t row, const char *text);

#endif /* BSP_LCD_H */
