
#ifndef DS18B20_H
#define DS18B20_H

#include <stdint.h>
#include "stm32f4xx_ll_gpio.h"


// DS18B20 driver for one sensor only.
// Non-blocking conversion uses DELAY_GetTick() from delay.c.

typedef enum {
    DS18B20_STATE_IDLE = 0,
    DS18B20_STATE_WAIT_CONVERSION,
} DS18B20_StateTypeDef;

typedef struct {
    GPIO_TypeDef *port;
    uint32_t      pin;

    DS18B20_StateTypeDef state;
    uint32_t conv_start_tick;

    int16_t last_temp_c;
    uint8_t data_valid;
    uint8_t new_data_ready;
} DS18B20_HandleTypeDef;

uint8_t DS18B20_Init(DS18B20_HandleTypeDef *ds18b20);

// Low-level split APIs.
uint8_t DS18B20_StartConversion(DS18B20_HandleTypeDef *ds18b20);
uint8_t DS18B20_ReadTempAfterConversion(DS18B20_HandleTypeDef *ds18b20, int16_t *temp_c);

// Non-blocking task style APIs.
uint8_t DS18B20_RequestSample(DS18B20_HandleTypeDef *ds18b20, uint32_t now);
void DS18B20_Process(DS18B20_HandleTypeDef *ds18b20, uint32_t now);

uint8_t DS18B20_IsBusy(DS18B20_HandleTypeDef *ds18b20);
uint8_t DS18B20_IsNewDataReady(DS18B20_HandleTypeDef *ds18b20);
uint8_t DS18B20_GetLastTempInt(DS18B20_HandleTypeDef *ds18b20, int16_t *temp_c);
uint8_t DS18B20_IsDataValid(DS18B20_HandleTypeDef *ds18b20);

#endif
