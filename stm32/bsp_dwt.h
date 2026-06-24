/**
  ******************************************************************************
  * @file        : bsp_dwt.h
  * @author      : ZJY
  * @version     : V1.0
  * @date        : 2025-10-16
  * @brief       : STM32 DWT driver header file
  * @attention   : None
  ******************************************************************************
  * @history     :
  *         V1.0 : 1.Initial version
  ******************************************************************************
  */
#ifndef __BSP_DWT_H__
#define __BSP_DWT_H__

#ifdef __cplusplus
 extern "C" {
#endif /* __cplusplus */

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/* Exported define -----------------------------------------------------------*/

/* Exported typedef ----------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported variable prototypes ----------------------------------------------*/

/* Exported function prototypes ----------------------------------------------*/
void     BSP_DWT_Init(void);
uint32_t BSP_DWT_GetTick(void);
void     BSP_DWT_DelayUs(uint32_t time_us);
void     BSP_DWT_DelayMs(uint32_t time_ms);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BSP_DWT_H__ */

