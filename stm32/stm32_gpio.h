/**
  ******************************************************************************
  * @copyright   : Copyright To Hangzhou Dinova EP Technology Co.,Ltd
  * @file        : stm32_gpio.h
  * @author      : ZJY
  * @version     : V1.0
  * @date        : 2024-09-26
  * @brief       : STM32 GPIO driver header file
  * @attention   : None
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

/* Exported typedef ----------------------------------------------------------*/

/**
 * @brief Pin interrupt map structure
 * @note This structure maps pin bits to interrupt numbers
 */
struct pin_irq_map
{
    uint16_t pinbit;     /**< Pin bit number */
    IRQn_Type irqno;     /**< Interrupt number */
};
/* Exported macro ------------------------------------------------------------*/

/* Exported variable prototypes ----------------------------------------------*/

/* Exported function prototypes ----------------------------------------------*/
void stm32_gpio_init(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __STM32_GPIO_H__ */

