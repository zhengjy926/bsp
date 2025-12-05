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
#include "bsp_comp.h"
#include "board.h"

/* Private typedef -----------------------------------------------------------*/


/* Private define ------------------------------------------------------------*/


/* Private macro -------------------------------------------------------------*/


/* Private variables ---------------------------------------------------------*/
COMP_HandleTypeDef hcomp1;
COMP_HandleTypeDef hcomp3;

/* Exported variables  -------------------------------------------------------*/


/* Private function prototypes -----------------------------------------------*/


/* Exported functions --------------------------------------------------------*/
void MX_COMP1_Init(void)
{
    hcomp1.Instance = COMP1;
    hcomp1.Init.InputPlus = COMP_INPUT_PLUS_IO1;
    hcomp1.Init.InputMinus = COMP_INPUT_MINUS_DAC3_CH1;
    hcomp1.Init.OutputPol = COMP_OUTPUTPOL_NONINVERTED;
    hcomp1.Init.Hysteresis = COMP_HYSTERESIS_70MV;
    hcomp1.Init.BlankingSrce = COMP_BLANKINGSRC_NONE;
    hcomp1.Init.TriggerMode = COMP_TRIGGERMODE_IT_RISING;
    if (HAL_COMP_Init(&hcomp1) != HAL_OK)
    {
        Error_Handler();
    }
}

void MX_COMP3_Init(void)
{
    hcomp3.Instance = COMP3;
    hcomp3.Init.InputPlus = COMP_INPUT_PLUS_IO1;
    hcomp3.Init.InputMinus = COMP_INPUT_MINUS_DAC1_CH1;
    hcomp3.Init.OutputPol = COMP_OUTPUTPOL_NONINVERTED;
    hcomp3.Init.Hysteresis = COMP_HYSTERESIS_70MV;
    hcomp3.Init.BlankingSrce = COMP_BLANKINGSRC_NONE;
    hcomp3.Init.TriggerMode = COMP_TRIGGERMODE_IT_RISING;
    if (HAL_COMP_Init(&hcomp3) != HAL_OK)
    {
        Error_Handler();
    }
}

void HAL_COMP_MspInit(COMP_HandleTypeDef* compHandle)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    if(compHandle->Instance==COMP1)
    {
        __HAL_RCC_GPIOA_CLK_ENABLE();
        /**COMP1 GPIO Configuration
        PA1     ------> COMP1_INP
        */
        GPIO_InitStruct.Pin = GPIO_PIN_1;
        GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        /* COMP1 interrupt Init */
        HAL_NVIC_SetPriority(COMP1_2_3_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(COMP1_2_3_IRQn);
    }
    else if(compHandle->Instance==COMP3)
    {
        __HAL_RCC_GPIOA_CLK_ENABLE();
        /**COMP3 GPIO Configuration
        PA0     ------> COMP3_INP
        */
        GPIO_InitStruct.Pin = GPIO_PIN_0;
        GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        /* COMP3 interrupt Init */
        HAL_NVIC_SetPriority(COMP1_2_3_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(COMP1_2_3_IRQn);
    }
}

void HAL_COMP_MspDeInit(COMP_HandleTypeDef* compHandle)
{

    if(compHandle->Instance==COMP1)
    {
        /**COMP1 GPIO Configuration
        PA1     ------> COMP1_INP
        */
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_1);
    }
    else if(compHandle->Instance==COMP3)
    {
        /**COMP3 GPIO Configuration
        PA0     ------> COMP3_INP
        */
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_0);
    }
}

/**
  * @brief This function handles COMP1, COMP2 and COMP3 interrupts through EXTI lines 21, 22 and 29.
  */
void COMP1_2_3_IRQHandler(void)
{
    HAL_COMP_IRQHandler(&hcomp1);
    HAL_COMP_IRQHandler(&hcomp3);
}

/* Private functions ---------------------------------------------------------*/


