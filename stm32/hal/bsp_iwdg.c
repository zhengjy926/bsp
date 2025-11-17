/**
  ******************************************************************************
  * @file        : bsp_iwdg.c
  * @author      : ZJY
  * @version     : V1.0
  * @date        : 20xx-xx-xx
  * @brief       : 
  * @attention   : None
  * @date        : 2025-10-16
  * @brief       : STM32 IWDG driver implementation
  * @attention   : None
  ******************************************************************************
  * @history     :
  *         V1.0 : 1.Initial version
  *
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "bsp_iwdg.h"

#include "bsp_iwdg.h"
#include "bsp_conf.h"
#include "watchdog.h"
/* Private typedef -----------------------------------------------------------*/


/* Private define ------------------------------------------------------------*/


/* Private macro -------------------------------------------------------------*/


/* Private variables ---------------------------------------------------------*/


/* Exported variables  -------------------------------------------------------*/


/* Private function prototypes -----------------------------------------------*/


/* Exported functions --------------------------------------------------------*/


int bsp_iwdg_init(void)
{
    stm32_wdt.hiwdg.Instance = IWDG;
    stm32_wdt.hiwdg.Init.Prescaler = IWDG_PRESCALER_256;
    stm32_wdt.hiwdg.Init.Reload = 0xFFF;
#if defined STM32G474xx
    stm32_wdt.hiwdg.Init.Window = 0xFFF;
#endif
    
    stm32_wdt.is_start = 0;

    ops.init = &wdt_init;
    ops.control = &wdt_control;
    stm32_wdt.watchdog.ops = &ops;
    
    /* register watchdog device */
    if (hw_watchdog_register(&stm32_wdt.watchdog, "wdt", DEVICE_AFLAG_UNINITIALIZED, NULL) != 0)
    {
//        LOG_E("wdt device register failed.");
        return -ERROR;
    }
//    LOG_D("wdt device register success.");
    return 0;
}
/* Private functions ---------------------------------------------------------*/





