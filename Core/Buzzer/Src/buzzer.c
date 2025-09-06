/**
 * @file    buzzer.c
 * @brief   Parking-Sensor project.
 * @details .
 * @version 1.0.0
 * @date    28.07.2025
 * @author  Filip Radojevic
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "main.h"
#include "buzzer.h"

/*******************************************************************************
 * Defines
 ******************************************************************************/

/*******************************************************************************
 * Typedefs
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
extern TIM_HandleTypeDef htim1;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

HAL_StatusTypeDef BUZZER_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // Omogući clock za GPIO port A
    __HAL_RCC_GPIOA_CLK_ENABLE();

    // Konfiguriši TRIG pin kao izlaz
    GPIO_InitStruct.Pin = BUZZER_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(BUZZER_PORT, &GPIO_InitStruct);
    
    return HAL_OK;
}

void buzzer_set_pwm_by_distance(float distance_cm)
{
    // Ograniči distancu na interval [2, 100] cm
    if (distance_cm < 2.0f) distance_cm = 2.0f;
    if (distance_cm > 100.0f) distance_cm = 100.0f;

    float scale = (distance_cm - 2.0f) / (100.0f - 2.0f);

    // Limitiraj frekvenciju između 500 Hz i 3500 Hz
    uint32_t freq_min = 500;
    uint32_t freq_max = 3500;

    uint32_t freq = freq_max - (uint32_t)(scale * (freq_max - freq_min));

    uint32_t timer_clock = SystemCoreClock;
    uint32_t prescaler = (timer_clock / 1000000) - 1; // 1 MHz timer clock
    uint32_t period = (1000000 / freq) - 1;

    __HAL_TIM_SET_PRESCALER(&htim1, prescaler);
    __HAL_TIM_SET_AUTORELOAD(&htim1, period);

    uint32_t pulse = (period + 1) / 2;  // Duty 50%

    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, pulse);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, pulse);

    __HAL_TIM_SET_COUNTER(&htim1, 0);
}