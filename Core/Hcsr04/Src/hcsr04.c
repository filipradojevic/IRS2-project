/**
 * @file    hc-sr04.c
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
#include "hcsr04.h"

/*******************************************************************************
 * Defines
 ******************************************************************************/

/*******************************************************************************
 * Typedefs
 ******************************************************************************/
extern TIM_HandleTypeDef htim2;


/*******************************************************************************
 * Variables
 ******************************************************************************/
extern volatile timer_tick_t start_time;
extern volatile timer_tick_t end_time;
extern volatile echo_state_t echo_state;


/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

HAL_StatusTypeDef HCSR04_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // Omogući clock za GPIO port A
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    // Konfiguriši TRIG pin kao izlaz
    GPIO_InitStruct.Pin = HS_SR04_TRIG_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(HS_SR04_TRIG_PORT, &GPIO_InitStruct);
    // Konfiguriši ECHO pin kao ulaz sa prekidom na obe ivice (Rising i Falling)
    GPIO_InitStruct.Pin = HS_SR04_ECHO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(HS_SR04_ECHO_PORT, &GPIO_InitStruct);
    

    // Omogući EXTI prekid
    HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(EXTI0_IRQn);


    return HAL_OK;
}

// Funkcija koja meri udaljenost (u cm)
float HCSR04_measure_distance_cm(void)
{
    if(echo_state == MEASURING_ECHO_DATA)
    {
        uint32_t duration;

        if(end_time >= start_time)
            duration = end_time - start_time;
        else
            duration = (0xFFFFFFFF - start_time) + end_time + 1;

        if(duration == 0){
            return -1.0f;
        }
        echo_state = VALIDATE_MEASURE;

        // brzina zvuka ~343 m/s => 0.0343 cm/us
        // udaljenost = (vreme u us) * brzina / 2
        float distance = (duration * 0.0343f) / 2.0f;

        return distance;
    }
    else
    {
        return -1.0f; // Nema validnog merenja
    }
}

void delay_us(uint32_t us)
{
    __HAL_TIM_SET_COUNTER(&htim3, 0);  // reset counter
    HAL_TIM_Base_Start(&htim3);
    while (__HAL_TIM_GET_COUNTER(&htim3) < us);
    HAL_TIM_Base_Stop(&htim3);
}


void HCSR04_Trigger(void)
{
    HAL_GPIO_WritePin(HS_SR04_TRIG_PORT, HS_SR04_TRIG_PIN, GPIO_PIN_RESET); // resetuj za svaki slučaj
    delay_us(2); // mini delay da se očisti
    // Set TRIG pin HIGH to start the ultrasonic burst
    HAL_GPIO_WritePin(HS_SR04_TRIG_PORT, HS_SR04_TRIG_PIN, GPIO_PIN_SET);
    
    // Wait for 20 microseconds (pulse duration required by HC-SR04)
    delay_us(10);
    
    // Set TRIG pin LOW to finish the pulse
    HAL_GPIO_WritePin(HS_SR04_TRIG_PORT, HS_SR04_TRIG_PIN, GPIO_PIN_RESET);
}