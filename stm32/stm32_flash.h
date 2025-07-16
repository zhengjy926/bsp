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
#ifndef __STM32_FLASH_H__
#define __STM32_FLASH_H__

#ifdef __cplusplus
 extern "C" {
#endif /* __cplusplus */

/* Includes ------------------------------------------------------------------*/
#include "mtd.h"
/* Exported define -----------------------------------------------------------*/
#define STM32_FLASH_START_INDEX         (12)
#define STM32_FLASH_USER_START_ADDR     (0x08100000)
#define STM32_FLASH_USER_SIZE           (1024 * 1024)
/* Exported typedef ----------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported variable prototypes ----------------------------------------------*/
extern struct mtd_info stm32_flash_info;
/* Exported function prototypes ----------------------------------------------*/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __STM32_FLASH_H__ */

