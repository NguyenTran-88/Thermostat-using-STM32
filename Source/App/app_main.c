/*
 * app_main.c
 *
 *  Created on: May 3, 2026
 *      Author: ASUS
 */

#include "app_main.h"

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

#include "bsp_button.h"
#include "bsp_ds18b20.h"
#include "bsp_fan.h"
#include "bsp_lcd.h"
#include "delay.h"

#define APP_TEMP_READ_PERIOD_MS     500U

#define APP_SETPOINT_MIN_C          0
#define APP_SETPOINT_MAX_C          70
#define APP_SETPOINT_DEFAULT_C      28


typedef enum
{
    S_THERMO_SLEEP = 0,
    S_THERMO_NORMAL,
    S_THERMO_SETPOINT
} app_state_t;

static app_state_t app_state = S_THERMO_SLEEP;

static uint32_t temp_read_tick = 0U;

static int16_t current_temp_c = 0;
static uint8_t current_temp_valid = 0U;

static uint8_t ds18b20_ready = 0U;
static uint8_t lcd_update = 0U;

static int16_t setpoint_c = APP_SETPOINT_DEFAULT_C;
static int16_t setpoint_edit = APP_SETPOINT_DEFAULT_C;

static void app_button_handle(uint8_t button_event);
static void app_sensor_handle(void);
static void app_fan_handle(void);
static void app_lcd_handle(void);


void app_main_init(void) {
    DELAY_Init();

    bsp_button_init();
    bsp_lcd_init();
    bsp_fan_init();

    app_state = S_THERMO_SLEEP;
    temp_read_tick = DELAY_GetTick();

    current_temp_c = 0;
    current_temp_valid = 0U;
    setpoint_c = APP_SETPOINT_DEFAULT_C;
    setpoint_edit = APP_SETPOINT_DEFAULT_C;

    bsp_fan_off();

    ds18b20_ready = bsp_ds18b20_init();
    if (ds18b20_ready != 0U) {
        (void)bsp_ds18b20_request_sample();
    }

    lcd_update = 1U;
    app_lcd_handle();
}

void app_main_fsm(void) {
    uint8_t button_event = bsp_button_get_events();

    app_button_handle(button_event);
    app_sensor_handle();
    app_fan_handle();
    app_lcd_handle();
}

static void app_button_handle(uint8_t button_event) {
    if (button_event == BSP_BUTTON_EVENT_NONE) {
        return;
    }

    switch (app_state) {
        case S_THERMO_SLEEP:
            if ((button_event & BSP_BUTTON_EVENT_SLEEP) != 0U) {
                app_state = S_THERMO_NORMAL;
                lcd_update = 1U;
            }
            break;

        case S_THERMO_NORMAL:
            if ((button_event & BSP_BUTTON_EVENT_SLEEP) != 0U) {
                app_state = S_THERMO_SLEEP;
                bsp_fan_off();
                lcd_update = 1U;
            }
            else if ((button_event & BSP_BUTTON_EVENT_SET) != 0U) {
                setpoint_edit = setpoint_c;
                app_state = S_THERMO_SETPOINT;
                lcd_update = 1U;
            }
            else {
                /* No action. */
            }
            break;

        case S_THERMO_SETPOINT:
            if ((button_event & BSP_BUTTON_EVENT_SET) != 0U) {
                setpoint_c = setpoint_edit;
                app_state = S_THERMO_NORMAL;
                lcd_update = 1U;
            }
            else {
                if (((button_event & BSP_BUTTON_EVENT_UP) != 0U) &&
                    (setpoint_edit < APP_SETPOINT_MAX_C)) {
                    setpoint_edit++;
                    lcd_update = 1U;
                }

                if (((button_event & BSP_BUTTON_EVENT_DOWN) != 0U) &&
                    (setpoint_edit > APP_SETPOINT_MIN_C)) {
                    setpoint_edit--;
                    lcd_update = 1U;
                }
            }
            break;

        default:
            app_state = S_THERMO_SLEEP;
            bsp_fan_off();
            lcd_update = 1U;
            break;
    }
}

static void app_sensor_handle(void) {
    int16_t temp_c;

    if (ds18b20_ready != 0U) {
        bsp_ds18b20_process();

        if (bsp_ds18b20_is_new_data_ready() != 0U) {
            if (bsp_ds18b20_get_last_temp_int(&temp_c) != 0U) {
                current_temp_c = temp_c;
                current_temp_valid = 1U;
                lcd_update = 1U;
            }
        }

        if (bsp_ds18b20_is_data_valid() == 0U) {
            current_temp_valid = 0U;
        }
    }

    if (DELAY_IsExpired(temp_read_tick, APP_TEMP_READ_PERIOD_MS) == 0U) {
        return;
    }

    temp_read_tick = DELAY_GetTick();

    if (ds18b20_ready == 0U) {
        ds18b20_ready = bsp_ds18b20_init();
    }

    if (ds18b20_ready == 0U) {
        current_temp_valid = 0U;
        return;
    }

    if (bsp_ds18b20_is_busy() == 0U) {
        if (bsp_ds18b20_request_sample() == 0U) {
            ds18b20_ready = 0U;
            current_temp_valid = 0U;
            lcd_update = 1U;
        }
    }
}

static void app_fan_handle(void) {
    uint8_t fan_should_on = 0U;
    uint8_t fan_was_on = bsp_fan_is_on();

    if ((app_state != S_THERMO_SLEEP) &&
        (current_temp_valid != 0U) &&
        (current_temp_c > setpoint_c)) {
        fan_should_on = 1U;
    }

    if (fan_should_on == fan_was_on) {
        return;
    }

    if (fan_should_on != 0U) {
        bsp_fan_on();
    }
    else {
        bsp_fan_off();
    }

    lcd_update = 1U;
}

static void app_lcd_handle(void) {
    char line1[24];
    char line2[24];
    int16_t display_setpoint;
    const char *setpoint_tag;

    if (lcd_update == 0U) {
        return;
    }

    switch (app_state) {
        case S_THERMO_SLEEP:
            if (current_temp_valid != 0U) {
                snprintf(line1, sizeof(line1), "T:%02dC | SLEEP",
                         (int)current_temp_c);
            }
            else {
                snprintf(line1, sizeof(line1), "T:--C | SLEEP");
            }

            snprintf(line2, sizeof(line2), "MONITOR ONLY");
            break;

        case S_THERMO_SETPOINT:
            display_setpoint = setpoint_edit;
            setpoint_tag = ">SET";

            if (current_temp_valid != 0U) {
                snprintf(line1, sizeof(line1), "T:%02dC |%s:%02dC",
                         (int)current_temp_c,
                         setpoint_tag,
                         (int)display_setpoint);
            }
            else {
                snprintf(line1, sizeof(line1), "T:--C |%s:%02dC",
                         setpoint_tag,
                         (int)display_setpoint);
            }

            snprintf(line2, sizeof(line2), "UP/DN | SET=OK");
            break;

        case S_THERMO_NORMAL:
        default:
            display_setpoint = setpoint_c;
            setpoint_tag = " SET";

            if (current_temp_valid != 0U) {
                snprintf(line1, sizeof(line1), "T:%02dC |%s:%02dC",
                         (int)current_temp_c,
                         setpoint_tag,
                         (int)display_setpoint);
            }
            else {
                snprintf(line1, sizeof(line1), "T:--C |%s:%02dC",
                         setpoint_tag,
                         (int)display_setpoint);
            }

            if (bsp_fan_is_on() != 0U) {
                snprintf(line2, sizeof(line2), "AUTO  | FAN:ON");
            }
            else {
                snprintf(line2, sizeof(line2), "AUTO  | FAN:OFF");
            }
            break;
    }

    bsp_lcd_print_line(0U, line1);
    bsp_lcd_print_line(1U, line2);
    lcd_update = 0U;
}
