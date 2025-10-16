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
#ifndef __STM32_DMA_H__
#define __STM32_DMA_H__

#ifdef __cplusplus
 extern "C" {
#endif /* __cplusplus */

/* Includes ------------------------------------------------------------------*/
#include "bsp_conf.h"

/* Exported define -----------------------------------------------------------*/
#if defined(SOC_SERIES_STM32F4)
    #define DMA_INSTANCE_TYPE   DMA_Stream_TypeDef
#else
    #define DMA_INSTANCE_TYPE   DMA_Channel_TypeDef   
#endif

struct dma_config {
    DMA_INSTANCE_TYPE *Instance;
    DMA_HandleTypeDef hdma;
    IRQn_Type dma_irq;

#if defined(STM32F429xx)
    uint32_t channel;
#elif defined(STM32G474xx)
    uint32_t request;
#endif
};
/* Exported typedef ----------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported variable prototypes ----------------------------------------------*/

/* Exported function prototypes ----------------------------------------------*/


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __STM32_DMA_H__ */

