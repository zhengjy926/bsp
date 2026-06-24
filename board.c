/**
  ******************************************************************************
  * @copyright   : Copyright To Hangzhou Dinova EP Technology Co.,Ltd
  * @file        : xxxx.c
  * @author      : ZJY
  * @version     : V1.0
  * @data        : 20xx-xx-xx
  * @brief       :
  * @attention   : None
  ******************************************************************************
  * @history     :
  *         V1.0 : 1.xxx
  *
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "board.h"
#include "bsp_conf.h"

#define LOG_TAG    "board"
#define LOG_LVL    ELOG_LVL_DEBUG
#include "elog.h"

#include "bsp_adc.h"
#include "bsp_dac.h"
#include "bsp_dma.h"
#include "bsp_dwt.h"
#include "bsp_gpio.h"
#include "bsp_iwdg.h"
#include "bsp_sram.h"
#include "bsp_tim.h"
#include "bsp_hwtimer.h"
#include "bsp_uart.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Exported variables  -------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);

/* Exported functions --------------------------------------------------------*/
/**
  * @brief
  * @param
  * @retval
  * @note
  */
int32_t Board_Init(void)
{
    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    /* Configure the system clock */
    SystemClock_Config();
    
    /* initialize EasyLogger */
    elog_init();
    /* set EasyLogger log format */
    elog_set_fmt(ELOG_LVL_ASSERT, ELOG_FMT_ALL);
    elog_set_fmt(ELOG_LVL_ERROR, ELOG_FMT_LVL | ELOG_FMT_TAG);
    elog_set_fmt(ELOG_LVL_WARN, ELOG_FMT_LVL | ELOG_FMT_TAG);
    elog_set_fmt(ELOG_LVL_INFO, ELOG_FMT_LVL | ELOG_FMT_TAG);
    elog_set_fmt(ELOG_LVL_DEBUG, ELOG_FMT_LVL | ELOG_FMT_TAG);
    elog_set_fmt(ELOG_LVL_VERBOSE, ELOG_FMT_LVL | ELOG_FMT_TAG);
    /* start EasyLogger */
    elog_start();
    
    log_d("SYSCLK frequency is %d!", HAL_RCC_GetSysClockFreq());

    /* 检查复位源 */
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_SFTRST)) {
        log_d("System reset by Software");
    }
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST)) {
        log_d("System reset by IWDG timeout");
    }
    __HAL_RCC_CLEAR_RESET_FLAGS();  /* 清除复位标志 */
    
    /* Initialize all configured peripherals */
    if (BSP_GPIO_Init()) {
        log_e("BSP_GPIO_Init failed!");
        Error_Handler();
    }
    
    log_d("Init success!");
    
    return 0;
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /** Configure the main internal regulator output voltage
    */
    HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST);

    /** Initializes the RCC Oscillators according to the specified parameters
    * in the RCC_OscInitTypeDef structure.
    */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.LSIState = RCC_LSI_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV2;
    RCC_OscInitStruct.PLL.PLLN = 85;
    //    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV8; // TODO:修改
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
    RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
    RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks
    */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                          |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
    {
        Error_Handler();
    }
    
    /** Enables the Clock Security System
    */
    HAL_RCC_EnableCSS();
}

/**
  * @brief  RCC Clock Security System interrupt callback.
  * @retval none
  */
void HAL_RCC_CSSCallback(void)
{
  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_RCC_CSSCallback should be implemented in the user file
   */
}

/**
  * Initializes the Global MSP.
  */
void HAL_MspInit(void)
{
#if defined(STM32F1)
    __HAL_RCC_AFIO_CLK_ENABLE();
    __HAL_AFIO_REMAP_SWJ_NOJTAG(); /* NOJTAG: JTAG-DP Disabled and SW-DP Enabled */
#else
    __HAL_RCC_SYSCFG_CLK_ENABLE();
#endif
    __HAL_RCC_PWR_CLK_ENABLE();
    
#ifdef UCPD1
    /** Disable the internal Pull-Up in Dead Battery pins of UCPD peripheral
    */
    HAL_PWREx_DisableUCPDDeadBattery();
#endif
    
    /* Enable the system handler fault */
    SET_BIT(SCB->SHCSR, SCB_SHCSR_USGFAULTENA_Msk);
    SET_BIT(SCB->SHCSR, SCB_SHCSR_BUSFAULTENA_Msk);
    SET_BIT(SCB->SHCSR, SCB_SHCSR_MEMFAULTENA_Msk);
    
    __HAL_DBGMCU_FREEZE_IWDG();
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();
    while (1)
    {

    }
}

/* Private functions ---------------------------------------------------------*/

