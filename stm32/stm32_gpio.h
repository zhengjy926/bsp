/**
  ******************************************************************************
  * @copyright   : Copyright To Hangzhou Dinova EP Technology Co.,Ltd
  * @file        : xxx.h
  * @author      : ZJY
  * @version     : V1.0
  * @data        : 20xx-xx-xx
  * @brief       : 
  * @attattention: None
  ******************************************************************************
  * @history     :
  *         V1.0 : 1.xxx
  *
  *
  ******************************************************************************
  */
#ifndef __STM32_GPIO_H__
#define __STM32_GPIO_H__

#ifdef __cplusplus
 extern "C" {
#endif /* __cplusplus */

/* Includes ------------------------------------------------------------------*/
#include "board.h"
#include "sys_def.h"
/* Exported define -----------------------------------------------------------*/
#define __STM32_PORT(port)  GPIO##port##_BASE

#define GET_PIN(PORTx,PIN) (int)((16 * ( ((uint32_t)__STM32_PORT(PORTx) - (uint32_t)GPIOA_BASE)/(0x0400UL) )) + PIN)

/* Exported typedef ----------------------------------------------------------*/
struct pin_irq_map
{
    uint16_t pinbit;
    IRQn_Type irqno;
};
/* Exported macro ------------------------------------------------------------*/

/* Exported variable prototypes ----------------------------------------------*/

/* Exported function prototypes ----------------------------------------------*/
void stm32_gpio_init(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __STM32_GPIO_H__ */

