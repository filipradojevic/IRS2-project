/**
 * @file    main.c
 * @brief   Parking-Sensor project
 * @details Parking sensor implementation with HC-SR04 ultrasonic sensor.
 *          The measured distance is displayed on the OLED screen and a buzzer
 *          is used to signalize the detected distance range.
 * @version 1.0.0
 * @date    28.07.2025
 * @author  Filip
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/

/* STM32L4 HAL peripherals */
#include "main.h"
#include "gpio.h"
#include "i2c.h"
#include "systemclock.h"
#include "timer.h"
#include "uart.h"

/* External hardware drivers */
#include "hcsr04.h"
#include "buzzer.h"
#include "ssd1306.h"
#include "ssd1306_fonts.h"

/* Standard C library */
#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>

/*******************************************************************************
 * Defines
 ******************************************************************************/
#define UART_MAX_BUFFER_LEN    100     /**< Maximum length of the UART buffer */

/*******************************************************************************
 * Typedefs
 ******************************************************************************/
/* OLED values */
typedef char    oled_value_t;          /**< Character used for OLED display  */
typedef uint8_t oled_value_size_t;     /**< String length for OLED display   */

/* UART values */
typedef char    uart_value_t;          /**< Character used for UART transfer */
typedef uint8_t uart_value_size_t;     /**< String length for UART transfer  */

/*******************************************************************************
 * Global variables
 ******************************************************************************/
/* HAL handles */
UART_HandleTypeDef huart2;
TIM_HandleTypeDef  htim1;
TIM_HandleTypeDef  htim2;
TIM_HandleTypeDef  htim3;
I2C_HandleTypeDef  hi2c2;

/* HC-SR04 echo timing */
volatile timer_tick_t start_time  = 0;        /**< Rising edge timestamp [timer ticks] */
volatile timer_tick_t end_time    = 0;        /**< Falling edge timestamp [timer ticks] */
echo_state_t          echo_state  = WAITING_RISING_EDGE; /**< Current state of echo signal */

/* UART communication */
uart_value_size_t uart_mes_len = 0;          /**< Length of the message sent via UART */
uart_value_t      uart_buffer[UART_MAX_BUFFER_LEN]; /**< UART message buffer */

/* Distance and buzzer logic */
static uint32_t          last_distance_measure = 0;     /**< Last measurement timestamp [ms] */
static hcsr04_distance_t distance              = -1.0f; /**< Last measured distance [cm] */
static uint32_t          last_buzzer_toggle    = 0;     /**< Last buzzer toggle timestamp [ms] */
static bool              buzzer_on             = false; /**< Buzzer state flag (ON/OFF) */

/*******************************************************************************
 * Constants
 ******************************************************************************/
const uint32_t measure_interval = 1;   /**< Measurement interval [ms] */
const uint32_t interval_min     = 50;  /**< Minimum buzzer toggle interval [ms] */
const uint32_t interval_max     = 500; /**< Maximum buzzer toggle interval [ms] */

/*******************************************************************************
 * System Initialization
 ******************************************************************************/
static void System_Init(void) {
    HAL_Init();
    SystemClock_Config();

    MX_GPIO_Init();
    MX_USART2_UART_Init();
    MX_TIM2_Init();
    MX_TIM3_Init();
    MX_TIM1_Init();
    MX_I2C2_Init();

    ssd1306_Init();
    ssd1306_Fill(Black);
    ssd1306_UpdateScreen();

    if (HCSR04_Init() != HAL_OK) {
        Error_Handler();
    }

    HAL_TIM_Base_Start(&htim2);
    HAL_TIM_Base_Start(&htim3);
}

/*******************************************************************************
 * Display message on OLED and UART
 ******************************************************************************/
static void Show_Message(const char *msg) {
    /* Clear OLED display */
    ssd1306_Fill(Black);

    /* Compute horizontal centering */
    uint8_t str_len = strlen(msg);
    uint8_t oled_width = 128;      // OLED width in pixels
    uint8_t char_width = 7;        // Font_7x10 character width
    uint8_t x_pos = (oled_width - (str_len * char_width)) / 2;

    /* Vertical centering for single line */
    uint8_t oled_height = 64;      // OLED height in pixels
    uint8_t char_height = 10;      // Font_7x10 character height
    uint8_t y_pos = (oled_height - char_height) / 2;

    /* Set cursor and write string */
    ssd1306_SetCursor(x_pos, y_pos);
    ssd1306_WriteString((char*)msg, Font_7x10, White);
    ssd1306_UpdateScreen();

    /* UART transmission */
    uart_mes_len = sprintf(uart_buffer, "%s\r\n", msg);
    HAL_UART_Transmit(&huart2, (uint8_t*)uart_buffer, uart_mes_len, HAL_MAX_DELAY);
}


/*******************************************************************************
 * Update OLED and UART with distance
 ******************************************************************************/
static void Display_Update(float distance) {
    char oled_buffer[32];
    int int_part = (int)distance;
    int frac_part = (int)((distance - int_part) * 100);

    if ((distance >= 2.5f && distance <= 40.0f) && (echo_state == VALIDATE_MEASURE)) {
        snprintf(oled_buffer, sizeof(oled_buffer), "Dist: %d.%02d cm", int_part, frac_part);
    } else {
        snprintf(oled_buffer, sizeof(oled_buffer), "Distance: Invalid");
    }

    /* Clear display */
    ssd1306_Fill(Black);

    /* Compute horizontal centering */
    uint8_t str_len = strlen(oled_buffer);
    uint8_t oled_width = 128;      // OLED width in pixels
    uint8_t char_width = 7;        // Font_7x10 width
    uint8_t x_pos = (oled_width - (str_len * char_width)) / 2;

    /* Vertical centering for 1 line */
    uint8_t oled_height = 64;      // OLED height in pixels
    uint8_t char_height = 10;      // Font_7x10 height
    uint8_t y_pos = (oled_height - char_height) / 2;

    ssd1306_SetCursor(x_pos, y_pos);
    ssd1306_WriteString(oled_buffer, Font_7x10, White);
    ssd1306_UpdateScreen();

    /* UART output remains the same */
    uart_mes_len = sprintf(uart_buffer, "%s\r\n", oled_buffer);
    HAL_UART_Transmit(&huart2, (uint8_t*)uart_buffer, uart_mes_len, HAL_MAX_DELAY);
}
/*******************************************************************************
 * Configure and start buzzer PWM signal
 * @param freq Frequency of the buzzer tone in Hz
 ******************************************************************************/
static void Buzzer_Start(uint32_t freq) {
    uint32_t timer_clock = SystemCoreClock;             /**< System clock frequency */
    uint32_t prescaler   = (timer_clock / 1000000) - 1; /**< Timer prescaler for 1 MHz base */
    uint32_t period      = (1000000 / freq) - 1;        /**< Timer period for given frequency */
    uint32_t pulse       = (period + 1) / 2;            /**< 50% duty cycle */

    /* Configure timer registers */
    __HAL_TIM_SET_PRESCALER(&htim1, prescaler);
    __HAL_TIM_SET_AUTORELOAD(&htim1, period);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, pulse);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, pulse);
    __HAL_TIM_SET_COUNTER(&htim1, 0);

    /* Start PWM */
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);

    buzzer_on = true;
}

/*******************************************************************************
 * Stop buzzer PWM signal
 ******************************************************************************/
static void Buzzer_Stop(void) {
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_4);

    buzzer_on = false;
}

/*******************************************************************************
 * Control buzzer behavior based on distance
 ******************************************************************************/
static void Buzzer_Control(float distance) {
    if (distance < 0) {
        /* Invalid distance → stop buzzer */
        Buzzer_Stop();
        Show_Message("Distance: Invalid");
        return;
    }

    if (distance < 2.5f) {
        /* Always ON for very close objects */
        if (!buzzer_on) {
            Buzzer_Start(2000);
        }
        return;
    }

    if (distance >= 40.0f) {
        /* Always OFF for distant objects */
        if (buzzer_on) {
            Buzzer_Stop();
        }
        return;
    }

    /* Scale toggle interval between 2.5 cm and 40 cm */
    float scale = (distance - 2.5f) / (40.0f - 2.5f);
    if (scale < 0.0f) scale = 0.0f;
    if (scale > 1.0f) scale = 1.0f;

    /* Compute toggle interval based on distance */
    uint32_t buzzer_interval = interval_min + (uint32_t)(scale * (interval_max - interval_min));
    uint32_t now = HAL_GetTick();

    /* Toggle buzzer if interval elapsed */
    if (now - last_buzzer_toggle >= buzzer_interval) {
        last_buzzer_toggle = now;

        if (buzzer_on) {
            Buzzer_Stop();
        } else {
            Buzzer_Start(2000);
        }
    }
}

/*******************************************************************************
 * Measure distance using HC-SR04
 ******************************************************************************/
static float Measure_Distance(void) {
    uint32_t now = HAL_GetTick();

    if (now - last_distance_measure >= measure_interval) {
        last_distance_measure = now;

        echo_state = WAITING_RISING_EDGE;
        HCSR04_Trigger();

        timer_tick_t timeout = now + 20;  
        float dist = -1.0f;

        while(HAL_GetTick() < timeout) {
            dist = HCSR04_measure_distance_cm();
            if(dist >= 0)
                return dist;
        }
    }
    return distance; /* Return last known distance */
}

/*******************************************************************************
 * Main function
 ******************************************************************************/
int main(void) {
    System_Init();

    while (1) {
        distance = Measure_Distance();
        Buzzer_Control(distance);
        Display_Update(distance);
    }
}

/*******************************************************************************
 * EXTI callback for HC-SR04 echo pin
 ******************************************************************************/
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if(GPIO_Pin == HS_SR04_ECHO_PIN)
    {
        switch(echo_state){
            case WAITING_RISING_EDGE: {
                /* Rising edge detected → start timing */
                if(HAL_GPIO_ReadPin(HS_SR04_ECHO_PORT, HS_SR04_ECHO_PIN) == GPIO_PIN_SET)
                {
                    start_time = __HAL_TIM_GET_COUNTER(&htim2);
                    echo_state = WAITING_FALLING_EDGE;
                }
                break;
            }
            case WAITING_FALLING_EDGE: { 
                /* Falling edge detected → stop timing */
                if(HAL_GPIO_ReadPin(HS_SR04_ECHO_PORT, HS_SR04_ECHO_PIN) == GPIO_PIN_RESET)
                {
                    end_time = __HAL_TIM_GET_COUNTER(&htim2);
                    echo_state = MEASURING_ECHO_DATA;
                }
                break;
            }
            default:
                echo_state = UNKNOWN_ECHO_SPIKES; /* Unexpected signal → mark as unknown */
                break;
        }
    }
}

/*******************************************************************************
 * Generic error handler
 ******************************************************************************/
void Error_Handler(void)
{
    __disable_irq();
    while (1) {}
}
