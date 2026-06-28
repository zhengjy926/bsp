/**
  ******************************************************************************
  * @copyright   : Copyright To Hangzhou Dinova EP Technology Co.,Ltd
  * @file        : bsp_adc.c
  * @author      : ZJY
  * @version     : V1.0
  * @data        : 2025-10-16
  * @brief       : STM32 ADC driver implementation
  * @attention   : None
  ******************************************************************************
  * @history     :
  *         V1.0 : 1.Initial version
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "bsp_adc.h"
#include "main.h"
#include "errno-base.h"

#define  LOG_TAG             "bsp_adc"
#define  LOG_LVL             ELOG_LVL_DEBUG
#include "elog.h"

/* Private typedef -----------------------------------------------------------*/
struct stm32_adc {
    ADC_HandleTypeDef hadc;
    DMA_HandleTypeDef hdma_adc;
    char *name;
    uint8_t *dma_buf;
    uint16_t dma_bufsz;
    uint8_t index;
};

enum
{
#ifdef BSP_USING_ADC1
    ADC1_INDEX,
#endif

#ifdef BSP_USING_ADC2
    ADC2_INDEX,
#endif

#ifdef BSP_USING_ADC3
    ADC3_INDEX,
#endif

#ifdef BSP_USING_ADC4
    ADC4_INDEX,
#endif

#ifdef BSP_USING_ADC5
    ADC5_INDEX,
#endif
    UART_INDEX_MAX,
};

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

