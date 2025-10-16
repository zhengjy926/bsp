/**
  ******************************************************************************
  * @file        : bsp_adc.h
  * @author      : ZJY
  * @version     : V1.0
  * @date        : 2025-10-16
  * @brief       : STM32 ADC driver header file
  * @attention   : None
  ******************************************************************************
  * @history     :
  *         V1.0 : 1.Initial version
  *
  *
  ******************************************************************************
  */
#ifndef __BSP_ADC_H__
#define __BSP_ADC_H__

#ifdef __cplusplus
 extern "C" {
#endif /* __cplusplus */

/* Includes ------------------------------------------------------------------*/
#include "bsp_conf.h"

/* Exported define -----------------------------------------------------------*/

/* Exported typedef ----------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported variable prototypes ----------------------------------------------*/
extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;

/* Exported function prototypes ----------------------------------------------*/
void bsp_adc1_init(void);
void bsp_adc2_init(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BSP_ADC_H__ */

