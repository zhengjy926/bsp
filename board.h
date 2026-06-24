/**
  ******************************************************************************
  * @copyright   : Copyright To Hangzhou Dinova EP Technology Co.,Ltd
  * @file        : xxx.h
  * @author      : ZJY
  * @version     : V1.0
  * @data        : 20xx-xx-xx
  * @brief       : 
  * @attention   : None
  ******************************************************************************
  * @history     :
  *         V1.0 : 1.xxx
  *
  *
  ******************************************************************************
  */
#ifndef BOARD_H__
#define BOARD_H__

#ifdef __cplusplus
 extern "C" {
#endif /* __cplusplus */

/* Includes ------------------------------------------------------------------*/
#include "types.h"
#include "bsp_conf.h"

/* Exported define -----------------------------------------------------------*/

/* Exported typedef ----------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported variable prototypes ----------------------------------------------*/

/* Exported function prototypes ----------------------------------------------*/
int Board_Init(void);
void Error_Handler(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* BOARD_H__ */

