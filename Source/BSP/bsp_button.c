/*
 * bsp_button.c
 *
 *  Created on: May 3, 2026
 *      Author: ASUS
 */

#include "bsp_button.h"

#include "stm32f4xx_ll_exti.h"
#include "delay.h"


#define BSP_BUTTON_DEBOUNCE_MS  100U

#define BSP_BUTTON_SLEEP_LINE   LL_EXTI_LINE_2
#define BSP_BUTTON_UP_LINE      LL_EXTI_LINE_3
#define BSP_BUTTON_DOWN_LINE    LL_EXTI_LINE_4
#define BSP_BUTTON_SET_LINE     LL_EXTI_LINE_5


static volatile uint8_t button_events = BSP_BUTTON_EVENT_NONE;

static uint32_t sleep_last_ms = 0U;
static uint32_t up_last_ms = 0U;
static uint32_t down_last_ms = 0U;
static uint32_t set_last_ms = 0U;


void bsp_button_init(void) {
    button_events = BSP_BUTTON_EVENT_NONE;

    sleep_last_ms = 0U;
    up_last_ms = 0U;
    down_last_ms = 0U;
    set_last_ms = 0U;
}



// Called from EXTI interrupt handler
void bsp_button_exti_callback(uint32_t exti_line) {
    uint32_t now;

    now = DELAY_GetTick();

    if (exti_line == BSP_BUTTON_SLEEP_LINE) {
        if ((sleep_last_ms == 0U) || ((uint32_t)(now - sleep_last_ms) >= BSP_BUTTON_DEBOUNCE_MS)) {
            button_events |= BSP_BUTTON_EVENT_SLEEP;
            sleep_last_ms = now;
        }
    }
    else if (exti_line == BSP_BUTTON_UP_LINE) {
        if ((up_last_ms == 0U) || ((uint32_t)(now - up_last_ms) >= BSP_BUTTON_DEBOUNCE_MS)) {
            button_events |= BSP_BUTTON_EVENT_UP;
            up_last_ms = now;
        }
    }
    else if (exti_line == BSP_BUTTON_DOWN_LINE) {
        if ((down_last_ms == 0U) || ((uint32_t)(now - down_last_ms) >= BSP_BUTTON_DEBOUNCE_MS)) {
            button_events |= BSP_BUTTON_EVENT_DOWN;
            down_last_ms = now;
        }
    }
    else if (exti_line == BSP_BUTTON_SET_LINE) {
        if ((set_last_ms == 0U) || ((uint32_t)(now - set_last_ms) >= BSP_BUTTON_DEBOUNCE_MS)) {
            button_events |= BSP_BUTTON_EVENT_SET;
            set_last_ms = now;
        }
    }
    else {
    }
}


// App calls this function to get button events, and clear after read
uint8_t bsp_button_get_events(void) {
    uint8_t events;

    __disable_irq();
    events = button_events;
    button_events = BSP_BUTTON_EVENT_NONE;
    __enable_irq();

    return events;
}
