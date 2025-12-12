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
#include "board.h"
#include "errno-base.h"

#define  LOG_TAG             "bsp_hrtim"
#define  LOG_LVL             4
#include "log.h"

/* Private typedef -----------------------------------------------------------*/


/* Private define ------------------------------------------------------------*/


/* Private macro -------------------------------------------------------------*/


/* Private variables ---------------------------------------------------------*/

/* Exported variables  -------------------------------------------------------*/


/* Private function prototypes -----------------------------------------------*/
static void HRTIM_Fault_Configuration(void);

/* Exported functions --------------------------------------------------------*/
int bsp_hrtim_init(void)
{
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    /* Peripheral clock enable */
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_HRTIM1);
    
    /* 校准 */
    LL_HRTIM_ConfigDLLCalibration(HRTIM1, 
                                  LL_HRTIM_DLLCALIBRATION_MODE_CONTINUOUS,
                                  LL_HRTIM_DLLCALIBRATION_RATE_3);
    /* Poll for DLL end of calibration */
    uint32_t Timeout = 10; /* Timeout Initialization */
    while(LL_HRTIM_IsActiveFlag_DLLRDY(HRTIM1) == RESET)
    {
        if (LL_SYSTICK_IsActiveCounterFlag()) {  /* Check Systick counter flag to decrement the time-out value */
            if(Timeout-- == 0) {
                LOG_W("HRTIM calibration timeout!");
                return -EIO;
            }
        }
    }

    /* HRTIM1 interrupt Init */
    NVIC_SetPriority(HRTIM1_TIMB_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
    NVIC_EnableIRQ(HRTIM1_TIMB_IRQn);
    
    /* 故障保护配置 */
    HRTIM_Fault_Configuration();
    
    /* 突发模式配置 */
    LL_HRTIM_BM_SetMode(HRTIM1, LL_HRTIM_BM_MODE_SINGLESHOT);
    LL_HRTIM_BM_SetClockSrc(HRTIM1, LL_HRTIM_BM_CLKSRC_FHRTIM);
    LL_HRTIM_BM_SetTrig(HRTIM1, LL_HRTIM_BM_TRIG_TIMA_REPETITION);
    LL_HRTIM_BM_SetPeriod(HRTIM1, 0x0001);
    LL_HRTIM_BM_SetPrescaler(HRTIM1, LL_HRTIM_BM_PRESCALER_DIV512);
    LL_HRTIM_BM_SetCompare(HRTIM1, 0x0001);
    LL_HRTIM_BM_EnablePreload(HRTIM1);

    /* Base setting */
    LL_HRTIM_TIM_SetPeriod(HRTIM1, LL_HRTIM_TIMER_A, 0xFFFB);
    LL_HRTIM_TIM_SetRepetition(HRTIM1, LL_HRTIM_TIMER_A, 0x00);
    LL_HRTIM_TIM_SetPrescaler(HRTIM1, LL_HRTIM_TIMER_A, LL_HRTIM_PRESCALERRATIO_MUL4);
    LL_HRTIM_TIM_SetCounterMode(HRTIM1, LL_HRTIM_TIMER_A, LL_HRTIM_MODE_CONTINUOUS);
    LL_HRTIM_TIM_SetCountingMode(HRTIM1, LL_HRTIM_TIMER_A, LL_HRTIM_COUNTING_MODE_UP);
    
    LL_HRTIM_TIM_SetUpdateGating(HRTIM1, LL_HRTIM_TIMER_A, LL_HRTIM_UPDATEGATING_DMABURST);
    LL_HRTIM_TIM_SetComp1Mode(HRTIM1, LL_HRTIM_TIMER_A, LL_HRTIM_GTCMP1_EQUAL);
    LL_HRTIM_TIM_SetComp3Mode(HRTIM1, LL_HRTIM_TIMER_A, LL_HRTIM_GTCMP3_EQUAL);
    LL_HRTIM_TIM_EnablePreload(HRTIM1, LL_HRTIM_TIMER_A);
    LL_HRTIM_TIM_SetUpdateTrig(HRTIM1, LL_HRTIM_TIMER_A, LL_HRTIM_UPDATETRIG_NONE|LL_HRTIM_UPDATETRIG_NONE|LL_HRTIM_UPDATETRIG_RESET);
    LL_HRTIM_TIM_SetResetTrig(HRTIM1, LL_HRTIM_TIMER_A, LL_HRTIM_RESETTRIG_NONE);
    LL_HRTIM_TIM_DisablePushPullMode(HRTIM1, LL_HRTIM_TIMER_A);
    LL_HRTIM_TIM_DisableDeadTime(HRTIM1, LL_HRTIM_TIMER_A);
    LL_HRTIM_TIM_SetBurstModeOption(HRTIM1, LL_HRTIM_TIMER_A, LL_HRTIM_BURSTMODE_RESETCOUNTER);
    
    /* ============================================================ */
    /* 将故障应用到具体的定时器输出 (以 Timer A 为例)               */
    /* ============================================================ */
    /* 重要：仅仅使能 Fault 通道是不够的，必须告诉 Timer A 也要受其控制 */
    /* 启用 Timer A 的 Fault 4 和 Fault 5 功能 */
    LL_HRTIM_TIM_EnableFault(HRTIM1, LL_HRTIM_TIMER_A,
                             LL_HRTIM_FAULT_4 | LL_HRTIM_FAULT_5);                      
    LL_HRTIM_ForceUpdate(HRTIM1, LL_HRTIM_TIMER_A);
    

    LL_HRTIM_TIM_SetCompare1(HRTIM1, LL_HRTIM_TIMER_A, 0xFFFB);
    LL_HRTIM_TIM_SetCompareMode(HRTIM1, LL_HRTIM_TIMER_A, LL_HRTIM_COMPAREUNIT_2, LL_HRTIM_COMPAREMODE_REGULAR);
    LL_HRTIM_TIM_SetCompare2(HRTIM1, LL_HRTIM_TIMER_A, 0xFFFB);
    LL_HRTIM_TIM_SetCompare3(HRTIM1, LL_HRTIM_TIMER_A, 0xFFFB);

    LL_HRTIM_OUT_SetPolarity(HRTIM1, LL_HRTIM_OUTPUT_TA1, LL_HRTIM_OUT_POSITIVE_POLARITY);
    LL_HRTIM_OUT_SetOutputSetSrc(HRTIM1, LL_HRTIM_OUTPUT_TA1, LL_HRTIM_OUTPUTSET_TIMCMP1);
    LL_HRTIM_OUT_SetOutputResetSrc(HRTIM1, LL_HRTIM_OUTPUT_TA1, LL_HRTIM_OUTPUTRESET_TIMCMP2|LL_HRTIM_OUTPUTRESET_TIMPER);
    LL_HRTIM_OUT_SetIdleMode(HRTIM1, LL_HRTIM_OUTPUT_TA1, LL_HRTIM_OUT_IDLE_WHEN_BURST);
    LL_HRTIM_OUT_SetIdleLevel(HRTIM1, LL_HRTIM_OUTPUT_TA1, LL_HRTIM_OUT_IDLELEVEL_INACTIVE);
    /* 配置故障时的输出电平状态 */
    LL_HRTIM_OUT_SetFaultState(HRTIM1, LL_HRTIM_OUTPUT_TA1, LL_HRTIM_OUT_FAULTSTATE_INACTIVE);
    LL_HRTIM_OUT_SetChopperMode(HRTIM1, LL_HRTIM_OUTPUT_TA1, LL_HRTIM_OUT_CHOPPERMODE_DISABLED);
    
    LL_HRTIM_OUT_SetPolarity(HRTIM1, LL_HRTIM_OUTPUT_TA2, LL_HRTIM_OUT_POSITIVE_POLARITY);
    LL_HRTIM_OUT_SetOutputSetSrc(HRTIM1, LL_HRTIM_OUTPUT_TA2, LL_HRTIM_OUTPUTSET_TIMCMP3);
    LL_HRTIM_OUT_SetOutputResetSrc(HRTIM1, LL_HRTIM_OUTPUT_TA2, LL_HRTIM_OUTPUTRESET_TIMPER);
    LL_HRTIM_OUT_SetIdleMode(HRTIM1, LL_HRTIM_OUTPUT_TA2, LL_HRTIM_OUT_IDLE_WHEN_BURST);
    LL_HRTIM_OUT_SetIdleLevel(HRTIM1, LL_HRTIM_OUTPUT_TA2, LL_HRTIM_OUT_IDLELEVEL_INACTIVE);
    LL_HRTIM_OUT_SetFaultState(HRTIM1, LL_HRTIM_OUTPUT_TA2, LL_HRTIM_OUT_FAULTSTATE_INACTIVE);
    LL_HRTIM_OUT_SetChopperMode(HRTIM1, LL_HRTIM_OUTPUT_TA2, LL_HRTIM_OUT_CHOPPERMODE_DISABLED);

    /* ============================================================ */
    /*  Timer B 配置                                                */
    /* ============================================================ */
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

    LL_HRTIM_ClearFlag_REP(HRTIM1, LL_HRTIM_TIMER_B);
    LL_HRTIM_EnableIT_REP(HRTIM1, LL_HRTIM_TIMER_B);
    
    /* 清除标志 */
    LL_HRTIM_ClearFlag_FLT4(HRTIM1);
    LL_HRTIM_ClearFlag_FLT5(HRTIM1);

    /* 使能 HRTIM 故障中断源  */
    LL_HRTIM_EnableIT_FLT4(HRTIM1);
    LL_HRTIM_EnableIT_FLT5(HRTIM1);

    /* 配置 NVIC (嵌套向量中断控制器) */
    NVIC_SetPriority(HRTIM1_FLT_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));
    NVIC_EnableIRQ(HRTIM1_FLT_IRQn);
    
    /* ============================================================ */
    /*  GPIO 配置                                                   */
    /* ============================================================ */
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
    
    return 0;
}
/**
  * @brief  配置 HRTIM 故障保护，使用 COMP1 和 COMP3 作为源
  * @note   需确保 COMP1 和 COMP3 已在其他地方被初始化并使能
  */
static void HRTIM_Fault_Configuration(void)
{
    /* ============================================================ */
    /* 配置 Fault 4 (对应 COMP1)                                     */
    /* ============================================================ */
    
    /* 设置故障源为内部 (Internal)，即连接到 COMP1 */
    LL_HRTIM_FLT_SetSrc(HRTIM1, LL_HRTIM_FAULT_4, LL_HRTIM_FLT_SRC_INTERNAL);
    
    /* 设置极性 */
    LL_HRTIM_FLT_SetPolarity(HRTIM1, LL_HRTIM_FAULT_4, LL_HRTIM_FLT_POLARITY_HIGH);
    
    /* 设置滤波 */
    LL_HRTIM_FLT_SetFilter(HRTIM1, LL_HRTIM_FAULT_4, LL_HRTIM_FLT_FILTER_NONE);
    
    /* 使能 Fault 4 通道 */
    LL_HRTIM_FLT_Enable(HRTIM1, LL_HRTIM_FAULT_4);


    /* ============================================================ */
    /* 配置 Fault 5 (对应 COMP3)                                     */
    /* ============================================================ */
    LL_HRTIM_FLT_SetSrc(HRTIM1, LL_HRTIM_FAULT_5, LL_HRTIM_FLT_SRC_INTERNAL);
    LL_HRTIM_FLT_SetPolarity(HRTIM1, LL_HRTIM_FAULT_5, LL_HRTIM_FLT_POLARITY_HIGH);
    LL_HRTIM_FLT_SetFilter(HRTIM1, LL_HRTIM_FAULT_5, LL_HRTIM_FLT_FILTER_NONE);
    LL_HRTIM_FLT_Enable(HRTIM1, LL_HRTIM_FAULT_5);
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
