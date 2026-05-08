#ifndef BSP_BUTTON_H
#define BSP_BUTTON_H

#include <stdint.h>


#define BSP_BUTTON_EVENT_NONE   0U
#define BSP_BUTTON_EVENT_SLEEP  (1U << 0)
#define BSP_BUTTON_EVENT_UP     (1U << 1)
#define BSP_BUTTON_EVENT_DOWN   (1U << 2)
#define BSP_BUTTON_EVENT_SET    (1U << 3)


void bsp_button_init(void);
void bsp_button_exti_callback(uint32_t exti_line);
uint8_t bsp_button_get_events(void);

#endif
