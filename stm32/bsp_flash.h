/**
  ******************************************************************************
  * @copyright   : Copyright To Hangzhou Dinova EP Technology Co.,Ltd
  * @file        : bsp_flash.h
  * @author      : ZJY
  * @version     : V1.0
  * @date        : 2025-10-16
  * @brief       : STM32 FLASH driver header file
  * @attention   : None
  ******************************************************************************
  * @history     :
  *         V1.0 : 1.Initial version
  *
  *
  ******************************************************************************
  */
#ifndef __BSP_FLASH_H__
#define __BSP_FLASH_H__

#ifdef __cplusplus
 extern "C" {
#endif /* __cplusplus */

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/* Exported define -----------------------------------------------------------*/

/* Exported typedef ----------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported variable prototypes ----------------------------------------------*/
extern struct mtd_info bsp_flash_info;

/* Exported function prototypes ----------------------------------------------*/
int run_all_flash_tests(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BSP_FLASH_H__ */

