/**
  ******************************************************************************
  * @file        : stm32_dwt.c
  * @author      : ZJY
  * @version     : V1.0
  * @data        : 20xx-xx-xx
  * @brief       : 
  * @attattention: None
  ******************************************************************************
  * @history     :
  *         V1.0 : 1.xxx
  *
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "stm32_dwt.h"

#if defined(SOC_SERIES_STM32F1)
    #include "stm32f1xx.h"
#elif defined(SOC_SERIES_STM32F4)
    #include "stm32f4xx.h"
#elif defined(SOC_SERIES_STM32G4)
    #include "stm32g4xx.h"
#else
#error "Please select first the soc series used in your application!"    
#endif
/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
// 用于存储溢出次数，结合CYCCNT构成一个64位计数器
static volatile uint64_t g_dwt_overflow_cycles = 0;
// 上一次读取的CYCCNT值，用于判断溢出
static volatile uint32_t g_dwt_last_cyccnt = 0;

/* Exported variables  -------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Exported functions --------------------------------------------------------*/
int32_t stm32_dwt_init(void)
{
    /* 检查内核是否支持DWT, Cortex-M0/M0+ 不支持 */ 
#if (__CORTEX_M < 3)
    return -1; // 不支持
#endif
    
    DCB->DEMCR |= DCB_DEMCR_TRCENA_Msk;
    DWT->CTRL  |= DWT_CTRL_CYCCNTENA_Msk;
    
    return 0;
}

/**
 * @brief  Gets the current value of the DWT cycle counter.
 * @param  None
 * @retval Current cycle count.
 */
uint32_t dwt_get_tick(void)
{
    return DWT->CYCCNT;
}

void dwt_update_overflow_counter(void)
{
    uint32_t current_cyccnt = DWT->CYCCNT;
    
    /* 检查是否发生溢出 (当前值小于上一次的值) */
    if (current_cyccnt < g_dwt_last_cyccnt)
    {
        // 0x100000000ULL 表示 2^32
        g_dwt_overflow_cycles += 0x100000000ULL; 
    }
    
    g_dwt_last_cyccnt = current_cyccnt;
}

double dwt_get_seconds(void)
{
    /* 为保证读取的原子性，临时禁用中断 */
    uint32_t primask = __get_PRIMASK();
    __disable_irq();

    uint32_t current_cyccnt = DWT->CYCCNT;
    uint64_t overflow_cycles = g_dwt_overflow_cycles;
    
    /* 再次检查溢出，防止在读取 g_dwt_overflow_cycles 和 DWT->CYCCNT 之间
     * 发生 SysTick 中断并且DWT恰好溢出。
     */
    if (current_cyccnt < g_dwt_last_cyccnt)
    {
        overflow_cycles += 0x100000000ULL;
    }

    __set_PRIMASK(primask);
    /* 恢复中断状态 */

    uint64_t total_cycles = overflow_cycles + current_cyccnt;

    return (double)total_cycles / SystemCoreClock;
}

void dwt_delay_us(uint32_t time_us)
{
	uint32_t ticks_start, ticks_cnt, ticks_delay = 0;
    
    ticks_start = DWT->CYCCNT;
	ticks_delay = time_us * (SystemCoreClock / 1000000);

	while(ticks_cnt < ticks_delay)
	{
		ticks_cnt = DWT->CYCCNT - ticks_start;
	}
}

void dwt_delay_ms(uint32_t time_ms)
{
    dwt_delay_us(1000 * time_ms);
}
/* Private functions ---------------------------------------------------------*/

/* End of stm32_dwt.c -----------------------------------------------------------------------*/
