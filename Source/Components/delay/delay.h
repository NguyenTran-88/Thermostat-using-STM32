
#ifndef DELAY_H
#define DELAY_H

#include <stdint.h>


void DELAY_Init(void);
void DELAY_IncTick(void);
uint32_t DELAY_GetTick(void);

void DELAY_ms(uint32_t ms);
void DELAY_us(uint32_t us);
uint8_t DELAY_IsExpired(uint32_t start_tick, uint32_t delay_ms);

#endif
