/**
  ******************************************************************************
  * @copyright   : Copyright To Hangzhou Dinova EP Technology Co.,Ltd
  * @file        : stm32_usart.h
  * @author      : ZJY
  * @version     : V1.0
  * @date        : 2025-01-27
  * @brief       : STM32 USART driver header file
  * @attention   : None
  ******************************************************************************
  * @history     :
  *         V1.0 : 1.Enhanced interface
  *
  ******************************************************************************
  */
#ifndef __STM32_USART_H__
#define __STM32_USART_H__

#ifdef __cplusplus
 extern "C" {
#endif /* __cplusplus */

/* Includes ------------------------------------------------------------------*/
#include "sys_def.h"

/* Exported define -----------------------------------------------------------*/
/* Exported typedef ----------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported variable prototypes ----------------------------------------------*/
/* Exported function prototypes ----------------------------------------------*/
int hw_usart_init(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __STM32_USART_H__ */

