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
#ifndef __STM32_ADC_H__
#define __STM32_ADC_H__

#ifdef __cplusplus
 extern "C" {
#endif /* __cplusplus */

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
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

