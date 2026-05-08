/*
 * bsp_fan.c
 *
 *  Created on: May 3, 2026
 *      Author: ASUS
 */

#include "bsp_fan.h"

#include "stm32f4xx_ll_gpio.h"

#define BSP_FAN_PORT    GPIOA
#define BSP_FAN_PIN     LL_GPIO_PIN_7

static uint8_t fan_status = 0U;

void bsp_fan_init(void) {
    bsp_fan_off();
}

void bsp_fan_on(void) {
    LL_GPIO_SetOutputPin(BSP_FAN_PORT, BSP_FAN_PIN);
    fan_status = 1U;
}

void bsp_fan_off(void) {
    LL_GPIO_ResetOutputPin(BSP_FAN_PORT, BSP_FAN_PIN);
    fan_status = 0U;
}

void bsp_fan_toggle(void) {
    if (fan_status != 0U) {
        bsp_fan_off();
    }
    else {
        bsp_fan_on();
    }
}

uint8_t bsp_fan_is_on(void) {
    return fan_status;
}
