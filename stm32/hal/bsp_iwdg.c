/**
  ******************************************************************************
  * @file        : bsp_iwdg.c
  * @author      : ZJY
  * @version     : V1.0
  * @date        : 2025-01-21
  * @brief       : STM32 IWDG driver implementation
  * @attention   : 目前并未实现任何并发保护，在多线程下存在数据竞争和列表被并发
  *                修改的风险，因此需在“单线程访问每个设备 + 注册阶段单线程”的约
  *                束下，才能满足多线程环境下的安全使用。
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
#include "wdg.h"
#include "errno-base.h"
#include "board.h"
#include "mymath.h"
#include "log2.h"
#include <stdint.h>
#include <stddef.h>

#define  LOG_TAG             "bsp_iwdg"
#define  LOG_LVL             3
#include "log.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
#define PR_SHIFT                        (2U)

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
static IWDG_HandleTypeDef hiwdg;

/* Exported variables  -------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Exported functions --------------------------------------------------------*/
static int stm32_iwdg_start(struct wdg_device *wdg)
{
    uint32_t presc = 0;
    HAL_StatusTypeDef status;
    
    presc = DIV_ROUND_UP(wdg->timeout * LSI_VALUE, IWDG_RLR_RL + 1);
    presc = roundup_pow_of_two(presc);
    if (presc < 4U) {
        presc = 4U;
    }
    
    hiwdg.Instance = IWDG;
    hiwdg.Init.Prescaler = ( presc <= (1 << PR_SHIFT) ) ? 0 : (ilog2(presc) - PR_SHIFT);
    hiwdg.Init.Reload = ((wdg->timeout * LSI_VALUE) / presc) - 1;
#ifdef IWDG_WINR_WIN
    hiwdg.Init.Window = hiwdg.Init.Reload;
#endif
    status = HAL_IWDG_Init(&hiwdg);
    if (status != HAL_OK) {
        return -EIO;
    }
    
    return 0;
}

static int stm32_iwdg_feed(struct wdg_device *wdg)
{
    HAL_IWDG_Refresh(&hiwdg);
    return 0;
}

static uint32_t stm32_iwdg_status(struct wdg_device *wdg)
{
    /* IWDG一旦启动就一直运行，无法停止 */
    (void)wdg;
    return (LL_IWDG_IsReady(IWDG) == 1) ? (uint32_t)WDOG_HW_RUNNING : 0U;
}

static int stm32_iwdg_set_timeout(struct wdg_device *wdg, uint32_t timeout)
{
    int ret = 0;
    uint32_t old_timeout;
    
    old_timeout = wdg->timeout;
    wdg->timeout = timeout;
    ret = stm32_iwdg_start(wdg);
    if (ret == 0) {
        stm32_iwdg_feed(wdg);
    } else {
        wdg->timeout = old_timeout;  /* 失败则设备状态不变 */
        ret = -EIO;
    }
    
    return ret;
}

/**
 * @brief IWDG操作函数结构体
 */
static const struct wdg_ops stm32_iwdg_ops = {
    .start = stm32_iwdg_start,
    .stop = NULL,
    .feed = stm32_iwdg_feed,
    .status = stm32_iwdg_status,
    .set_timeout = stm32_iwdg_set_timeout,
    .set_pretimeout = NULL,
};

/**
 * @brief 
 */
static struct wdg_device stm32_iwdg = {
    .name           = "stm32_iwdg",
    .ops            = &stm32_iwdg_ops,
    .timeout        = 1,
    .min_timeout    = 1,
    .max_timeout    = 300,
};

/**
 * @brief 初始化STM32 IWDG驱动
 * @return 0成功，负值表示错误码
 */
int bsp_iwdg_init(void)
{
    int ret;
    
    ret = wdg_register_device(&stm32_iwdg);
    if (ret != 0) {
        return ret;
    }
    
    return 0;
}

/* Private functions ---------------------------------------------------------*/
