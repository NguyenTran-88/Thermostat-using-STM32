/*
 * ds18b20.c
 *
 *  Created on: May 3, 2026
 *      Author: ASUS
 */

#include "ds18b20.h"
#include "delay.h"


// DS18B20 command list
#define DS18B20_CMD_SKIP_ROM          0xCCU
#define DS18B20_CMD_CONVERT_T         0x44U
#define DS18B20_CMD_READ_SCRATCHPAD   0xBEU
#define DS18B20_CMD_WRITE_SCRATCHPAD  0x4EU
#define DS18B20_RESOLUTION_9BIT       0x1FU

#define DS18B20_DEFAULT_TH            75U
#define DS18B20_DEFAULT_TL            70U
#define DS18B20_CONVERSION_TIME_MS    100U


static uint8_t DS18B20_GetPinPosition(uint32_t pin) {
    uint8_t pos = 0U;

    while (((pin & (1UL << pos)) == 0U) && (pos < 16U)) {
        pos++;
    }

    return pos;
}


// Release 1-Wire bus by changing PA1 to input mode.
// This is safer than driving PA1 high, because DS18B20 must be able to pull DQ low.
static void DS18B20_ReleaseBus(DS18B20_HandleTypeDef *ds18b20) {
    uint8_t pos;
    uint32_t shift;

    pos = DS18B20_GetPinPosition(ds18b20->pin);
    shift = (uint32_t)pos * 2U;

    // MODER: 00 = input
    ds18b20->port->MODER &= ~(3UL << shift);

    // PUPDR: 01 = pull-up. External 4.7k pull-up is still required/recommended.
    ds18b20->port->PUPDR &= ~(3UL << shift);
    ds18b20->port->PUPDR |=  (1UL << shift);
}


// Pull 1-Wire bus low by changing PA1 to output open-drain low.
static void DS18B20_PullBusLow(DS18B20_HandleTypeDef *ds18b20) {
    uint8_t pos;
    uint32_t shift;

    pos = DS18B20_GetPinPosition(ds18b20->pin);
    shift = (uint32_t)pos * 2U;

    // Prepare output level low before changing mode to output.
    LL_GPIO_ResetOutputPin(ds18b20->port, ds18b20->pin);

    // OTYPER: 1 = open-drain
    ds18b20->port->OTYPER |= (1UL << pos);

    // OSPEEDR: 00 = low speed is enough for 1-Wire
    ds18b20->port->OSPEEDR &= ~(3UL << shift);

    // PUPDR: no pull while output low
    ds18b20->port->PUPDR &= ~(3UL << shift);

    // MODER: 01 = output
    ds18b20->port->MODER &= ~(3UL << shift);
    ds18b20->port->MODER |=  (1UL << shift);
}


static uint8_t DS18B20_ReadBus(DS18B20_HandleTypeDef *ds18b20) {
    if (LL_GPIO_IsInputPinSet(ds18b20->port, ds18b20->pin)) {
        return 1U;
    }

    return 0U;
}


// Send reset pulse and check presence pulse from DS18B20.
static uint8_t DS18B20_Reset(DS18B20_HandleTypeDef *ds18b20) {
    uint8_t presence;

    DS18B20_ReleaseBus(ds18b20);
    DELAY_us(5U);

    DS18B20_PullBusLow(ds18b20);
    DELAY_us(480U);

    DS18B20_ReleaseBus(ds18b20);
    DELAY_us(70U);

    presence = (DS18B20_ReadBus(ds18b20) == 0U) ? 1U : 0U;

    DELAY_us(410U);

    return presence;
}


// Write one bit to DS18B20.
// To write 1: pull low briefly, then release.
// To write 0: pull low for longer time.
static void DS18B20_WriteBit(DS18B20_HandleTypeDef *ds18b20, uint8_t bit_value) {
    if (bit_value) {
        DS18B20_PullBusLow(ds18b20);
        DELAY_us(6U);
        DS18B20_ReleaseBus(ds18b20);
        DELAY_us(64U);
    }
    else {
        DS18B20_PullBusLow(ds18b20);
        DELAY_us(60U);
        DS18B20_ReleaseBus(ds18b20);
        DELAY_us(10U);
    }
}


// Read one bit from DS18B20.
// Read slot by pulling DQ low shortly, then release and sample.
static uint8_t DS18B20_ReadBit(DS18B20_HandleTypeDef *ds18b20) {
    uint8_t bit_value;

    DS18B20_PullBusLow(ds18b20);
    DELAY_us(6U);

    DS18B20_ReleaseBus(ds18b20);
    DELAY_us(9U);

    bit_value = DS18B20_ReadBus(ds18b20);

    DELAY_us(55U);

    return bit_value;
}


// Write one byte, LSB first.
static void DS18B20_WriteByte(DS18B20_HandleTypeDef *ds18b20, uint8_t data) {
    uint8_t i;

    for (i = 0U; i < 8U; i++) {
        DS18B20_WriteBit(ds18b20, data & 0x01U);
        data >>= 1U;
    }
}


// Read one byte, LSB first.
static uint8_t DS18B20_ReadByte(DS18B20_HandleTypeDef *ds18b20) {
    uint8_t i;
    uint8_t data = 0U;

    for (i = 0U; i < 8U; i++) {
        if (DS18B20_ReadBit(ds18b20)) {
            data |= (uint8_t)(1U << i);
        }
    }

    return data;
}


// Calculate CRC8 for scratchpad data.
// If calculated CRC equals byte 8, the reading is valid.
static uint8_t DS18B20_CalcCRC8(uint8_t *data, uint8_t length) {
    uint8_t crc = 0U;
    uint8_t i;
    uint8_t j;

    for (i = 0U; i < length; i++) {
        crc ^= data[i];

        // If lost bit was 0: just shift.
        // If lost bit was 1: shift, then apply DS18B20 correction pattern.
        for (j = 0U; j < 8U; j++) {
            if (crc & 0x01U) {
                crc = (uint8_t)((crc >> 1U) ^ 0x8CU);
            }
            else {
                crc >>= 1U;
            }
        }
    }

    return crc;
}


// Convert raw DS18B20 value to integer Celsius.
// DS18B20 raw value uses 1/16 degree Celsius per bit.
// This function rounds to nearest integer.
static int16_t DS18B20_RawToIntCelsius(int16_t raw) {
    if (raw >= 0) {
        return (int16_t)((raw + 8) / 16);
    }

    return (int16_t)((raw - 8) / 16);
}


// Read scratchpad only. Conversion must already be completed before this function.
static uint8_t DS18B20_ReadScratchpad(DS18B20_HandleTypeDef *ds18b20, uint8_t scratchpad[9]) {
    uint8_t i;

    if ((ds18b20 == 0) || (scratchpad == 0)) {
        return 0U;
    }

    if (DS18B20_Reset(ds18b20) == 0U) {
        return 0U;
    }

    DS18B20_WriteByte(ds18b20, DS18B20_CMD_SKIP_ROM);
    DS18B20_WriteByte(ds18b20, DS18B20_CMD_READ_SCRATCHPAD);

    for (i = 0U; i < 9U; i++) {
        scratchpad[i] = DS18B20_ReadByte(ds18b20);
    }

    if (DS18B20_CalcCRC8(scratchpad, 8U) != scratchpad[8]) {
        return 0U;
    }

    return 1U;
}


// Initialize DS18B20 protocol.
uint8_t DS18B20_Init(DS18B20_HandleTypeDef *ds18b20) {
    if (ds18b20 == 0) {
        return 0U;
    }

    ds18b20->state = DS18B20_STATE_IDLE;
    ds18b20->conv_start_tick = 0U;
    ds18b20->last_temp_c = 0;
    ds18b20->data_valid = 0U;
    ds18b20->new_data_ready = 0U;

    // Make sure PA1 is released before starting 1-Wire.
    DS18B20_ReleaseBus(ds18b20);

    if (DS18B20_Reset(ds18b20) == 0U) {
        return 0U;
    }

    DS18B20_WriteByte(ds18b20, DS18B20_CMD_SKIP_ROM);
    DS18B20_WriteByte(ds18b20, DS18B20_CMD_WRITE_SCRATCHPAD);

    // TH and TL are alarm features, DS18B20 requires them.
    DS18B20_WriteByte(ds18b20, DS18B20_DEFAULT_TH);
    DS18B20_WriteByte(ds18b20, DS18B20_DEFAULT_TL);

    // Use 9-bit resolution, so conversion time is about 94 ms.
    DS18B20_WriteByte(ds18b20, DS18B20_RESOLUTION_9BIT);

    return 1U;
}


// Start temperature conversion only. Do not read temperature here.
uint8_t DS18B20_StartConversion(DS18B20_HandleTypeDef *ds18b20) {
    if (ds18b20 == 0) {
        return 0U;
    }

    if (DS18B20_Reset(ds18b20) == 0U) {
        return 0U;
    }

    DS18B20_WriteByte(ds18b20, DS18B20_CMD_SKIP_ROM);
    DS18B20_WriteByte(ds18b20, DS18B20_CMD_CONVERT_T);

    return 1U;
}


// Read temperature after conversion time has passed.
uint8_t DS18B20_ReadTempAfterConversion(DS18B20_HandleTypeDef *ds18b20, int16_t *temp_c) {
    uint8_t scratchpad[9];
    int16_t raw;

    if ((ds18b20 == 0) || (temp_c == 0)) {
        return 0U;
    }

    if (DS18B20_ReadScratchpad(ds18b20, scratchpad) == 0U) {
        ds18b20->data_valid = 0U;
        ds18b20->new_data_ready = 0U;
        return 0U;
    }

    raw = (int16_t)(((uint16_t)scratchpad[1] << 8U) | scratchpad[0]);
    *temp_c = DS18B20_RawToIntCelsius(raw);

    ds18b20->last_temp_c = *temp_c;
    ds18b20->data_valid = 1U;
    ds18b20->new_data_ready = 1U;

    return 1U;
}


// Non-blocking: request one sample, then return immediately.
// Call DS18B20_Process() later to finish the read.
uint8_t DS18B20_RequestSample(DS18B20_HandleTypeDef *ds18b20, uint32_t now) {
    if (ds18b20 == 0) {
        return 0U;
    }

    if (ds18b20->state != DS18B20_STATE_IDLE) {
        return 0U;
    }

    if (DS18B20_StartConversion(ds18b20) == 0U) {
        ds18b20->data_valid = 0U;
        ds18b20->new_data_ready = 0U;
        ds18b20->state = DS18B20_STATE_IDLE;
        return 0U;
    }

    ds18b20->conv_start_tick = now;
    ds18b20->state = DS18B20_STATE_WAIT_CONVERSION;
    ds18b20->new_data_ready = 0U;

    return 1U;
}


// Non-blocking driver process function.
// Call this repeatedly in while(1).
void DS18B20_Process(DS18B20_HandleTypeDef *ds18b20, uint32_t now) {
    int16_t temp_c;

    if (ds18b20 == 0) {
        return;
    }

    switch (ds18b20->state) {
        case DS18B20_STATE_IDLE:
        {
            break;
        }

        case DS18B20_STATE_WAIT_CONVERSION:
        {
            if ((uint32_t)(now - ds18b20->conv_start_tick) >= DS18B20_CONVERSION_TIME_MS) {
                if (DS18B20_ReadTempAfterConversion(ds18b20, &temp_c) == 0U) {
                    ds18b20->data_valid = 0U;
                    ds18b20->new_data_ready = 0U;
                }

                ds18b20->state = DS18B20_STATE_IDLE;
            }
            break;
        }

        default:
        {
            ds18b20->state = DS18B20_STATE_IDLE;
            break;
        }
    }
}


uint8_t DS18B20_IsBusy(DS18B20_HandleTypeDef *ds18b20) {
    if (ds18b20 == 0) {
        return 0U;
    }

    if (ds18b20->state != DS18B20_STATE_IDLE) {
        return 1U;
    }

    return 0U;
}


uint8_t DS18B20_IsNewDataReady(DS18B20_HandleTypeDef *ds18b20) {
    if (ds18b20 == 0) {
        return 0U;
    }

    return ds18b20->new_data_ready;
}


uint8_t DS18B20_GetLastTempInt(DS18B20_HandleTypeDef *ds18b20, int16_t *temp_c) {
    if ((ds18b20 == 0) || (temp_c == 0)) {
        return 0U;
    }

    if (ds18b20->data_valid == 0U) {
        return 0U;
    }

    *temp_c = ds18b20->last_temp_c;
    ds18b20->new_data_ready = 0U;

    return 1U;
}


uint8_t DS18B20_IsDataValid(DS18B20_HandleTypeDef *ds18b20) {
    if (ds18b20 == 0) {
        return 0U;
    }

    return ds18b20->data_valid;
}
