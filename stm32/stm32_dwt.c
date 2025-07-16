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
#include "board.h"
/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
/**
 * @brief DWT registers
 */
#define  DWT_CYCCNT                 (*(volatile uint32_t *)0xE0001004)
#define  DWT_CR                     (*(volatile uint32_t *)0xE0001000)
#define  DEM_CR                     (*(volatile uint32_t *)0xE000EDFC)
#define  DBGMCU_CR                  (*(volatile uint32_t *)0xE0042004)

#define  DEM_CR_TRCENA              (1 << 24)
#define  DWT_CR_CYCCNTENA           (1 <<  0)
/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Exported variables  -------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Exported functions --------------------------------------------------------*/
void stm32_dwt_init(void)
{
	DEM_CR     |= (uint32_t)DEM_CR_TRCENA;   ///< Enable Cortex-M4's DWT CYCCNT reg.
	DWT_CYCCNT  = (uint32_t)0u;
	DWT_CR     |= (uint32_t)DWT_CR_CYCCNTENA;
}

void dwt_delay_us(uint32_t time_us)
{
	uint32_t ticks_start, ticks_cnt, ticks_delay = 0;
    
    ticks_start = DWT_CYCCNT;
	ticks_delay = time_us * (SystemCoreClock / 1000000);

	while(ticks_cnt < ticks_delay)
	{
		ticks_cnt = DWT_CYCCNT - ticks_start;
	}
}

void dwt_delay_ms(uint32_t time_ms)
{
    dwt_delay_us(1000 * time_ms);
}
/* Private functions ---------------------------------------------------------*/

/* End of stm32_dwt.c -----------------------------------------------------------------------*/
