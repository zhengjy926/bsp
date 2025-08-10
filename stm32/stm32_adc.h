/**
  ******************************************************************************
  * @file        : stm32_adc.h
  * @author      : ZJY
  * @version     : V1.0
  * @date        : 2024-09-26
  * @brief       : 
  * @attention   : None
  ******************************************************************************
  * @history     :
  *         V1.0 : 1.xxx
  *
  *
  ******************************************************************************
  */
#ifndef __STM32_ADC_H__
#define __STM32_ADC_H__

#ifdef __cplusplus
 extern "C" {
#endif /* __cplusplus */

/* Includes ------------------------------------------------------------------*/
#include "sys_def.h"

/* Exported define -----------------------------------------------------------*/
#define ADC1_USE_CHANNEL_NUM        (4)
#define ADC1_CHANNEL_BUFF_SIZE      (1)

/* Exported typedef ----------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported variable prototypes ----------------------------------------------*/
extern uint16_t adc_dma_buf[ADC1_USE_CHANNEL_NUM][ADC1_CHANNEL_BUFF_SIZE];

/* Exported function prototypes ----------------------------------------------*/
void stm32_adc1_init(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __STM32_ADC_H__ */

