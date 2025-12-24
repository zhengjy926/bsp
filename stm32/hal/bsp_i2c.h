/**
 ******************************************************************************
 * @file        : bsp_i2c.h
 * @author      : ZJY
 * @version     : V1.0
 * @date        : 2025-01-XX
 * @brief       : STM32 I2C BSP驱动头文件 (LL库实现)
 * @attention   : None
 ******************************************************************************
 * @history     :
 *         V1.0 : 1. Complete I2C BSP driver with LL library
 *                2. Support I2C framework interface
 *
 ******************************************************************************
 */
#ifndef __BSP_I2C_H__
#define __BSP_I2C_H__

#ifdef __cplusplus
 extern "C" {
#endif /* __cplusplus */

/* Includes ------------------------------------------------------------------*/
#include "i2c.h"

/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/

/* Exported macros -----------------------------------------------------------*/

/* Exported variables --------------------------------------------------------*/

/* Exported functions --------------------------------------------------------*/

/**
 * @brief Initialize STM32 I2C BSP driver
 * @return 0 on success, error code on failure
 */
int bsp_i2c_init(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BSP_I2C_H__ */
