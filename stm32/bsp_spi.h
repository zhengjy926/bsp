/**
  ******************************************************************************
  * @file        : bsp_spi.h
  * @author      : ZJY
  * @version     : V1.0
  * @date        : 2025-01-XX
  * @brief       : STM32 SPI BSP驱动头文件 (LL库实现)
  * @attention   : None
  ******************************************************************************
  * @history     :
  *         V1.0 : 1. Complete refactoring with LL library
  *                2. Support new SPI framework interface
  *
  ******************************************************************************
  */
#ifndef __BSP_SPI_H__
#define __BSP_SPI_H__

#ifdef __cplusplus
 extern "C" {
#endif /* __cplusplus */

/* Includes ------------------------------------------------------------------*/
#include "spi.h"

/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/

/* Exported macros -----------------------------------------------------------*/

/* Exported variables --------------------------------------------------------*/

/* Exported functions --------------------------------------------------------*/

/**
 * @brief Initialize STM32 SPI BSP driver
 * @return 0 on success, error code on failure
 */
int bsp_spi_init(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BSP_SPI_H__ */
