/**
  ******************************************************************************
  * @file        : bsp_comp.h
  * @author      : ZJY
  * @version     : V1.0
  * @date        : 2025-11-11
  * @brief       : 
  * @attention   : None
  ******************************************************************************
  * @history     :
  *         V1.0 : 1.初始化
  ******************************************************************************
  */
#ifndef __BSP_COMP_H__
#define __BSP_COMP_H__

#ifdef __cplusplus
 extern "C" {
#endif /* __cplusplus */

/* Includes ------------------------------------------------------------------*/
#include "bsp_conf.h"

/* Exported types ------------------------------------------------------------*/


/* Exported constants --------------------------------------------------------*/


/* Exported macros -----------------------------------------------------------*/


/* Exported variables --------------------------------------------------------*/
extern COMP_HandleTypeDef hcomp1;

extern COMP_HandleTypeDef hcomp3;

/* Exported functions --------------------------------------------------------*/

void MX_COMP1_Init(void);
void MX_COMP3_Init(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BSP_COMP_H__ */


