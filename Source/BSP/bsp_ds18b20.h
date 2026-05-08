#ifndef BSP_DS18B20_H
#define BSP_DS18B20_H

#include <stdint.h>

uint8_t bsp_ds18b20_init(void);

// Non-blocking APIs for app_main FSM.
uint8_t bsp_ds18b20_request_sample(void);
void bsp_ds18b20_process(void);

uint8_t bsp_ds18b20_is_busy(void);
uint8_t bsp_ds18b20_is_new_data_ready(void);
uint8_t bsp_ds18b20_get_last_temp_int(int16_t *temp_c);
uint8_t bsp_ds18b20_is_data_valid(void);

#endif
