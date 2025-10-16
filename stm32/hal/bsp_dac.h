/**
  ******************************************************************************
  * @file        : xxx.h
  * @author      : ZJY
  * @version     : V1.0
  * @date        : 20xx-xx-xx
  * @brief       : 
  * @attention   : None
  ******************************************************************************
  * @history     :
  *         V1.0 : 1.xxx
  ******************************************************************************
  */
#ifndef __BSP_DAC_H__
#define __BSP_DAC_H__

#ifdef __cplusplus
 extern "C" {
#endif /* __cplusplus */

/* Includes ------------------------------------------------------------------*/
#include "bsp_conf.h"

/* Exported types ------------------------------------------------------------*/


/* Exported constants --------------------------------------------------------*/


/* Exported macros -----------------------------------------------------------*/


/* Exported variables --------------------------------------------------------*/

extern DAC_HandleTypeDef hdac1;

extern DAC_HandleTypeDef hdac3;

/* Exported functions --------------------------------------------------------*/

void MX_DAC1_Init(void);
void MX_DAC3_Init(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BSP_DAC_H__ */


