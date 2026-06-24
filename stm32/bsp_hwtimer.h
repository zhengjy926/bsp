/**
  ******************************************************************************
  * @file        : bsp_hwtimer.h
  * @author      : ZJY
  * @version     : V1.0
  * @date        : 2025-10-16
  * @brief       : STM32 HWTIMER driver header file
  * @attattention: None
  ******************************************************************************
  * @history     :
  *         V1.0 : 1.Initial version
  *
  *
  ******************************************************************************
  */
#ifndef __BSP_HWTIMER_H__
#define __BSP_HWTIMER_H__

#ifdef __cplusplus
 extern "C" {
#endif /* __cplusplus */

/* Includes ------------------------------------------------------------------*/
#include "board.h"

/* Exported define -----------------------------------------------------------*/

/* Exported typedef ----------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported variable prototypes ----------------------------------------------*/

/* Exported function prototypes ----------------------------------------------*/
/**
 * @brief 初始化STM32硬件定时器驱动
 * @return 0成功，负值表示错误码
 */
int bsp_hwtimer_init(void);

/**
 * @brief TIM6和TIM7中断回调函数（在HAL_TIM_PeriodElapsedCallback中调用）
 * @param htim 定时器句柄指针
 */
void bsp_hwtimer_period_elapsed_callback(TIM_HandleTypeDef *htim);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BSP_HWTIMER_H__ */

