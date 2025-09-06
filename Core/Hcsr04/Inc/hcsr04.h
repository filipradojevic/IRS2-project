#ifndef _HCSR04_H
#define _HCSR04_H

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************
 * Includes
****************************************************************/
#include "stm32l4xx_hal_gpio.h"

/****************************************************************
 * Defines
****************************************************************/
#define HS_SR04_ECHO_PORT GPIOB
#define HS_SR04_ECHO_PIN  GPIO_PIN_0
#define HS_SR04_TRIG_PORT GPIOA
#define HS_SR04_TRIG_PIN  GPIO_PIN_6

/****************************************************************
 * Typedefs
****************************************************************/
typedef uint32_t timer_tick_t;
typedef float hcsr04_distance_t;

typedef enum {
    ECHO_NOT_CAPTURED = 0,
    ECHO_CAPTURED = 1
} echo_t;

typedef enum{
    WAITING_RISING_EDGE  = 0,
    WAITING_FALLING_EDGE = 1,
    MEASURING_ECHO_DATA  = 2,
    VALIDATE_MEASURE     = 3,
    UNKNOWN_ECHO_SPIKES  = 4
} echo_state_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
HAL_StatusTypeDef HCSR04_Init(void);
void HCSR04_Trigger(void);
float HCSR04_measure_distance_cm(void);
void delay_us(uint32_t us);


#ifdef __cplusplus
}
#endif

#endif /* _HCSR04_H*/
