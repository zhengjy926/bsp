/**
  ******************************************************************************
  * @copyright   : Copyright To Hangzhou Dinova EP Technology Co.,Ltd
  * @file        : bsp_iwdg.c
  * @author      : ZJY
  * @version     : V1.0
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
#include "bsp_conf.h"
#include "watchdog.h"
/* Private typedef -----------------------------------------------------------*/
struct stm32_wdt_obj
{
    watchdog_t watchdog;
    IWDG_HandleTypeDef hiwdg;
    uint16_t is_start;
};

/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static struct stm32_wdt_obj stm32_wdt;
static struct watchdog_ops ops;

/* Exported variables  -------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
static int wdt_init(watchdog_t *wdt)
{
    return 0;
}

static int wdt_control(watchdog_t *wdt, int cmd, void *arg)
{
    switch (cmd)
    {
        /* feed the watchdog */
    case DEVICE_CTRL_WDT_REFRESH:
        if(HAL_IWDG_Refresh(&stm32_wdt.hiwdg) != HAL_OK)
        {
//            LOG_E("watch dog keepalive fail.");
        }
        break;
        /* set watchdog timeout */
    case DEVICE_CTRL_WDT_SET_TIMEOUT:
#if defined(LSI_VALUE)
        if(LSI_VALUE)
        {
            stm32_wdt.hiwdg.Init.Reload = (*((uint32_t*)arg)) * LSI_VALUE / 256 ;
        }
        else
        {
//            LOG_E("Please define the value of LSI_VALUE!");
        }
        if(stm32_wdt.hiwdg.Init.Reload > 0xFFF)
        {
//            LOG_E("wdg set timeout parameter too large, please less than %ds",0xFFF * 256 / LSI_VALUE);
            return -1;
        }
#else
  #error "Please define the value of LSI_VALUE!"
#endif
        if(stm32_wdt.is_start)
        {
            if (HAL_IWDG_Init(&stm32_wdt.hiwdg) != HAL_OK)
            {
//                LOG_E("wdg set timeout failed.");
                return -ERROR;
            }
        }
        break;
    case DEVICE_CTRL_WDT_GET_TIMEOUT:
#if defined(LSI_VALUE)
        if(LSI_VALUE)
        {
            (*((uint32_t*)arg)) = stm32_wdt.hiwdg.Init.Reload * 256 / LSI_VALUE;
        }
        else
        {
//            LOG_E("Please define the value of LSI_VALUE!");
        }
#else
  #error "Please define the value of LSI_VALUE!"
#endif
        break;
    case DEVICE_CTRL_WDT_START:
        if (HAL_IWDG_Init(&stm32_wdt.hiwdg) != HAL_OK)
        {
//            LOG_E("wdt start failed.");
            return -ERROR;
        }
        stm32_wdt.is_start = 1;
        break;
    default:
//        LOG_W("This command is not supported.");
        return -ERROR;
    }
    return 0;
}

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


