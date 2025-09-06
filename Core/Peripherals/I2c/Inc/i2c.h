#ifndef _I2C_H
#define _I2C_H

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************
 * Includes
****************************************************************/
#include "main.h"
#include "stm32l4xx_hal.h"

/****************************************************************
 * Defines
****************************************************************/

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/
void HAL_I2C_MspDeInit(I2C_HandleTypeDef* i2cHandle);
void HAL_I2C_MspInit(I2C_HandleTypeDef* i2cHandle);
void MX_I2C2_Init(void);


#ifdef __cplusplus
}
#endif

#endif /* _I2C_H */