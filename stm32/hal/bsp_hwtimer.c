/**
  ******************************************************************************
  * @file        : bsp_hwtimer.c
  * @author      : ZJY
  * @version     : V1.0
  * @date        : 2025-10-16
  * @brief       : STM32 HWTIMER driver implementation
  * @attattention: None
  ******************************************************************************
  * @history     :
  *         V1.0 : 1.Initial version
  *
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "bsp_hwtimer.h"
#include "hwtimer.h"
#include "bsp_tim.h"
#include "errno-base.h"
#include <stddef.h>
#include <stdbool.h>

/* Private typedef -----------------------------------------------------------*/
/**
 * @brief 定时器ID定义
 */
#define HWTIMER_ID_TIM6    0U    /**< TIM6 定时器ID */
#define HWTIMER_ID_TIM7    1U    /**< TIM7 定时器ID */

/* Private define ------------------------------------------------------------*/
#define HWTIMER_TIM6_ID    HWTIMER_ID_TIM6
#define HWTIMER_TIM7_ID    HWTIMER_ID_TIM7

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
/**
 * @brief 定时器模式状态（用于单次模式时自动停止）
 */
static struct {
    hwtimer_mode_t mode;        /**< 定时器模式 */
    bool oneshot_stopped;        /**< 单次模式是否已停止 */
} _timer_state[2];

/* Exported variables  -------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
/**
 * @brief 获取定时器句柄
 * @param timer_id 定时器ID
 * @return 定时器句柄指针，失败返回NULL
 */
static TIM_HandleTypeDef* _get_timer_handle(uint32_t timer_id);

/**
 * @brief 计算定时器时钟频率
 * @return 定时器时钟频率（Hz）
 */
static uint32_t _get_timer_clock_freq(void);

/**
 * @brief 配置定时器周期
 * @param htim 定时器句柄指针
 * @param period_us 周期（微秒）
 * @return 0成功，负值表示错误码
 */
static int _config_timer_period(TIM_HandleTypeDef *htim, uint32_t period_us);
static int _stm32_hwtimer_stop(uint32_t timer_id);

/* Exported functions --------------------------------------------------------*/

/**
 * @brief 初始化定时器硬件
 * @param timer_id 定时器ID
 * @return 0成功，负值表示错误码
 */
static int _stm32_hwtimer_init(uint32_t timer_id)
{
    TIM_HandleTypeDef *htim;
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    HAL_StatusTypeDef hal_ret;
    
    htim = _get_timer_handle(timer_id);
    if (htim == NULL)
    {
        return -EINVAL;
    }
    
    /* 如果定时器已经初始化，先反初始化 */
    if (htim->Instance != NULL)
    {
        (void)HAL_TIM_Base_DeInit(htim);
    }
    
    /* 配置定时器基本参数 */
    htim->Init.Prescaler = 0U;
    htim->Init.CounterMode = TIM_COUNTERMODE_UP;
    htim->Init.Period = 0xFFFFU;
    htim->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    
    hal_ret = HAL_TIM_Base_Init(htim);
    if (hal_ret != HAL_OK)
    {
        return -EIO;
    }
    
    /* 配置主输出触发 */
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    hal_ret = HAL_TIMEx_MasterConfigSynchronization(htim, &sMasterConfig);
    if (hal_ret != HAL_OK)
    {
        (void)HAL_TIM_Base_DeInit(htim);
        return -EIO;
    }
    
    /* 清除中断标志，但不使能中断（由start函数使能） */
    __HAL_TIM_CLEAR_FLAG(htim, TIM_FLAG_UPDATE);
    __HAL_TIM_DISABLE_IT(htim, TIM_IT_UPDATE);
    
    /* 初始化状态 */
    if (timer_id < 2U)
    {
        _timer_state[timer_id].mode = HWTIMER_MODE_PERIODIC;
        _timer_state[timer_id].oneshot_stopped = false;
    }
    
    return 0;
}

/**
 * @brief 反初始化定时器硬件
 * @param timer_id 定时器ID
 * @return 0成功，负值表示错误码
 */
static int _stm32_hwtimer_deinit(uint32_t timer_id)
{
    TIM_HandleTypeDef *htim;
    HAL_StatusTypeDef hal_ret;
    
    htim = _get_timer_handle(timer_id);
    if (htim == NULL)
    {
        return -EINVAL;
    }
    
    /* 停止定时器 */
    (void)_stm32_hwtimer_stop(timer_id);
    
    /* 反初始化 */
    hal_ret = HAL_TIM_Base_DeInit(htim);
    if (hal_ret != HAL_OK)
    {
        return -EIO;
    }
    
    return 0;
}

/**
 * @brief 启动定时器
 * @param timer_id 定时器ID
 * @param period_us 周期（微秒）
 * @param mode 定时器模式
 * @return 0成功，负值表示错误码
 */
static int _stm32_hwtimer_start(uint32_t timer_id, uint32_t period_us, hwtimer_mode_t mode)
{
    TIM_HandleTypeDef *htim;
    int ret;
    HAL_StatusTypeDef hal_ret;
    
    htim = _get_timer_handle(timer_id);
    if (htim == NULL)
    {
        return -EINVAL;
    }
    
    if (period_us == 0U)
    {
        return -EINVAL;
    }
    
    /* 配置定时器周期 */
    ret = _config_timer_period(htim, period_us);
    if (ret != 0)
    {
        return ret;
    }
    
    /* 保存模式 */
    if (timer_id < 2U)
    {
        _timer_state[timer_id].mode = mode;
        _timer_state[timer_id].oneshot_stopped = false;
    }
    
    /* 停止定时器（如果正在运行） */
    (void)HAL_TIM_Base_Stop_IT(htim);
    
    /* 清除中断标志 */
    __HAL_TIM_CLEAR_FLAG(htim, TIM_FLAG_UPDATE);
    
    /* 使能更新中断 */
    __HAL_TIM_ENABLE_IT(htim, TIM_IT_UPDATE);
    
    /* 启动定时器 */
    hal_ret = HAL_TIM_Base_Start_IT(htim);
    if (hal_ret != HAL_OK)
    {
        __HAL_TIM_DISABLE_IT(htim, TIM_IT_UPDATE);
        return -EIO;
    }
    
    return 0;
}

/**
 * @brief 停止定时器
 * @param timer_id 定时器ID
 * @return 0成功，负值表示错误码
 */
static int _stm32_hwtimer_stop(uint32_t timer_id)
{
    TIM_HandleTypeDef *htim;
    HAL_StatusTypeDef hal_ret;
    
    htim = _get_timer_handle(timer_id);
    if (htim == NULL)
    {
        return -EINVAL;
    }
    
    /* 禁用中断 */
    __HAL_TIM_DISABLE_IT(htim, TIM_IT_UPDATE);
    
    /* 停止定时器 */
    hal_ret = HAL_TIM_Base_Stop_IT(htim);
    if (hal_ret != HAL_OK)
    {
        return -EIO;
    }
    
    /* 清除中断标志 */
    __HAL_TIM_CLEAR_FLAG(htim, TIM_FLAG_UPDATE);
    
    return 0;
}

/**
 * @brief 设置定时器周期
 * @param timer_id 定时器ID
 * @param period_us 周期（微秒）
 * @return 0成功，负值表示错误码
 */
static int _stm32_hwtimer_set_period(uint32_t timer_id, uint32_t period_us)
{
    TIM_HandleTypeDef *htim;
    int ret;
    bool was_running = false;
    
    htim = _get_timer_handle(timer_id);
    if (htim == NULL)
    {
        return -EINVAL;
    }
    
    if (period_us == 0U)
    {
        return -EINVAL;
    }
    
    /* 检查定时器是否正在运行 */
    if (__HAL_TIM_IS_TIM_COUNTING_DOWN(htim) || (htim->Instance->CR1 & TIM_CR1_CEN) != 0U)
    {
        was_running = true;
        (void)_stm32_hwtimer_stop(timer_id);
    }
    
    /* 配置周期 */
    ret = _config_timer_period(htim, period_us);
    if (ret != 0)
    {
        return ret;
    }
    
    /* 如果之前正在运行，重新启动 */
    if (was_running)
    {
        hwtimer_mode_t mode = HWTIMER_MODE_PERIODIC;
        if (timer_id < 2U)
        {
            mode = _timer_state[timer_id].mode;
        }
        ret = _stm32_hwtimer_start(timer_id, period_us, mode);
    }
    
    return ret;
}

/**
 * @brief 获取定时器当前计数值
 * @param timer_id 定时器ID
 * @return 当前计数值，失败返回0
 */
static uint32_t _stm32_hwtimer_get_count(uint32_t timer_id)
{
    TIM_HandleTypeDef *htim;
    
    htim = _get_timer_handle(timer_id);
    if (htim == NULL)
    {
        return 0U;
    }
    
    return (uint32_t)__HAL_TIM_GET_COUNTER(htim);
}

/**
 * @brief 获取定时器最大周期值（微秒）
 * @param timer_id 定时器ID
 * @return 最大周期值（微秒）
 */
static uint32_t _stm32_hwtimer_get_max_period(uint32_t timer_id)
{
    uint32_t fclk_hz;
    uint64_t max_ticks;
    uint64_t max_period_us;
    
    (void)timer_id;  /* 未使用参数 */
    
    fclk_hz = _get_timer_clock_freq();
    if (fclk_hz == 0U)
    {
        return 0U;
    }
    
    /* 最大计数值 = PSC_MAX * ARR_MAX = 65536 * 65536 */
    max_ticks = 65536ULL * 65536ULL;
    
    /* 转换为微秒 */
    max_period_us = (max_ticks * 1000000ULL) / (uint64_t)fclk_hz;
    
    /* 限制在32位范围内 */
    if (max_period_us > 0xFFFFFFFFULL)
    {
        return 0xFFFFFFFFU;
    }
    
    return (uint32_t)max_period_us;
}

/**
 * @brief 获取定时器最小周期值（微秒）
 * @param timer_id 定时器ID
 * @return 最小周期值（微秒）
 */
static uint32_t _stm32_hwtimer_get_min_period(uint32_t timer_id)
{
    uint32_t fclk_hz;
    uint64_t min_period_us;
    
    (void)timer_id;  /* 未使用参数 */
    
    fclk_hz = _get_timer_clock_freq();
    if (fclk_hz == 0U)
    {
        return 0U;
    }
    
    /* 最小周期 = 1个时钟周期 */
    min_period_us = 1000000ULL / (uint64_t)fclk_hz;
    if (min_period_us == 0ULL)
    {
        min_period_us = 1ULL;
    }
    
    return (uint32_t)min_period_us;
}

/**
 * @brief 获取定时器分辨率（微秒）
 * @param timer_id 定时器ID
 * @return 分辨率（微秒）
 */
static uint32_t _stm32_hwtimer_get_resolution(uint32_t timer_id)
{
    uint32_t fclk_hz;
    uint64_t resolution_us;
    
    (void)timer_id;  /* 未使用参数 */
    
    fclk_hz = _get_timer_clock_freq();
    if (fclk_hz == 0U)
    {
        return 0U;
    }
    
    /* 分辨率 = 1个时钟周期对应的微秒数 */
    resolution_us = 1000000ULL / (uint64_t)fclk_hz;
    if (resolution_us == 0ULL)
    {
        resolution_us = 1ULL;
    }
    
    return (uint32_t)resolution_us;
}

/**
 * @brief 硬件定时器操作函数结构体
 */
static const struct hwtimer_ops _stm32_hwtimer_ops = {
    .init = _stm32_hwtimer_init,
    .deinit = _stm32_hwtimer_deinit,
    .start = _stm32_hwtimer_start,
    .stop = _stm32_hwtimer_stop,
    .set_period = _stm32_hwtimer_set_period,
    .get_count = _stm32_hwtimer_get_count,
    .get_max_period = _stm32_hwtimer_get_max_period,
    .get_min_period = _stm32_hwtimer_get_min_period,
    .get_resolution = _stm32_hwtimer_get_resolution,
};

/**
 * @brief 初始化STM32硬件定时器驱动
 * @return 0成功，负值表示错误码
 */
int bsp_hwtimer_init(void)
{
    return hwtimer_register(&_stm32_hwtimer_ops);
}

/* Private functions ---------------------------------------------------------*/

/**
 * @brief 获取定时器句柄
 * @param timer_id 定时器ID
 * @return 定时器句柄指针，失败返回NULL
 */
static TIM_HandleTypeDef* _get_timer_handle(uint32_t timer_id)
{
    TIM_HandleTypeDef *htim = NULL;
    
    switch (timer_id)
    {
        case HWTIMER_ID_TIM6:
            htim = &htim6;
            break;
            
        case HWTIMER_ID_TIM7:
            htim = &htim7;
            break;
            
        default:
            break;
    }
    
    return htim;
}

/**
 * @brief 计算定时器时钟频率
 * @return 定时器时钟频率（Hz）
 */
static uint32_t _get_timer_clock_freq(void)
{
    RCC_ClkInitTypeDef clkconfig;
    uint32_t pFLatency;
    uint32_t uwAPB1Prescaler;
    uint32_t uwTimclock;
    
    /* 获取时钟配置 */
    HAL_RCC_GetClockConfig(&clkconfig, &pFLatency);
    
    /* 获取APB1分频器 */
    uwAPB1Prescaler = clkconfig.APB1CLKDivider;
    
    /* 计算定时器时钟频率 */
    /* 如果APB1分频为1，定时器时钟 = APB1时钟，否则定时器时钟 = 2 * APB1时钟 */
    if (uwAPB1Prescaler == RCC_HCLK_DIV1)
    {
        uwTimclock = HAL_RCC_GetPCLK1Freq();
    }
    else
    {
        uwTimclock = 2U * HAL_RCC_GetPCLK1Freq();
    }
    
    return uwTimclock;
}

/**
 * @brief 配置定时器周期
 * @param htim 定时器句柄指针
 * @param period_us 周期（微秒）
 * @return 0成功，负值表示错误码
 */
static int _config_timer_period(TIM_HandleTypeDef *htim, uint32_t period_us)
{
    uint32_t fclk_hz;
    uint16_t psc;
    uint16_t arr;
    uint64_t period_ns;
    HAL_StatusTypeDef hal_ret;
    int ret;
    
    if (htim == NULL)
    {
        return -EINVAL;
    }
    
    /* 获取定时器时钟频率 */
    fclk_hz = _get_timer_clock_freq();
    if (fclk_hz == 0U)
    {
        return -EIO;
    }
    
    /* 转换为纳秒 */
    period_ns = (uint64_t)period_us * 1000ULL;
    
    /* 计算分频器和周期值 */
    ret = tim_pick_psc_arr_from_ns(fclk_hz, period_ns, &psc, &arr, NULL);
    if (ret < 0)
    {
        return -EINVAL;
    }
    
    /* 配置定时器 */
    htim->Init.Prescaler = psc;
    htim->Init.Period = arr;
    
    hal_ret = HAL_TIM_Base_Init(htim);
    if (hal_ret != HAL_OK)
    {
        return -EIO;
    }
    
    return 0;
}

/**
 * @brief TIM6和TIM7中断回调函数（在HAL_TIM_PeriodElapsedCallback中调用）
 * @param htim 定时器句柄指针
 */
void bsp_hwtimer_period_elapsed_callback(TIM_HandleTypeDef *htim)
{
    uint32_t timer_id;
    
    if (htim == NULL)
    {
        return;
    }
    
    /* 确定定时器ID */
    if (htim->Instance == TIM6)
    {
        timer_id = HWTIMER_ID_TIM6;
    }
    else if (htim->Instance == TIM7)
    {
        timer_id = HWTIMER_ID_TIM7;
    }
    else
    {
        return;
    }
    
    /* 如果是单次模式，停止定时器 */
    if (timer_id < 2U)
    {
        if (_timer_state[timer_id].mode == HWTIMER_MODE_ONESHOT)
        {
            if (!_timer_state[timer_id].oneshot_stopped)
            {
                _timer_state[timer_id].oneshot_stopped = true;
                (void)_stm32_hwtimer_stop(timer_id);
            }
        }
    }
    
    /* 调用上层回调函数 */
    hwtimer_irq_callback(timer_id);
}


