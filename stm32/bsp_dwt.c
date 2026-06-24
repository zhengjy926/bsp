/**
  ******************************************************************************
  * @file        : bsp_dwt.c
  * @author      : ZJY
  * @version     : V1.0
  * @data        : 2025-10-16
  * @brief       : STM32 DWT driver implementation
  * @attention   : None
  ******************************************************************************
  * @history     :
  *         V1.0 : 1.Initial version
  *
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "bsp_dwt.h"
#include "bsp_conf.h"

#define  LOG_TAG             "bsp_dwt"
#define  LOG_LVL             ELOG_LVL_DEBUG
#include "elog.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
static uint32_t cycles_per_us = 0U;
static uint32_t max_safe_us = 0U;

/* Exported variables  -------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Exported functions --------------------------------------------------------*/
void BSP_DWT_Init(void)
{
#if (__CORTEX_M < 3)
    #error "DWT is not supported on this core"
#endif
    DCB->DEMCR |= DCB_DEMCR_TRCENA_Msk;
    DWT->CTRL  |= DWT_CTRL_CYCCNTENA_Msk;
    DWT->CYCCNT = 0U;

    cycles_per_us = SystemCoreClock / 1000000;
    max_safe_us = 0xFFFFFFFFU / cycles_per_us;
}

/**
 * @brief  Gets the current value of the DWT cycle counter.
 * @param  None
 * @retval Current cycle count.
 */
uint32_t BSP_DWT_GetTick(void)
{
    return DWT->CYCCNT;
}

void BSP_DWT_DelayUs(uint32_t time_us)
{
    /* 使用断言保护，防止 time_us 过大导致溢出 */
    ELOG_ASSERT(time_us <= max_safe_us);
    uint32_t ticks_start = DWT->CYCCNT;
    uint32_t ticks_delay = time_us * cycles_per_us;
    
    while ((DWT->CYCCNT - ticks_start) < ticks_delay);
}

void BSP_DWT_DelayMs(uint32_t time_ms)
{
    /* 使用断言保护，防止 1000 * time_ms 在 32 位下溢出 */
    /* 0xFFFFFFFF / 1000 = 4294967，最大安全延时约 4294 秒 */
    ELOG_ASSERT(time_ms <= (0xFFFFFFFFU / 1000U));

    while (time_ms--) {
        BSP_DWT_DelayUs(1000U);
    }
}
/* Private functions ---------------------------------------------------------*/

/* End of bsp_dwt.c -----------------------------------------------------------------------*/
