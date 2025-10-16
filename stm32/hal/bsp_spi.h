/**
  ******************************************************************************
  * @file        : bsp_spi.h
  * @author      : ZJY
  * @version     : V1.0
  * @date        : 2025-10-16
  * @brief       : STM32 SPI驱动头文件
  * @attention   : None
  ******************************************************************************
  * @history     :
  *         V1.0 : 1.Initial version
  *
  ******************************************************************************
  */
#ifndef __BSP_SPI_H__
#define __BSP_SPI_H__

#ifdef __cplusplus
 extern "C" {
#endif /* __cplusplus */

/* Includes ------------------------------------------------------------------*/

/* Exported define -----------------------------------------------------------*/

/* Exported typedef ----------------------------------------------------------*/

/* Exported function prototypes ----------------------------------------------*/
int bsp_spi_init(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BSP_SPI_H__ */
