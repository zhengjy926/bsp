/**
  ******************************************************************************
  * @file        : bsp_hrtim.c
  * @author      : ZJY
  * @version     : V1.0
  * @date        : 2025-10-16
  * @brief       : 
  * @attention   : None
  ******************************************************************************
  * @history     :
  *         V1.0 : 1.Initial version
  *
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "bsp_hrtim.h"
#include "bsp_conf.h"

/* Private typedef -----------------------------------------------------------*/


/* Private define ------------------------------------------------------------*/


/* Private macro -------------------------------------------------------------*/


/* Private variables ---------------------------------------------------------*/

/* Exported variables  -------------------------------------------------------*/


/* Private function prototypes -----------------------------------------------*/

/* Exported functions --------------------------------------------------------*/
void bsp_hrtim_init(void)
{

  /* USER CODE BEGIN HRTIM1_Init 0 */

  /* USER CODE END HRTIM1_Init 0 */

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* Peripheral clock enable */
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_HRTIM1);

  /* HRTIM1 DMA Init */

  /* HRTIM1_B Init */
//  LL_DMA_SetPeriphRequest(DMA1, LL_DMA_CHANNEL_1, LL_DMAMUX_REQ_HRTIM1_B);

//  LL_DMA_SetDataTransferDirection(DMA1, LL_DMA_CHANNEL_1, LL_DMA_DIRECTION_MEMORY_TO_PERIPH);

//  LL_DMA_SetChannelPriorityLevel(DMA1, LL_DMA_CHANNEL_1, LL_DMA_PRIORITY_LOW);

//  LL_DMA_SetMode(DMA1, LL_DMA_CHANNEL_1, LL_DMA_MODE_CIRCULAR);

//  LL_DMA_SetPeriphIncMode(DMA1, LL_DMA_CHANNEL_1, LL_DMA_PERIPH_NOINCREMENT);

//  LL_DMA_SetMemoryIncMode(DMA1, LL_DMA_CHANNEL_1, LL_DMA_MEMORY_INCREMENT);

//  LL_DMA_SetPeriphSize(DMA1, LL_DMA_CHANNEL_1, LL_DMA_PDATAALIGN_HALFWORD);

//  LL_DMA_SetMemorySize(DMA1, LL_DMA_CHANNEL_1, LL_DMA_MDATAALIGN_HALFWORD);

  /* HRTIM1 interrupt Init */
  NVIC_SetPriority(HRTIM1_TIMB_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
  NVIC_EnableIRQ(HRTIM1_TIMB_IRQn);

  /* USER CODE BEGIN HRTIM1_Init 1 */

  /* USER CODE END HRTIM1_Init 1 */
  LL_HRTIM_ConfigDLLCalibration(HRTIM1, LL_HRTIM_DLLCALIBRATION_MODE_CONTINUOUS, LL_HRTIM_DLLCALIBRATION_RATE_3);

  /* Poll for DLL end of calibration */
#if (USE_TIMEOUT == 1)
  uint32_t Timeout = 10; /* Timeout Initialization */
#endif  /*USE_TIMEOUT*/

  while(LL_HRTIM_IsActiveFlag_DLLRDY(HRTIM1) == RESET){
#if (USE_TIMEOUT == 1)
    if (LL_SYSTICK_IsActiveCounterFlag())  /* Check Systick counter flag to decrement the time-out value */
    {
        if(Timeout-- == 0)
        {
          Error_Handler();  /* error management */
        }
    }
#endif  /* USE_TIMEOUT */
  }

  LL_HRTIM_BM_SetMode(HRTIM1, LL_HRTIM_BM_MODE_SINGLESHOT);
  LL_HRTIM_BM_SetClockSrc(HRTIM1, LL_HRTIM_BM_CLKSRC_FHRTIM);
  LL_HRTIM_BM_SetTrig(HRTIM1, LL_HRTIM_BM_TRIG_TIMA_REPETITION);
  LL_HRTIM_BM_SetPeriod(HRTIM1, 0x0001);
  LL_HRTIM_BM_SetPrescaler(HRTIM1, LL_HRTIM_BM_PRESCALER_DIV512);
  LL_HRTIM_BM_SetCompare(HRTIM1, 0x0001);
  LL_HRTIM_BM_EnablePreload(HRTIM1);
  LL_HRTIM_TIM_SetPrescaler(HRTIM1, LL_HRTIM_TIMER_A, LL_HRTIM_PRESCALERRATIO_MUL4);
  LL_HRTIM_TIM_SetCounterMode(HRTIM1, LL_HRTIM_TIMER_A, LL_HRTIM_MODE_CONTINUOUS);
  LL_HRTIM_TIM_SetPeriod(HRTIM1, LL_HRTIM_TIMER_A, 0xFFFB);
  LL_HRTIM_TIM_SetRepetition(HRTIM1, LL_HRTIM_TIMER_A, 0x00);
  LL_HRTIM_TIM_SetUpdateGating(HRTIM1, LL_HRTIM_TIMER_A, LL_HRTIM_UPDATEGATING_DMABURST);
  LL_HRTIM_TIM_SetCountingMode(HRTIM1, LL_HRTIM_TIMER_A, LL_HRTIM_COUNTING_MODE_UP);
  LL_HRTIM_TIM_SetTriggeredHalfMode(HRTIM1, LL_HRTIM_TIMER_A, LL_HRTIM_TRIGHALF_DISABLED);
  LL_HRTIM_TIM_SetComp1Mode(HRTIM1, LL_HRTIM_TIMER_A, LL_HRTIM_GTCMP1_EQUAL);
  LL_HRTIM_TIM_SetComp3Mode(HRTIM1, LL_HRTIM_TIMER_A, LL_HRTIM_GTCMP3_EQUAL);
  LL_HRTIM_TIM_SetDACTrig(HRTIM1, LL_HRTIM_TIMER_A, LL_HRTIM_DACTRIG_NONE);
  LL_HRTIM_TIM_DisableHalfMode(HRTIM1, LL_HRTIM_TIMER_A);
  LL_HRTIM_TIM_SetInterleavedMode(HRTIM1, LL_HRTIM_TIMER_A, LL_HRTIM_INTERLEAVED_MODE_DISABLED);
  LL_HRTIM_TIM_DisableStartOnSync(HRTIM1, LL_HRTIM_TIMER_A);
  LL_HRTIM_TIM_DisableResetOnSync(HRTIM1, LL_HRTIM_TIMER_A);
  LL_HRTIM_TIM_EnablePreload(HRTIM1, LL_HRTIM_TIMER_A);
  LL_HRTIM_TIM_SetUpdateTrig(HRTIM1, LL_HRTIM_TIMER_A, LL_HRTIM_UPDATETRIG_NONE|LL_HRTIM_UPDATETRIG_NONE|LL_HRTIM_UPDATETRIG_RESET);
  LL_HRTIM_TIM_SetResetTrig(HRTIM1, LL_HRTIM_TIMER_A, LL_HRTIM_RESETTRIG_NONE);
  LL_HRTIM_TIM_DisablePushPullMode(HRTIM1, LL_HRTIM_TIMER_A);
  LL_HRTIM_TIM_DisableDeadTime(HRTIM1, LL_HRTIM_TIMER_A);
  LL_HRTIM_TIM_SetBurstModeOption(HRTIM1, LL_HRTIM_TIMER_A, LL_HRTIM_BURSTMODE_RESETCOUNTER);
  LL_HRTIM_ForceUpdate(HRTIM1, LL_HRTIM_TIMER_A);
  LL_HRTIM_TIM_SetCompare1(HRTIM1, LL_HRTIM_TIMER_A, 0xFFFB);
  LL_HRTIM_TIM_SetCompareMode(HRTIM1, LL_HRTIM_TIMER_A, LL_HRTIM_COMPAREUNIT_2, LL_HRTIM_COMPAREMODE_REGULAR);
  LL_HRTIM_TIM_SetCompare2(HRTIM1, LL_HRTIM_TIMER_A, 0xFFFB);
  LL_HRTIM_TIM_SetCompare3(HRTIM1, LL_HRTIM_TIMER_A, 0xFFFB);
  LL_HRTIM_TIM_ConfigBurstDMA(HRTIM1, LL_HRTIM_TIMER_A, LL_HRTIM_BURSTDMA_MPER|LL_HRTIM_BURSTDMA_MREP
                              |LL_HRTIM_BURSTDMA_MCMP1|LL_HRTIM_BURSTDMA_MCMP2
                              |LL_HRTIM_BURSTDMA_MCMP3);
  LL_HRTIM_OUT_SetPolarity(HRTIM1, LL_HRTIM_OUTPUT_TA1, LL_HRTIM_OUT_POSITIVE_POLARITY);
  LL_HRTIM_OUT_SetOutputSetSrc(HRTIM1, LL_HRTIM_OUTPUT_TA1, LL_HRTIM_OUTPUTSET_TIMCMP1);
  LL_HRTIM_OUT_SetOutputResetSrc(HRTIM1, LL_HRTIM_OUTPUT_TA1, LL_HRTIM_OUTPUTRESET_TIMCMP2|LL_HRTIM_OUTPUTRESET_TIMPER);
  LL_HRTIM_OUT_SetIdleMode(HRTIM1, LL_HRTIM_OUTPUT_TA1, LL_HRTIM_OUT_IDLE_WHEN_BURST);
  LL_HRTIM_OUT_SetIdleLevel(HRTIM1, LL_HRTIM_OUTPUT_TA1, LL_HRTIM_OUT_IDLELEVEL_INACTIVE);
  LL_HRTIM_OUT_SetFaultState(HRTIM1, LL_HRTIM_OUTPUT_TA1, LL_HRTIM_OUT_FAULTSTATE_NO_ACTION);
  LL_HRTIM_OUT_SetChopperMode(HRTIM1, LL_HRTIM_OUTPUT_TA1, LL_HRTIM_OUT_CHOPPERMODE_DISABLED);
  LL_HRTIM_OUT_SetPolarity(HRTIM1, LL_HRTIM_OUTPUT_TA2, LL_HRTIM_OUT_POSITIVE_POLARITY);
  LL_HRTIM_OUT_SetOutputSetSrc(HRTIM1, LL_HRTIM_OUTPUT_TA2, LL_HRTIM_OUTPUTSET_TIMCMP3);
  LL_HRTIM_OUT_SetOutputResetSrc(HRTIM1, LL_HRTIM_OUTPUT_TA2, LL_HRTIM_OUTPUTRESET_TIMPER);
  LL_HRTIM_OUT_SetIdleMode(HRTIM1, LL_HRTIM_OUTPUT_TA2, LL_HRTIM_OUT_IDLE_WHEN_BURST);
  LL_HRTIM_OUT_SetIdleLevel(HRTIM1, LL_HRTIM_OUTPUT_TA2, LL_HRTIM_OUT_IDLELEVEL_INACTIVE);
  LL_HRTIM_OUT_SetFaultState(HRTIM1, LL_HRTIM_OUTPUT_TA2, LL_HRTIM_OUT_FAULTSTATE_NO_ACTION);
  LL_HRTIM_OUT_SetChopperMode(HRTIM1, LL_HRTIM_OUTPUT_TA2, LL_HRTIM_OUT_CHOPPERMODE_DISABLED);

  /* Poll for DLL end of calibration */
#if (USE_TIMEOUT == 1)
  uint32_t Timeout = 10; /* Timeout Initialization */
#endif  /*USE_TIMEOUT*/

  while(LL_HRTIM_IsActiveFlag_DLLRDY(HRTIM1) == RESET){
#if (USE_TIMEOUT == 1)
    if (LL_SYSTICK_IsActiveCounterFlag())  /* Check Systick counter flag to decrement the time-out value */
    {
        if(Timeout-- == 0)
        {
          Error_Handler();  /* error management */
        }
    }
#endif  /* USE_TIMEOUT */
  }

  LL_HRTIM_TIM_SetPrescaler(HRTIM1, LL_HRTIM_TIMER_B, LL_HRTIM_PRESCALERRATIO_MUL4);
  LL_HRTIM_TIM_SetCounterMode(HRTIM1, LL_HRTIM_TIMER_B, LL_HRTIM_MODE_CONTINUOUS);
  LL_HRTIM_TIM_SetPeriod(HRTIM1, LL_HRTIM_TIMER_B, 0xFFFB);
  LL_HRTIM_TIM_SetRepetition(HRTIM1, LL_HRTIM_TIMER_B, 0x00);
  LL_HRTIM_TIM_SetUpdateGating(HRTIM1, LL_HRTIM_TIMER_B, LL_HRTIM_UPDATEGATING_DMABURST);
  LL_HRTIM_TIM_SetCountingMode(HRTIM1, LL_HRTIM_TIMER_B, LL_HRTIM_COUNTING_MODE_UP);
  LL_HRTIM_TIM_SetDACTrig(HRTIM1, LL_HRTIM_TIMER_B, LL_HRTIM_DACTRIG_NONE);
  LL_HRTIM_TIM_DisableHalfMode(HRTIM1, LL_HRTIM_TIMER_B);
  LL_HRTIM_TIM_SetInterleavedMode(HRTIM1, LL_HRTIM_TIMER_B, LL_HRTIM_INTERLEAVED_MODE_DISABLED);
  LL_HRTIM_TIM_DisableStartOnSync(HRTIM1, LL_HRTIM_TIMER_B);
  LL_HRTIM_TIM_DisableResetOnSync(HRTIM1, LL_HRTIM_TIMER_B);
  LL_HRTIM_TIM_EnablePreload(HRTIM1, LL_HRTIM_TIMER_B);
  LL_HRTIM_TIM_SetUpdateTrig(HRTIM1, LL_HRTIM_TIMER_B, LL_HRTIM_UPDATETRIG_NONE|LL_HRTIM_UPDATETRIG_NONE|LL_HRTIM_UPDATETRIG_RESET);
  LL_HRTIM_TIM_SetResetTrig(HRTIM1, LL_HRTIM_TIMER_B, LL_HRTIM_RESETTRIG_NONE);
  LL_HRTIM_TIM_DisablePushPullMode(HRTIM1, LL_HRTIM_TIMER_B);
  LL_HRTIM_TIM_DisableDeadTime(HRTIM1, LL_HRTIM_TIMER_B);
  LL_HRTIM_TIM_SetBurstModeOption(HRTIM1, LL_HRTIM_TIMER_B, LL_HRTIM_BURSTMODE_RESETCOUNTER);
  LL_HRTIM_ForceUpdate(HRTIM1, LL_HRTIM_TIMER_B);
  LL_HRTIM_TIM_ConfigBurstDMA(HRTIM1, LL_HRTIM_TIMER_B, LL_HRTIM_BURSTDMA_MPER|LL_HRTIM_BURSTDMA_MREP);
  /* USER CODE BEGIN HRTIM1_Init 2 */
    
    LL_HRTIM_ClearFlag_REP(HRTIM1, LL_HRTIM_TIMER_B);
    LL_HRTIM_EnableIT_REP(HRTIM1, LL_HRTIM_TIMER_B);
    
  /* USER CODE END HRTIM1_Init 2 */
  LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOA);
    /**HRTIM1 GPIO Configuration
    PA8     ------> HRTIM1_CHA1
    PA9     ------> HRTIM1_CHA2
    */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_8;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_13;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LL_GPIO_PIN_9;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_13;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

}

uint16_t tim_arr_from_ms(uint32_t ms)
{
    /* ticks = round(ms * f / 1000)  —— 用 +500 实现对 /1000 的四舍五入 */
    uint32_t ticks = (ms * HRTIM_BUSTMODE_FREQ_HZ + 500u) / 1000u;

    /* 保护：至少 1 个计数，且不超过 65536（因为 ARR = ticks-1） */
    if (ticks == 0u)
        ticks = 1u;
    
    if (ticks > (65535 + 1u))
        ticks = 65535 + 1u;

    return (uint16_t)(ticks - 1u);
}

/* Private functions ---------------------------------------------------------*/
