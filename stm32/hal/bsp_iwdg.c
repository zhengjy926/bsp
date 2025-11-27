/**
  ******************************************************************************
  * @file        : bsp_iwdg.c
  * @author      : ZJY
  * @version     : V1.0
  * @date        : 2025-01-XX
  * @brief       : STM32 IWDG driver implementation
  * @attention   : None
  ******************************************************************************
  * @history     :
  *         V1.0 : 1.实现STM32 IWDG看门狗驱动
  *
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "bsp_iwdg.h"
#include "bsp_conf.h"
#include "watchdog.h"
#include "errno-base.h"
#include "board.h"
#include <stdint.h>
#include <stddef.h>

#define  LOG_TAG             "bsp_iwdg"
#define  LOG_LVL             4
#include "log.h"

/* Private typedef -----------------------------------------------------------*/

/**
 * @brief IWDG预分频器配置结构体
 */
typedef struct {
    uint32_t prescaler;      /**< LL库预分频器值 */
    uint32_t div;            /**< 实际分频值 */
} iwdg_prescaler_t;

/* Private define ------------------------------------------------------------*/
#define IWDG_DEFAULT_READY_TIMEOUT      (((6UL * 256UL * 1000UL) / LSI_VALUE) + ((LSI_STARTUP_TIME / 1000UL) + 1UL))
#define IWDG_DEFAULT_TIMEOUT            (1000U)

/* IWDG超时时间范围（毫秒） */
#define IWDG_MIN_TIMEOUT_MS            125U     /**< 最小超时时间（约125us，实际为毫秒级） */
#define IWDG_MAX_TIMEOUT_MS            32700U   /**< 最大超时时间（约32.7秒） */

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/**
 * @brief IWDG配置状态
 */
static struct {
    uint32_t prescaler;      /**< 预分频器值 */
    uint32_t reload;         /**< 重载值 */
    uint32_t timeout_ms;     /**< 超时时间（毫秒） */
    uint8_t configured;      /**< 是否已配置 */
} _iwdg_config = {
    .prescaler = 0U,
    .reload = 0U,
    .timeout_ms = 0U,
    .configured = 0U
};

/**
 * @brief IWDG预分频器配置表
 */
static const iwdg_prescaler_t _prescaler_table[] = {
    {LL_IWDG_PRESCALER_4,   4U},
    {LL_IWDG_PRESCALER_8,   8U},
    {LL_IWDG_PRESCALER_16,  16U},
    {LL_IWDG_PRESCALER_32,  32U},
    {LL_IWDG_PRESCALER_64,  64U},
    {LL_IWDG_PRESCALER_128, 128U},
    {LL_IWDG_PRESCALER_256, 256U}
};

#define PRESCALER_TABLE_SIZE   (sizeof(_prescaler_table) / sizeof(_prescaler_table[0]))

/* Exported variables  -------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
/**
 * @brief 根据超时时间计算预分频器和重载值
 * @param timeout_ms 超时时间（毫秒）
 * @param prescaler 输出的预分频器值
 * @param reload 输出的重载值
 * @return 0成功，-EINVAL表示超时时间超出范围
 */
static int _calculate_timeout(uint32_t timeout_ms, uint32_t *prescaler, uint32_t *reload);

/**
 * @brief 根据预分频器和重载值计算超时时间
 * @param prescaler 预分频器值
 * @param reload 重载值
 * @return 超时时间（毫秒）
 */
static uint32_t _get_timeout_from_config(uint32_t prescaler, uint32_t reload);

/* Exported functions --------------------------------------------------------*/

/**
 * @brief 启动看门狗（必需操作）
 * @param dev 看门狗设备指针
 * @return 0成功，负值表示错误码
 */
static int _iwdg_start(struct watchdog_device *dev)
{
    uint32_t tickstart;
    
    if (dev == NULL)
    {
        LOG_E("Invalid parameter: dev is NULL");
        return -EINVAL;
    }
    
    /* 检查是否已配置超时时间 */
    if (_iwdg_config.configured == 0U)
    {
        LOG_E("IWDG timeout not configured! Please call watchdog_set_timeout first");
        return -EINVAL;
    }
    
    /* 检查设备超时时间是否与配置一致 */
    if (dev->timeout_ms != _iwdg_config.timeout_ms)
    {
        LOG_W("IWDG device timeout (%lu ms) != configured timeout (%lu ms), using configured value",
              dev->timeout_ms, _iwdg_config.timeout_ms);
    }
    
    LL_IWDG_Enable(IWDG);
    
    /* 开启写访问 */
    LL_IWDG_EnableWriteAccess(IWDG);
    
    /* 设置预分频器（使用已配置的值） */
    LL_IWDG_SetPrescaler(IWDG, _iwdg_config.prescaler);
    
    /* 设置重载值（使用已配置的值） */
    LL_IWDG_SetReloadCounter(IWDG, _iwdg_config.reload);
    
    /* 等待寄存器更新完成 */
    tickstart = HAL_GetTick();
    while (LL_IWDG_IsReady(IWDG) != 1)
    {
        if ((HAL_GetTick() - tickstart) > IWDG_DEFAULT_READY_TIMEOUT)
        {
            LOG_W("IWDG ready timeout!");
            return -ETIMEOUT;
        }
    }
    
    /* 重载计数器，启动看门狗 */
    LL_IWDG_ReloadCounter(IWDG);
    
    LOG_I("IWDG started, timeout: %lu ms", _iwdg_config.timeout_ms);
    
    return 0;
}

/**
 * @brief 喂狗
 * @param dev 看门狗设备指针
 * @return 0成功，负值表示错误码
 */
static int _iwdg_ping(struct watchdog_device *dev)
{
    (void)dev;
    
    /* 重载计数器 */
    LL_IWDG_ReloadCounter(IWDG);
    
    return 0;
}

/**
 * @brief 获取看门狗状态（可选操作）
 * @param dev 看门狗设备指针
 * @return 状态值（0=停止，1=运行）
 */
static uint32_t _iwdg_status(struct watchdog_device *dev)
{
    if (dev == NULL)
    {
        return 0U;
    }
    
    /* IWDG一旦启动就一直运行，无法停止 */
    /* 检查是否已经启动（通过检查寄存器状态） */
    if (LL_IWDG_IsReady(IWDG) == 1)
    {
        return 1U;  /* 运行中 */
    }
    
    return (uint32_t)dev->state;
}

/**
 * @brief 设置看门狗超时时间（可选操作）
 * @param dev 看门狗设备指针
 * @param timeout_ms 超时时间（毫秒）
 * @return 0成功，负值表示错误码
 */
static int _iwdg_set_timeout(struct watchdog_device *dev, uint32_t timeout_ms)
{
    uint32_t prescaler;
    uint32_t reload;
    int ret;
    
    if (dev == NULL)
    {
        LOG_E("Invalid parameter: dev is NULL");
        return -EINVAL;
    }
    
    /* 检查是否已经启动，如果已启动则无法修改配置 */
    if (dev->state == WATCHDOG_STATE_RUNNING)
    {
        LOG_W("IWDG already started, cannot change timeout. Current timeout: %lu ms", 
              _iwdg_config.timeout_ms);
        return -EBUSY;
    }
    
    /* 计算预分频器和重载值 */
    ret = _calculate_timeout(timeout_ms, &prescaler, &reload);
    if (ret != 0)
    {
        LOG_E("IWDG timeout out of range: %lu ms", timeout_ms);
        return ret;
    }
    
    /* 保存配置 */
    _iwdg_config.prescaler = prescaler;
    _iwdg_config.reload = reload;
    _iwdg_config.timeout_ms = timeout_ms;
    _iwdg_config.configured = 1U;
    
    /* 更新设备超时时间 */
    dev->timeout_ms = timeout_ms;
    
    LOG_I("IWDG timeout configured: %lu ms (prescaler=0x%lX, reload=%lu)", 
          timeout_ms, prescaler, reload);
    
    return 0;
}

/**
 * @brief 设置看门狗预超时时间（可选操作）
 * @param dev 看门狗设备指针
 * @param timeout_ms 预超时时间（毫秒）
 * @return -ENOSYS（IWDG不支持预超时功能）
 */
static int _iwdg_set_pretimeout(struct watchdog_device *dev, uint32_t timeout_ms)
{
    (void)dev;
    (void)timeout_ms;
    
    /* STM32 IWDG不支持预超时功能 */
    return -ENOSYS;
}

/**
 * @brief 获取看门狗超时时间（可选操作）
 * @param dev 看门狗设备指针
 * @return 超时时间（毫秒）
 */
static uint32_t _iwdg_get_timeout(const struct watchdog_device *dev)
{
    uint32_t prescaler;
    uint32_t reload;
    
    if (dev == NULL)
    {
        return 0U;
    }
    
    /* 从寄存器读取当前配置 */
    prescaler = LL_IWDG_GetPrescaler(IWDG);
    reload = LL_IWDG_GetReloadCounter(IWDG);
    
    /* 计算超时时间 */
    return _get_timeout_from_config(prescaler, reload);
}

/**
 * @brief 获取看门狗最大超时时间（可选操作）
 * @param dev 看门狗设备指针
 * @return 最大超时时间（毫秒）
 */
static uint32_t _iwdg_get_max_timeout(const struct watchdog_device *dev)
{
    (void)dev;
    return IWDG_MAX_TIMEOUT_MS;
}

/**
 * @brief 获取看门狗最小超时时间（可选操作）
 * @param dev 看门狗设备指针
 * @return 最小超时时间（毫秒）
 */
static uint32_t _iwdg_get_min_timeout(const struct watchdog_device *dev)
{
    (void)dev;
    return IWDG_MIN_TIMEOUT_MS;
}

/**
 * @brief IWDG操作函数结构体
 */
static const struct watchdog_ops _iwdg_ops = {
    .start = _iwdg_start,
    .stop = NULL,
    .ping = _iwdg_ping,
    .status = _iwdg_status,
    .set_timeout = _iwdg_set_timeout,
    .set_pretimeout = _iwdg_set_pretimeout,
    .get_timeout = _iwdg_get_timeout,
    .get_max_timeout = _iwdg_get_max_timeout,
    .get_min_timeout = _iwdg_get_min_timeout
};

/**
 * @brief 初始化STM32 IWDG驱动
 * @return 0成功，负值表示错误码
 */
int bsp_iwdg_init(void)
{
    struct watchdog_device *dev;
    
    /* 查找或创建IWDG设备 */
    dev = watchdog_find("iwdg");
    if (dev == NULL)
    {
        LOG_E("Failed to create IWDG device!");
        return -ENOMEM;
    }
    
    /* 注册操作函数 */
    if (watchdog_register(dev, &_iwdg_ops) != 0)
    {
        LOG_E("Failed to register IWDG ops!");
        return -EINVAL;
    }
    
    LOG_I("IWDG driver initialized");
    
    return 0;
}

/* Private functions ---------------------------------------------------------*/

/**
 * @brief 根据超时时间计算预分频器和重载值
 * @param timeout_ms 超时时间（毫秒）
 * @param prescaler 输出的预分频器值
 * @param reload 输出的重载值
 * @return 0成功，-EINVAL表示超时时间超出范围
 */
static int _calculate_timeout(uint32_t timeout_ms, uint32_t *prescaler, uint32_t *reload)
{
    uint32_t i;
    uint32_t div;
    uint32_t reload_val;
    uint32_t calculated_timeout;
    
    if (prescaler == NULL || reload == NULL)
    {
        return -EINVAL;
    }
    
    /* 检查超时时间范围 */
    if (timeout_ms < IWDG_MIN_TIMEOUT_MS || timeout_ms > IWDG_MAX_TIMEOUT_MS)
    {
        return -EINVAL;
    }
    
    /* 遍历所有预分频器，找到最合适的配置 */
    for (i = 0U; i < PRESCALER_TABLE_SIZE; i++)
    {
        div = _prescaler_table[i].div;
        
        /* 计算重载值：timeout_ms = (reload + 1) * prescaler / LSI_VALUE * 1000 */
        /* reload = (timeout_ms * LSI_VALUE) / (prescaler * 1000) - 1 */
        reload_val = ((timeout_ms * LSI_VALUE) / (div * 1000U)) - 1U;
        
        /* 检查重载值是否在有效范围内（0-0xFFF） */
        if (reload_val <= 0xFFFU)
        {
            /* 验证计算出的超时时间 */
            calculated_timeout = _get_timeout_from_config(_prescaler_table[i].prescaler, reload_val);
            
            /* 如果计算出的超时时间在允许范围内，使用此配置 */
            if (calculated_timeout >= (timeout_ms * 9U / 10U) && 
                calculated_timeout <= (timeout_ms * 11U / 10U))
            {
                *prescaler = _prescaler_table[i].prescaler;
                *reload = reload_val;
                return 0;
            }
        }
    }
    
    /* 如果找不到合适的配置，使用最大超时时间的配置 */
    *prescaler = LL_IWDG_PRESCALER_256;
    *reload = 0xFFFU;
    
    return 0;
}

/**
 * @brief 根据预分频器和重载值计算超时时间
 * @param prescaler 预分频器值
 * @param reload 重载值
 * @return 超时时间（毫秒）
 */
static uint32_t _get_timeout_from_config(uint32_t prescaler, uint32_t reload)
{
    uint32_t i;
    uint32_t div = 4U;  /* 默认值 */
    
    /* 查找对应的分频值 */
    for (i = 0U; i < PRESCALER_TABLE_SIZE; i++)
    {
        if (_prescaler_table[i].prescaler == prescaler)
        {
            div = _prescaler_table[i].div;
            break;
        }
    }
    
    /* 计算超时时间：timeout_ms = (reload + 1) * prescaler / LSI_VALUE * 1000 */
    return ((reload + 1U) * div * 1000U) / LSI_VALUE;
}





