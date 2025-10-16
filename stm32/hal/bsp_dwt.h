/**
  ******************************************************************************
  * @file        : stm32_dwt.h
  * @author      : ZJY
  * @version     : V1.0
  * @date        : 2024-09-26
  * @brief       : 
  * @attention   : None
  ******************************************************************************
  * @history     :
  *         V1.0 : 1.xxx
  ******************************************************************************
  */
#ifndef __STM32_DWT_H__
#define __STM32_DWT_H__

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
int32_t stm32_dwt_init(void);
void dwt_update_overflow_counter(void);
double dwt_get_seconds(void);
void dwt_delay_us(uint32_t time_us);
void dwt_delay_ms(uint32_t time_ms);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __STM32_DWT_H__ */

