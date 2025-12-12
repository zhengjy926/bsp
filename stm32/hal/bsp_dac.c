/**
  ******************************************************************************
  * @file        : xxxx.c
  * @author      : ZJY
  * @version     : V1.0
  * @date        : 20xx-xx-xx
  * @brief       : 
  * @attention   : None
  ******************************************************************************
  * @history     :
  *         V1.0 : 1.xxx
  *
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "bsp_dac.h"
#include "board.h"

#define  LOG_TAG             "bsp_dac"
#define  LOG_LVL             4
#include "log.h"

/* Private typedef -----------------------------------------------------------*/


/* Private define ------------------------------------------------------------*/


/* Private macro -------------------------------------------------------------*/


/* Private variables ---------------------------------------------------------*/
DAC_HandleTypeDef hdac1;
DAC_HandleTypeDef hdac3;

/* Exported variables  -------------------------------------------------------*/


/* Private function prototypes -----------------------------------------------*/


/* Exported functions --------------------------------------------------------*/
void MX_DAC1_Init(void)
{
    DAC_ChannelConfTypeDef sConfig = {0};

    /** DAC Initialization
    */
    hdac1.Instance = DAC1;
    if (HAL_DAC_Init(&hdac1) != HAL_OK)
    {
        Error_Handler();
    }

    /** DAC channel OUT1 config
    */
    sConfig.DAC_HighFrequency = DAC_HIGH_FREQUENCY_INTERFACE_MODE_AUTOMATIC;
    sConfig.DAC_DMADoubleDataMode = DISABLE;
    sConfig.DAC_SignedFormat = DISABLE;
    sConfig.DAC_SampleAndHold = DAC_SAMPLEANDHOLD_DISABLE;
    sConfig.DAC_Trigger = DAC_TRIGGER_NONE;
    sConfig.DAC_Trigger2 = DAC_TRIGGER_NONE;
    sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
    sConfig.DAC_ConnectOnChipPeripheral = DAC_CHIPCONNECT_BOTH;
    sConfig.DAC_UserTrimming = DAC_TRIMMING_FACTORY;
    if (HAL_DAC_ConfigChannel(&hdac1, &sConfig, DAC_CHANNEL_1) != HAL_OK)
    {
        Error_Handler();
    }

    HAL_DACEx_SelfCalibrate(&hdac1, &sConfig, DAC_CHANNEL_1);
}

/* DAC3 init function */
void MX_DAC3_Init(void)
{
    DAC_ChannelConfTypeDef sConfig = {0};

    /** DAC Initialization
    */
    hdac3.Instance = DAC3;
    if (HAL_DAC_Init(&hdac3) != HAL_OK)
    {
        Error_Handler();
    }

    /** DAC channel OUT1 config
    */
    sConfig.DAC_HighFrequency = DAC_HIGH_FREQUENCY_INTERFACE_MODE_AUTOMATIC;
    sConfig.DAC_DMADoubleDataMode = DISABLE;
    sConfig.DAC_SignedFormat = DISABLE;
    sConfig.DAC_SampleAndHold = DAC_SAMPLEANDHOLD_DISABLE;
    sConfig.DAC_Trigger = DAC_TRIGGER_NONE;
    sConfig.DAC_Trigger2 = DAC_TRIGGER_NONE;
    sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_DISABLE;
    sConfig.DAC_ConnectOnChipPeripheral = DAC_CHIPCONNECT_INTERNAL;
    sConfig.DAC_UserTrimming = DAC_TRIMMING_FACTORY;
    if (HAL_DAC_ConfigChannel(&hdac3, &sConfig, DAC_CHANNEL_1) != HAL_OK)
    {
        Error_Handler();
    }

    HAL_DACEx_SelfCalibrate(&hdac3, &sConfig, DAC_CHANNEL_1);
}

void HAL_DAC_MspInit(DAC_HandleTypeDef* dacHandle)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    if(dacHandle->Instance == DAC1)
    {
        /* DAC1 clock enable */
        __HAL_RCC_DAC1_CLK_ENABLE();

        __HAL_RCC_GPIOA_CLK_ENABLE();
        /**DAC1 GPIO Configuration
        PA4     ------> DAC1_OUT1
        */
        GPIO_InitStruct.Pin = GPIO_PIN_4;
        GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    }
    else if(dacHandle->Instance == DAC3)
    {
        /* DAC3 clock enable */
        __HAL_RCC_DAC3_CLK_ENABLE();
    }
}

void HAL_DAC_MspDeInit(DAC_HandleTypeDef* dacHandle)
{
    if(dacHandle->Instance == DAC1)
    {
        /* Peripheral clock disable */
        __HAL_RCC_DAC1_CLK_DISABLE();
    }
    else if(dacHandle->Instance == DAC3)
    {
        /* Peripheral clock disable */
        __HAL_RCC_DAC3_CLK_DISABLE();
    }
}


/* Private functions ---------------------------------------------------------*/




