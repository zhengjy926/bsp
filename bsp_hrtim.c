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
#include "errno-base.h"

#define  LOG_TAG             "bsp_hrtim"
#define  LOG_LVL             ELOG_LVL_DEBUG
#include "elog.h"

/* Private typedef -----------------------------------------------------------*/


/* Private define ------------------------------------------------------------*/
#define FAULT_THRESHOLD_NUM     (2U)

/* Private macro -------------------------------------------------------------*/


/* Private variables ---------------------------------------------------------*/
static HRTIM_HandleTypeDef hhrtim1;
static bsp_hrtim_rep_callback_t s_rep_callback;

/* Exported variables  -------------------------------------------------------*/


/* Private function prototypes -----------------------------------------------*/
static void HAL_HRTIM_MspPostInit(HRTIM_HandleTypeDef *hhrtim);
static int bsp_hrtim_common_init(void);
static int bsp_hrtim_fault_config(void);
static int bsp_hrtim_adc_trigger_config(void);
static int bsp_hrtim_timA_config(void);
static int bsp_hrtim_timB_config(void);

/* Exported functions --------------------------------------------------------*/
int bsp_hrtim_init(void)
{
    bsp_hrtim_common_init();
    bsp_hrtim_fault_config();
    bsp_hrtim_adc_trigger_config();
    bsp_hrtim_timA_config();
    bsp_hrtim_timB_config();
    HAL_HRTIM_MspPostInit(&hhrtim1);

    return 0;
}

static int bsp_hrtim_common_init(void)
{
    /* 配置整个 HRTIM 模块如何与外部世界（或其他外设）协同工作。 */
    hhrtim1.Instance = HRTIM1;
    hhrtim1.Init.HRTIMInterruptResquests = HRTIM_IT_FLT4 | HRTIM_IT_FLT5 | HRTIM_IT_SYSFLT;
    hhrtim1.Init.SyncOptions = HRTIM_SYNCOPTION_NONE;
    if (HAL_HRTIM_Init(&hhrtim1) != HAL_OK)
    {
        log_e("HAL_HRTIM_Init failed");
        return -ERR_IO;
    }
    
    /* 校准配置 */
    if (HAL_HRTIM_DLLCalibrationStart(&hhrtim1, HRTIM_CALIBRATIONRATE_3) != HAL_OK)
    {
        log_e("HAL_HRTIM_DLLCalibrationStart failed");
        return -ERR_IO;
    }
    if (HAL_HRTIM_PollForDLLCalibration(&hhrtim1, 10) != HAL_OK)
    {
        log_e("HAL_HRTIM_PollForDLLCalibration failed");
        return -ERR_IO;
    }
    
    return 0;
}

static int bsp_hrtim_timA_config(void)
{
    HRTIM_TimeBaseCfgTypeDef pTimeBaseCfg = {0};
    HRTIM_TimerCtlTypeDef pTimerCtl = {0};
    HRTIM_TimerCfgTypeDef pTimerCfg = {0};
    HRTIM_CompareCfgTypeDef pCompareCfg = {0};
    HRTIM_OutputCfgTypeDef pOutputCfg = {0};
    HAL_StatusTypeDef status = HAL_OK;
    
    /* 基础时基配置 */
    pTimeBaseCfg.Period            = 0xFFFF; /* 初始化时先设置为最大，后续再进行更改 */ // TODO: 验证这里从0xFFFD修改为0xFFFF是否可行
    pTimeBaseCfg.RepetitionCounter = 0;
    pTimeBaseCfg.PrescalerRatio    = HRTIM_PRESCALERRATIO_MUL2;
    pTimeBaseCfg.Mode              = HRTIM_MODE_CONTINUOUS;
    status = HAL_HRTIM_TimeBaseConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_A, &pTimeBaseCfg);
    if (status != HAL_OK)
    {
        log_e("HAL_HRTIM_TimeBaseConfig failed! Status = %d", status);
        return -ERR_IO;
    }
    
    /* 定时器控制配置 */
    pTimerCtl.UpDownMode           = HRTIM_TIMERUPDOWNMODE_UP;
    pTimerCtl.TrigHalf             = HRTIM_TIMERTRIGHALF_DISABLED;
    pTimerCtl.GreaterCMP3          = HRTIM_TIMERGTCMP3_EQUAL;
    pTimerCtl.GreaterCMP1          = HRTIM_TIMERGTCMP1_EQUAL;
    pTimerCtl.DualChannelDacEnable = HRTIM_TIMER_DCDE_DISABLED;
    pTimerCtl.DualChannelDacReset  = HRTIM_TIMER_DCDR_COUNTER;
    pTimerCtl.DualChannelDacStep   = HRTIM_TIMER_DCDS_CMP2;
    status = HAL_HRTIM_WaveformTimerControl(&hhrtim1, HRTIM_TIMERINDEX_TIMER_A, &pTimerCtl);
    if (status != HAL_OK)
    {
        log_e("HAL_HRTIM_WaveformTimerControl failed! Status = %d", status);
        return -ERR_IO;
    }
    
    pTimerCfg.InterruptRequests = HRTIM_TIM_IT_REP;
    pTimerCfg.DMARequests       = HRTIM_TIM_DMA_NONE;
    pTimerCfg.DMASrcAddress     = 0x0000;
    pTimerCfg.DMADstAddress     = 0x0000;
    pTimerCfg.DMASize           = 0x1;
    
    pTimerCfg.PreloadEnable    = HRTIM_PRELOAD_ENABLED;
    pTimerCfg.UpdateTrigger    = HRTIM_TIMUPDATETRIGGER_NONE;
    pTimerCfg.RepetitionUpdate = HRTIM_UPDATEONREPETITION_ENABLED;
    pTimerCfg.UpdateGating     = HRTIM_UPDATEGATING_INDEPENDENT;
    pTimerCfg.ResetUpdate      = HRTIM_TIMUPDATEONRESET_DISABLED;    // TODO: 需要验证这样是否可行
    pTimerCfg.ReSyncUpdate     = HRTIM_TIMERESYNC_UPDATE_UNCONDITIONAL;
    
    pTimerCfg.HalfModeEnable  = HRTIM_HALFMODE_DISABLED;
    pTimerCfg.InterleavedMode = HRTIM_INTERLEAVED_MODE_DISABLED;
    pTimerCfg.PushPull        = HRTIM_TIMPUSHPULLMODE_DISABLED;
    
    pTimerCfg.ResetTrigger = HRTIM_TIMRESETTRIGGER_NONE;
    pTimerCfg.StartOnSync  = HRTIM_SYNCSTART_DISABLED;
    pTimerCfg.ResetOnSync  = HRTIM_SYNCRESET_DISABLED;
    
    pTimerCfg.FaultEnable           = HRTIM_TIMFAULTENABLE_FAULT4 | HRTIM_TIMFAULTENABLE_FAULT5;
    pTimerCfg.FaultLock             = HRTIM_TIMFAULTLOCK_READWRITE;
    pTimerCfg.DeadTimeInsertion     = HRTIM_TIMDEADTIMEINSERTION_DISABLED;
    pTimerCfg.DelayedProtectionMode = HRTIM_TIMER_A_B_C_DELAYEDPROTECTION_DISABLED;
    
    pTimerCfg.DACSynchro                  = HRTIM_DACSYNC_NONE;
    pTimerCfg.BurstMode                   = HRTIM_TIMERBURSTMODE_MAINTAINCLOCK;
    pTimerCfg.BalancedIdleAutomaticResume = HRTIM_OUTPUTBIAR_DISABLED;
    if (HAL_HRTIM_WaveformTimerConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_A, &pTimerCfg) != HAL_OK)
    {
        log_e("HAL_HRTIM_WaveformTimerConfig failed");
        return -ERR_IO;
    }
    
    /* 比较器配置 */
    pCompareCfg.CompareValue = 0xFFFF;
    if (HAL_HRTIM_WaveformCompareConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_A, HRTIM_COMPAREUNIT_1, &pCompareCfg) != HAL_OK)
    {
        log_e("HAL_HRTIM_WaveformCompareConfig failed");
        return -ERR_IO;
    }
    
    pCompareCfg.CompareValue = 0xFFFF;
    pCompareCfg.AutoDelayedMode = HRTIM_AUTODELAYEDMODE_REGULAR;
    pCompareCfg.AutoDelayedTimeout = 0x0000;
    if (HAL_HRTIM_WaveformCompareConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_A, HRTIM_COMPAREUNIT_2, &pCompareCfg) != HAL_OK)
    {
        log_e("HAL_HRTIM_WaveformCompareConfig failed");
        return -ERR_IO;
    }
    
    pCompareCfg.CompareValue = 0xFFFF;
    if (HAL_HRTIM_WaveformCompareConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_A, HRTIM_COMPAREUNIT_3, &pCompareCfg) != HAL_OK)
    {
        log_e("HAL_HRTIM_WaveformCompareConfig failed");
        return -ERR_IO;
    }
    
    /* 输出配置 */
    pOutputCfg.Polarity = HRTIM_OUTPUTPOLARITY_HIGH;
    pOutputCfg.SetSource = HRTIM_OUTPUTSET_TIMCMP1;
    pOutputCfg.ResetSource = HRTIM_OUTPUTSET_TIMCMP2;
    pOutputCfg.IdleMode = HRTIM_OUTPUTIDLEMODE_NONE;        /* 配置输出是否受突发模式的影响 */
    pOutputCfg.IdleLevel = HRTIM_OUTPUTIDLELEVEL_INACTIVE;
    pOutputCfg.FaultLevel = HRTIM_OUTPUTFAULTLEVEL_INACTIVE;
    pOutputCfg.ChopperModeEnable = HRTIM_OUTPUTCHOPPERMODE_DISABLED;
    pOutputCfg.BurstModeEntryDelayed = HRTIM_OUTPUTBURSTMODEENTRY_REGULAR;
    if (HAL_HRTIM_WaveformOutputConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_A, HRTIM_OUTPUT_TA1, &pOutputCfg) != HAL_OK)
    {
        log_e("HAL_HRTIM_WaveformOutputConfig failed");
        return -ERR_IO;
    }
    
    pOutputCfg.Polarity = HRTIM_OUTPUTPOLARITY_HIGH;
    pOutputCfg.SetSource = HRTIM_OUTPUTSET_TIMCMP3;
    pOutputCfg.ResetSource = HRTIM_OUTPUTSET_TIMPER;
    pOutputCfg.IdleMode = HRTIM_OUTPUTIDLEMODE_NONE;        /* 配置输出是否受突发模式的影响 */
    pOutputCfg.IdleLevel = HRTIM_OUTPUTIDLELEVEL_INACTIVE;
    pOutputCfg.FaultLevel = HRTIM_OUTPUTFAULTLEVEL_INACTIVE;
    pOutputCfg.ChopperModeEnable = HRTIM_OUTPUTCHOPPERMODE_DISABLED;
    pOutputCfg.BurstModeEntryDelayed = HRTIM_OUTPUTBURSTMODEENTRY_REGULAR;
    if (HAL_HRTIM_WaveformOutputConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_A, HRTIM_OUTPUT_TA2, &pOutputCfg) != HAL_OK)
    {
        log_e("HAL_HRTIM_WaveformOutputConfig failed");
        return -ERR_IO;
    }
    
    return 0;
}

static int bsp_hrtim_timB_config(void)
{
    HRTIM_TimeBaseCfgTypeDef pTimeBaseCfg = {0};
    HRTIM_TimerCtlTypeDef pTimerCtl = {0};
    HRTIM_TimerCfgTypeDef pTimerCfg = {0};
    HRTIM_CompareCfgTypeDef pCompareCfg = {0};
    HAL_StatusTypeDef status = HAL_OK;

    pTimeBaseCfg.Period            = 0xFFFF;
    pTimeBaseCfg.RepetitionCounter = 0;
    pTimeBaseCfg.PrescalerRatio    = HRTIM_PRESCALERRATIO_MUL2;
    pTimeBaseCfg.Mode              = HRTIM_MODE_CONTINUOUS;
    if (HAL_HRTIM_TimeBaseConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_B, &pTimeBaseCfg) != HAL_OK)
    {
        log_e("HAL_HRTIM_TimeBaseConfig failed");
        return -ERR_IO;
    }
    
    pTimerCtl.UpDownMode           = HRTIM_TIMERUPDOWNMODE_UP;
    pTimerCtl.TrigHalf             = HRTIM_TIMERTRIGHALF_DISABLED;
    pTimerCtl.GreaterCMP3          = HRTIM_TIMERGTCMP3_EQUAL;
    pTimerCtl.GreaterCMP1          = HRTIM_TIMERGTCMP1_EQUAL;
    pTimerCtl.DualChannelDacEnable = HRTIM_TIMER_DCDE_DISABLED;
    if (HAL_HRTIM_WaveformTimerControl(&hhrtim1, HRTIM_TIMERINDEX_TIMER_B, &pTimerCtl) != HAL_OK)
    {
        log_e("HAL_HRTIM_WaveformTimerControl failed");
        return -ERR_IO;
    }
    
    pTimerCfg.InterruptRequests = HRTIM_TIM_IT_UPD | HRTIM_TIM_IT_REP;
    pTimerCfg.DMARequests       = HRTIM_TIM_DMA_NONE;
    pTimerCfg.DMASrcAddress     = 0x0000;
    pTimerCfg.DMADstAddress     = 0x0000;
    pTimerCfg.DMASize           = 0x1;
    
    pTimerCfg.PreloadEnable    = HRTIM_PRELOAD_DISABLED;
    pTimerCfg.UpdateTrigger    = HRTIM_TIMUPDATETRIGGER_NONE;
    pTimerCfg.RepetitionUpdate = HRTIM_UPDATEONREPETITION_DISABLED;
    pTimerCfg.UpdateGating     = HRTIM_UPDATEGATING_INDEPENDENT;
    pTimerCfg.ResetUpdate      = HRTIM_TIMUPDATEONRESET_ENABLED;
    pTimerCfg.ReSyncUpdate     = HRTIM_TIMERESYNC_UPDATE_UNCONDITIONAL;
    
    pTimerCfg.HalfModeEnable  = HRTIM_HALFMODE_DISABLED;
    pTimerCfg.InterleavedMode = HRTIM_INTERLEAVED_MODE_DISABLED;
    pTimerCfg.PushPull        = HRTIM_TIMPUSHPULLMODE_DISABLED;
    
    pTimerCfg.ResetTrigger = HRTIM_TIMRESETTRIGGER_NONE;
    pTimerCfg.StartOnSync  = HRTIM_SYNCSTART_DISABLED;
    pTimerCfg.ResetOnSync  = HRTIM_SYNCRESET_DISABLED;
    
    pTimerCfg.FaultEnable           = HRTIM_TIMFAULTENABLE_NONE;
    pTimerCfg.FaultLock             = HRTIM_TIMFAULTLOCK_READWRITE;
    pTimerCfg.DeadTimeInsertion     = HRTIM_TIMDEADTIMEINSERTION_DISABLED;
    pTimerCfg.DelayedProtectionMode = HRTIM_TIMER_A_B_C_DELAYEDPROTECTION_DISABLED;
    
    pTimerCfg.DACSynchro                  = HRTIM_DACSYNC_NONE;
    pTimerCfg.BurstMode                   = HRTIM_TIMERBURSTMODE_MAINTAINCLOCK;
    pTimerCfg.BalancedIdleAutomaticResume = HRTIM_OUTPUTBIAR_DISABLED;
    if (HAL_HRTIM_WaveformTimerConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_B, &pTimerCfg) != HAL_OK)
    {
        log_e("HAL_HRTIM_WaveformTimerConfig failed");
        return -ERR_IO;
    }
    
    pCompareCfg.CompareValue = 0xFFFF;
    if (HAL_HRTIM_WaveformCompareConfig(&hhrtim1, HRTIM_TIMERINDEX_TIMER_B, HRTIM_COMPAREUNIT_3, &pCompareCfg) != HAL_OK)
    {
        log_e("HAL_HRTIM_WaveformCompareConfig failed");
        return -ERR_IO;
    }
    
    return 0;
}

static int bsp_hrtim_fault_config(void)
{
    HRTIM_FaultCfgTypeDef pFaultCfg = {0};
    HRTIM_FaultBlankingCfgTypeDef pFaultBlkCfg = {0};
    
    if (HAL_HRTIM_FaultPrescalerConfig(&hhrtim1, HRTIM_FAULTPRESCALER_DIV1) != HAL_OK)
    {
        log_e("HAL_HRTIM_FaultPrescalerConfig failed");
        return -ERR_IO;
    }
    
    /* FAULT_4 */
    pFaultCfg.Source = HRTIM_FAULTSOURCE_INTERNAL;
    pFaultCfg.Polarity = HRTIM_FAULTPOLARITY_HIGH;
    pFaultCfg.Filter = HRTIM_FAULTFILTER_8; /* 持续时间 */
    pFaultCfg.Lock = HRTIM_FAULTLOCK_READWRITE;
    if (HAL_HRTIM_FaultConfig(&hhrtim1, HRTIM_FAULT_4, &pFaultCfg) != HAL_OK)
    {
        log_e("HAL_HRTIM_FaultConfig failed");
        return -ERR_IO;
    }
    
    pFaultBlkCfg.Threshold = FAULT_THRESHOLD_NUM - 1; /* 错误次数 */
    pFaultBlkCfg.ResetMode = HRTIM_FAULTCOUNTERRST_CONDITIONAL;
    if (HAL_HRTIM_FaultCounterConfig(&hhrtim1, HRTIM_FAULT_4, &pFaultBlkCfg) != HAL_OK)
    {
        log_e("HAL_HRTIM_FaultCounterConfig failed");
        return -ERR_IO;
    }
    
    pFaultBlkCfg.BlankingSource = HRTIM_FAULTBLANKINGMODE_RSTALIGNED;
    if (HAL_HRTIM_FaultBlankingConfigAndEnable(&hhrtim1, HRTIM_FAULT_4, &pFaultBlkCfg) != HAL_OK)
    {
        log_e("HAL_HRTIM_FaultBlankingConfigAndEnable failed");
        return -ERR_IO;
    }
    HAL_HRTIM_FaultModeCtl(&hhrtim1, HRTIM_FAULT_4, HRTIM_FAULTMODECTL_ENABLED);
    
    /* FAULT_5 */
    pFaultCfg.Source = HRTIM_FAULTSOURCE_INTERNAL;
    pFaultCfg.Polarity = HRTIM_FAULTPOLARITY_HIGH;
    pFaultCfg.Filter = HRTIM_FAULTFILTER_8;  /* 持续时间 */
    pFaultCfg.Lock = HRTIM_FAULTLOCK_READWRITE;
    if (HAL_HRTIM_FaultConfig(&hhrtim1, HRTIM_FAULT_5, &pFaultCfg) != HAL_OK)
    {
        log_e("HAL_HRTIM_FaultConfig failed");
        return -ERR_IO;
    }
    
    pFaultBlkCfg.Threshold = FAULT_THRESHOLD_NUM - 1; // 错误次数
    pFaultBlkCfg.ResetMode = HRTIM_FAULTCOUNTERRST_CONDITIONAL;
    if (HAL_HRTIM_FaultCounterConfig(&hhrtim1, HRTIM_FAULT_5, &pFaultBlkCfg) != HAL_OK)
    {
        log_e("HAL_HRTIM_FaultCounterConfig failed");
        return -ERR_IO;
    }
    
    pFaultBlkCfg.BlankingSource = HRTIM_FAULTBLANKINGMODE_RSTALIGNED;
    if (HAL_HRTIM_FaultBlankingConfigAndEnable(&hhrtim1, HRTIM_FAULT_5, &pFaultBlkCfg) != HAL_OK)
    {
        log_e("HAL_HRTIM_FaultBlankingConfigAndEnable failed");
        return -ERR_IO;
    }
    HAL_HRTIM_FaultModeCtl(&hhrtim1, HRTIM_FAULT_5, HRTIM_FAULTMODECTL_ENABLED);
    
    return 0;
}

static int bsp_hrtim_adc_trigger_config(void)
{
    HRTIM_ADCTriggerCfgTypeDef pADCTriggerCfg = {0};
    
    /* ADC 触发配置 */
    pADCTriggerCfg.UpdateSource = HRTIM_ADCTRIGGERUPDATE_TIMER_B;
    pADCTriggerCfg.Trigger = HRTIM_ADCTRIGGEREVENT13_TIMERB_CMP3;
    if (HAL_HRTIM_ADCTriggerConfig(&hhrtim1, HRTIM_ADCTRIGGER_1, &pADCTriggerCfg) != HAL_OK)
    {
        log_e("HAL_HRTIM_ADCTriggerConfig failed");
        return -ERR_IO;
    }
    if (HAL_HRTIM_ADCPostScalerConfig(&hhrtim1, HRTIM_ADCTRIGGER_1, 0x0) != HAL_OK)
    {
        log_e("HAL_HRTIM_ADCPostScalerConfig failed");
        return -ERR_IO;
    }
    
    pADCTriggerCfg.UpdateSource = HRTIM_ADCTRIGGERUPDATE_TIMER_B;
    pADCTriggerCfg.Trigger = HRTIM_ADCTRIGGEREVENT13_TIMERB_PERIOD;
    if (HAL_HRTIM_ADCTriggerConfig(&hhrtim1, HRTIM_ADCTRIGGER_3, &pADCTriggerCfg) != HAL_OK)
    {
        log_e("HAL_HRTIM_ADCTriggerConfig failed");
        return -ERR_IO;
    }
    if (HAL_HRTIM_ADCPostScalerConfig(&hhrtim1, HRTIM_ADCTRIGGER_3, 0x0) != HAL_OK)
    {
        log_e("HAL_HRTIM_ADCPostScalerConfig failed");
        return -ERR_IO;
    }
    
    return 0;
}

int bsp_hrtim_register_rep_callback(bsp_hrtim_rep_callback_t callback)
{
    if (callback == NULL) {
        return -ERR_INVAL;
    }

    s_rep_callback = callback;
    return 0;
}

void bsp_hrtim_configure_pulse(const bsp_hrtim_pulse_cfg_t *cfg)
{
    if (cfg == NULL) {
        return;
    }
    
    LL_HRTIM_TIM_SetPeriod(HRTIM1, LL_HRTIM_TIMER_A, cfg->periodA);
    LL_HRTIM_TIM_SetCompare1(HRTIM1, LL_HRTIM_TIMER_A, cfg->compareA1);
    LL_HRTIM_TIM_SetCompare2(HRTIM1, LL_HRTIM_TIMER_A, cfg->compareA2);
    LL_HRTIM_TIM_SetCompare3(HRTIM1, LL_HRTIM_TIMER_A, cfg->compareA3);
    LL_HRTIM_TIM_SetRepetition(HRTIM1, LL_HRTIM_TIMER_A, cfg->repetitionA);
    HAL_HRTIM_SoftwareUpdate(&hhrtim1, HRTIM_TIMERUPDATE_A);
    
    LL_HRTIM_TIM_SetPeriod(HRTIM1, LL_HRTIM_TIMER_B, cfg->periodB);
    LL_HRTIM_TIM_SetCompare3(HRTIM1, LL_HRTIM_TIMER_B, cfg->compareB3);
    LL_HRTIM_TIM_SetRepetition(HRTIM1, LL_HRTIM_TIMER_B, cfg->repetitionA);
    HAL_HRTIM_SoftwareUpdate(&hhrtim1, HRTIM_TIMERUPDATE_B);
}

void bsp_hrtim_start_output(void)
{
    HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TA1 | HRTIM_OUTPUT_TA2);
    HAL_HRTIM_WaveformCountStart_IT(&hhrtim1, HRTIM_TIMERID_TIMER_A | HRTIM_TIMERID_TIMER_B);
}

void bsp_hrtim_stop_output(void)
{
    HAL_HRTIM_WaveformOutputStop(&hhrtim1, HRTIM_OUTPUT_TA1 | HRTIM_OUTPUT_TA2);
    HAL_HRTIM_WaveformCountStop_IT(&hhrtim1, HRTIM_TIMERID_TIMER_A | HRTIM_TIMERID_TIMER_B);
    HAL_HRTIM_WaveformSetOutputLevel(&hhrtim1, HRTIM_TIMERINDEX_TIMER_A, HRTIM_OUTPUT_TA1, HRTIM_OUTPUTLEVEL_INACTIVE);
    HAL_HRTIM_WaveformSetOutputLevel(&hhrtim1, HRTIM_TIMERINDEX_TIMER_A, HRTIM_OUTPUT_TA2, HRTIM_OUTPUTLEVEL_INACTIVE);
    LL_HRTIM_TIM_SetCounter(HRTIM1, LL_HRTIM_TIMER_B, 0);
    LL_HRTIM_TIM_SetCounter(HRTIM1, LL_HRTIM_TIMER_A, 0);
}

void HAL_HRTIM_MspInit(HRTIM_HandleTypeDef* hrtimHandle)
{
    if(hrtimHandle->Instance == HRTIM1) 
    {
        /* HRTIM1 clock enable */
        __HAL_RCC_HRTIM1_CLK_ENABLE();
        
        /* HRTIM1 interrupt Init */
        HAL_NVIC_SetPriority(HRTIM1_TIMA_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(HRTIM1_TIMA_IRQn);
        HAL_NVIC_SetPriority(HRTIM1_FLT_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(HRTIM1_FLT_IRQn);
        
        HAL_NVIC_SetPriority(HRTIM1_TIMB_IRQn, 1, 0);
        HAL_NVIC_EnableIRQ(HRTIM1_TIMB_IRQn);
    }
}

void HAL_HRTIM_MspPostInit(HRTIM_HandleTypeDef* hrtimHandle)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if(hrtimHandle->Instance == HRTIM1)
    {
        __HAL_RCC_GPIOA_CLK_ENABLE();
        /**HRTIM1 GPIO Configuration
        PA8     ------> HRTIM1_CHA1
        PA9     ------> HRTIM1_CHA2
        */
        GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF13_HRTIM1;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    }
}

void HAL_HRTIM_MspDeInit(HRTIM_HandleTypeDef* hrtimHandle)
{
    if(hrtimHandle->Instance == HRTIM1)
    {
        /* Peripheral clock disable */
        __HAL_RCC_HRTIM1_CLK_DISABLE();
    }
}

void HRTIM1_TIMA_IRQHandler(void)
{
    if (LL_HRTIM_IsActiveFlag_REP(HRTIM1, LL_HRTIM_TIMER_A)) {
        LL_HRTIM_DisableOutput(HRTIM1, LL_HRTIM_OUTPUT_TA1 | LL_HRTIM_OUTPUT_TA2);
        LL_HRTIM_TIM_CounterDisable(HRTIM1, LL_HRTIM_TIMER_A);
        LL_HRTIM_ClearFlag_REP(HRTIM1, LL_HRTIM_TIMER_A);
        LL_HRTIM_TIM_SetCounter(HRTIM1, LL_HRTIM_TIMER_A, 0);
        if (s_rep_callback != NULL) {
            s_rep_callback();
        }
    }
}

void HRTIM1_TIMB_IRQHandler(void)
{
    if (LL_HRTIM_IsActiveFlag_UPDATE(HRTIM1, LL_HRTIM_TIMER_B)) {
        LL_HRTIM_ClearFlag_UPDATE(HRTIM1, LL_HRTIM_TIMER_B);
        LL_HRTIM_DisableIT_UPDATE(HRTIM1, LL_HRTIM_TIMER_B);
//        LL_HRTIM_TIM_SetCompare3(HRTIM1, LL_HRTIM_TIMER_B, s_hw_cfgs[s_burst_idx].new_compareB3);
//        LL_HRTIM_TIM_SetPeriod(HRTIM1, LL_HRTIM_TIMER_B, s_hw_cfgs[s_burst_idx].new_periodB);
//        HAL_HRTIM_SoftwareUpdate(&hhrtim1, HRTIM_TIMERUPDATE_B);
    }
    
    if (LL_HRTIM_IsActiveFlag_REP(HRTIM1, LL_HRTIM_TIMER_B)) {
        LL_HRTIM_ClearFlag_REP(HRTIM1, LL_HRTIM_TIMER_B);
        LL_HRTIM_TIM_CounterDisable(HRTIM1, LL_HRTIM_TIMER_B);
        LL_HRTIM_TIM_SetCounter(HRTIM1, LL_HRTIM_TIMER_B, 0);
    }
}

void HRTIM1_FLT_IRQHandler(void)
{
    if (LL_HRTIM_IsActiveFlag_FLT4(HRTIM1)) {
        LL_HRTIM_TIM_CounterDisable(HRTIM1, LL_HRTIM_TIMER_B);
        LL_HRTIM_ClearFlag_FLT4(HRTIM1);
        // TODO:
    }

    if (LL_HRTIM_IsActiveFlag_FLT5(HRTIM1)) {
        LL_HRTIM_TIM_CounterDisable(HRTIM1, LL_HRTIM_TIMER_B);
        LL_HRTIM_ClearFlag_FLT5(HRTIM1);
        // TODO:
    }
}

/* Private functions ---------------------------------------------------------*/
