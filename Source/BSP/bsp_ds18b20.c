/*
 * bsp_ds18b20.c
 *
 *  Created on: May 3, 2026
 *      Author: ASUS
 */

#include "bsp_ds18b20.h"

#include "ds18b20.h"
#include "delay.h"

/*
 * DS18B20 board connection:
 * PA1 -> DS18B20 DATA
 * 4.7k pull-up resistor to 3.3V
 */
static DS18B20_HandleTypeDef ds18b20_1 = {
    .port = GPIOA,
    .pin  = LL_GPIO_PIN_1
};

uint8_t bsp_ds18b20_init(void) {
    /* Do not call DELAY_Init() here. app_main_init() initializes delay.c. */
    return DS18B20_Init(&ds18b20_1);
}

uint8_t bsp_ds18b20_request_sample(void) {
    return DS18B20_RequestSample(&ds18b20_1, DELAY_GetTick());
}

void bsp_ds18b20_process(void) {
    DS18B20_Process(&ds18b20_1, DELAY_GetTick());
}

uint8_t bsp_ds18b20_is_busy(void) {
    return DS18B20_IsBusy(&ds18b20_1);
}

uint8_t bsp_ds18b20_is_new_data_ready(void) {
    return DS18B20_IsNewDataReady(&ds18b20_1);
}

uint8_t bsp_ds18b20_get_last_temp_int(int16_t *temp_c) {
    return DS18B20_GetLastTempInt(&ds18b20_1, temp_c);
}

uint8_t bsp_ds18b20_is_data_valid(void) {
    return DS18B20_IsDataValid(&ds18b20_1);
}
