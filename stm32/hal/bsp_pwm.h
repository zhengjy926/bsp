/**
  ******************************************************************************
  * @copyright   : Copyright To Hangzhou Dinova EP Technology Co.,Ltd
  * @file        : stm32_pwm.h
  * @author      : ZJY
  * @version     : V1.0
  * @data        : 2025-05-28
  * @brief       : STM32 PWM驱动头文件
  * @attention   : None
  ******************************************************************************
  * @history     :
  *         V1.0 : 1.add pwm driver
  *
  *
  ******************************************************************************
  */
#ifndef __STM32_PWM_H__
#define __STM32_PWM_H__

#ifdef __cplusplus
 extern "C" {
#endif /* __cplusplus */

/* Includes ------------------------------------------------------------------*/
#include "sys_def.h"

/* Exported define -----------------------------------------------------------*/
/**
 * @brief 通道号转换为HAL通道值
 */
#define TIM_CH_TO_HAL_CHANNEL(ch)  ((uint32_t)((ch - 1) << 2))  // 等价于 (ch-1)*4

/* Exported typedef ----------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/
/* Exported variable prototypes ----------------------------------------------*/
/* Exported function prototypes ----------------------------------------------*/
/**
 * @brief 初始化TIM3 PWM并注册设备
 * @return 成功返回0，失败返回负的错误码
 */
int stm32_pwm_tim3_init(void);

/**
 * @brief 初始化指定定时器的PWM
 * @param tim_num 定时器编号(1,2,3,...)
 * @return 成功返回0，失败返回负的错误码
 */
int stm32_pwm_init_timer(uint8_t tim_num);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __STM32_PWM_H__ */
