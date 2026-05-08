
#ifndef BSP_FAN_H
#define BSP_FAN_H

#include <stdint.h>

void bsp_fan_init(void);
void bsp_fan_on(void);
void bsp_fan_off(void);
void bsp_fan_toggle(void);
uint8_t bsp_fan_is_on(void);

#endif
