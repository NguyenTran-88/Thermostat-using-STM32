/*
 * delay.c
 *
 *  Created on: May 3, 2026
 *      Author: ASUS
 */

#include "delay.h"

#include "stm32f4xx.h"


static volatile uint32_t delay_tick_ms = 0U;

static uint8_t delay_is_initialized = 0U;
static uint8_t delay_dwt_ready = 0U;


static void DELAY_InitSysTick(void);
static void DELAY_InitDWT(void);
static void DELAY_us_software(uint32_t us);


void DELAY_Init(void)
{
    if (delay_is_initialized == 0U) {
        delay_tick_ms = 0U;
        delay_is_initialized = 1U;

        DELAY_InitSysTick();
        DELAY_InitDWT();
    }
}


void DELAY_IncTick(void)
{
    delay_tick_ms++;
}


uint32_t DELAY_GetTick(void)
{
    return delay_tick_ms;
}


void DELAY_ms(uint32_t ms)
{
    while (ms > 0U) {
        DELAY_us(1000U);
        ms--;
    }
}


void DELAY_us(uint32_t us)
{
    uint32_t start;
    uint32_t ticks;

    if (us == 0U) {
        return;
    }

    if (delay_is_initialized == 0U) {
        DELAY_Init();
    }

    if (delay_dwt_ready != 0U) {
        ticks = us * (SystemCoreClock / 1000000U);
        start = DWT->CYCCNT;

        while ((uint32_t)(DWT->CYCCNT - start) < ticks) {
            __NOP();
        }
    }
    else {
        DELAY_us_software(us);
    }
}


uint8_t DELAY_IsExpired(uint32_t start_tick, uint32_t delay_ms)
{
    if ((uint32_t)(DELAY_GetTick() - start_tick) >= delay_ms) {
        return 1U;
    }

    return 0U;
}


static void DELAY_InitSysTick(void)
{
    uint32_t ticks;

    if (SystemCoreClock == 0U) {
        SystemCoreClockUpdate();
    }

    ticks = SystemCoreClock / 1000U;

    if (ticks == 0U) {
        ticks = 16000U;
    }

    (void)SysTick_Config(ticks);

    NVIC_SetPriority(SysTick_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 15U, 0U));
}


static void DELAY_InitDWT(void)
{
    uint32_t start;
    uint32_t i;

    delay_dwt_ready = 0U;

    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;

    DWT->CYCCNT = 0U;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

    start = DWT->CYCCNT;

    for (i = 0U; i < 1000U; i++) {
        __NOP();
    }

    if (DWT->CYCCNT != start) {
        delay_dwt_ready = 1U;
    }
}


static void DELAY_us_software(uint32_t us)
{
    volatile uint32_t i;
    uint32_t loops_per_us;

    loops_per_us = SystemCoreClock / 1000000U;
    loops_per_us /= 6U;

    if (loops_per_us == 0U) {
        loops_per_us = 1U;
    }

    while (us > 0U) {
        for (i = 0U; i < loops_per_us; i++) {
            __NOP();
        }

        us--;
    }
}
