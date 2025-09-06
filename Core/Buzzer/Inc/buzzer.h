#ifndef _BUZZER_H
#define _BUZZER_H

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************
 * Includes
****************************************************************/
#include "stm32l4xx_hal_gpio.h"
#include <stdbool.h>

/****************************************************************
 * Defines
****************************************************************/
#define BUZZER_PORT GPIOA
#define BUZZER_PIN  GPIO_PIN_11

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
HAL_StatusTypeDef BUZZER_Init(void);
void buzzer_set_pwm_by_distance(float distance_cm);
uint32_t calculate_buzzer_interval(float distance);
void update_buzzer(uint32_t interval);

#ifdef __cplusplus
}
#endif

#endif /* _BUZZER_H*/
