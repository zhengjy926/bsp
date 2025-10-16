/**
  ******************************************************************************
  * @file        : xxxx.c
  * @author      : ZJY
  * @version     : V1.0
  * @date        : 20xx-xx-xx
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
#include "bsp_tim.h"
#include "bsp_conf.h"


/* Private typedef -----------------------------------------------------------*/
TIM_HandleTypeDef htim4;
TIM_HandleTypeDef htim6;
TIM_HandleTypeDef htim7;
TIM_HandleTypeDef htim15;

/* Private define ------------------------------------------------------------*/
#ifndef TIM16_WIDTH
#define TIM16_WIDTH        16u
#endif

#define TIM_LIMIT_PLUS1    (1u << TIM16_WIDTH)           /* 65536 */
#define TIM_PSC_MAX_PLUS1  TIM_LIMIT_PLUS1
#define TIM_ARR_MAX_PLUS1  TIM_LIMIT_PLUS1

/* Private macro -------------------------------------------------------------*/


/* Private variables ---------------------------------------------------------*/


/* Exported variables  -------------------------------------------------------*/


/* Private function prototypes -----------------------------------------------*/


/* Exported functions --------------------------------------------------------*/
void MX_TIM4_Init(void)
{
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    TIM_IC_InitTypeDef sConfigIC = {0};
    TIM_OC_InitTypeDef sConfigOC = {0};

    htim4.Instance = TIM4;
    htim4.Init.Prescaler = 1699;    /* 100KHz， 10us */
    htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim4.Init.Period = 65535;
    htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
    {
        Error_Handler();
    }
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
    {
        Error_Handler();
    }
    if (HAL_TIM_IC_Init(&htim4) != HAL_OK)
    {
        Error_Handler();
    }
    if (HAL_TIM_OC_Init(&htim4) != HAL_OK)
    {
        Error_Handler();
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
    {
        Error_Handler();
    }
    sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_FALLING;
    sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
    sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
    sConfigIC.ICFilter = 10;
    if (HAL_TIM_IC_ConfigChannel(&htim4, &sConfigIC, TIM_CHANNEL_1) != HAL_OK)
    {
        Error_Handler();
    }
    sConfigOC.OCMode = TIM_OCMODE_TIMING;
    sConfigOC.Pulse = 0;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    if (HAL_TIM_OC_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
    {
        Error_Handler();
    }
    
    HAL_TIM_IC_Start_IT(&htim4, TIM_CHANNEL_1);
    __HAL_TIM_DISABLE_IT(&htim4, TIM_IT_CC1);
    
}

void MX_TIM6_Init(void)
{
    TIM_MasterConfigTypeDef sMasterConfig = {0};

    htim6.Instance = TIM6;
    htim6.Init.Prescaler = 3;
    htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim6.Init.Period = 42499;
    htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
    {
        Error_Handler();
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
    {
        Error_Handler();
    }
    
    __HAL_TIM_CLEAR_FLAG(&htim6, TIM_IT_UPDATE);
    __HAL_TIM_ENABLE_IT(&htim6, TIM_IT_UPDATE);
}

void MX_TIM7_Init(void)
{
    TIM_MasterConfigTypeDef sMasterConfig = {0};

    htim7.Instance = TIM7;
    htim7.Init.Prescaler = 3;
    htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim7.Init.Period = 42499;
    htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim7) != HAL_OK)
    {
        Error_Handler();
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig) != HAL_OK)
    {
        Error_Handler();
    }
    
    __HAL_TIM_CLEAR_FLAG(&htim7, TIM_IT_UPDATE);
    __HAL_TIM_ENABLE_IT(&htim7, TIM_IT_UPDATE);
}

void MX_TIM15_Init(void)
{
    TIM_MasterConfigTypeDef sMasterConfig = {0};

    htim15.Instance = TIM15;
    htim15.Init.Prescaler = 3;
    htim15.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim15.Init.Period = 42499;
    htim15.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim15) != HAL_OK)
    {
        Error_Handler();
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim15, &sMasterConfig) != HAL_OK)
    {
        Error_Handler();
    }
    
    __HAL_TIM_CLEAR_FLAG(&htim15, TIM_IT_UPDATE);
    __HAL_TIM_ENABLE_IT(&htim15, TIM_IT_UPDATE);
}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* tim_baseHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(tim_baseHandle->Instance==TIM4)
  {
  /* USER CODE BEGIN TIM4_MspInit 0 */

  /* USER CODE END TIM4_MspInit 0 */
    /* TIM4 clock enable */
    __HAL_RCC_TIM4_CLK_ENABLE();

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**TIM4 GPIO Configuration
    PB6     ------> TIM4_CH1
    */
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM4;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* TIM4 interrupt Init */
    HAL_NVIC_SetPriority(TIM4_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(TIM4_IRQn);
  /* USER CODE BEGIN TIM4_MspInit 1 */

  /* USER CODE END TIM4_MspInit 1 */
  }else if (tim_baseHandle->Instance==TIM6)
    {
        __HAL_RCC_TIM6_CLK_ENABLE();

        /* TIM6 interrupt Init */
        HAL_NVIC_SetPriority(TIM6_DAC_IRQn, 1, 0);
        HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);
    } else if (tim_baseHandle->Instance == TIM7) {
        __HAL_RCC_TIM7_CLK_ENABLE();
        
        /* TIM7 interrupt Init */
        HAL_NVIC_SetPriority(TIM7_DAC_IRQn, 1, 0);
        HAL_NVIC_EnableIRQ(TIM7_DAC_IRQn);
    } else if (tim_baseHandle->Instance == TIM15) {
        __HAL_RCC_TIM15_CLK_ENABLE();
        
        /* TIM15 interrupt Init */
        HAL_NVIC_SetPriority(TIM1_BRK_TIM15_IRQn, 1, 0);
        HAL_NVIC_EnableIRQ(TIM1_BRK_TIM15_IRQn);
    }
}

int tim_pick_psc_arr_from_ns(uint32_t fclk_hz,
                            uint64_t period_ns,
                            uint16_t *psc_out,
                            uint16_t *arr_out,
                            uint64_t *p_actual_ns)
{
    if (!psc_out || !arr_out || fclk_hz == 0)
        return -1;

    /* 目标总计数 N = round(T * Fclk) */
    uint64_t N = udiv_round_u64(period_ns * (uint64_t)fclk_hz, 1000000000ull);
    if (N < 1u) N = 1u;

    const uint64_t MAX_TICKS = (uint64_t)TIM_PSC_MAX_PLUS1 * (uint64_t)TIM_ARR_MAX_PLUS1; /* 4.29e9 */

    int rc = TIM_PICK_OK;
    if (N < 1u) { N = 1u; rc = TIM_PICK_CLIPPED_MIN; }
    else if (N > MAX_TICKS) { N = MAX_TICKS; rc = TIM_PICK_CLIPPED_MAX; }

    /* 先令分频 D0 = ceil(N / 65536)，以保证 A(=ARR+1) ≤ 65536，从而使 ARR 尽量大 */
    uint64_t D0 = udiv_ceil_u64(N, TIM_ARR_MAX_PLUS1);
    if (D0 < 1u) D0 = 1u;
    if (D0 > TIM_PSC_MAX_PLUS1) D0 = TIM_PSC_MAX_PLUS1;

    /* 在 D0 附近做 ±1 的邻域优化，三次试算即可获得几乎最优的误差 */
    uint64_t best_D = 1, best_A = 1, best_err = (uint64_t)-1;
    for (int k = -1; k <= 1; ++k) {
        uint64_t D = D0;
        if (k == -1 && D0 > 1) D = D0 - 1;
        if (k == +1 && D0 < TIM_PSC_MAX_PLUS1) D = D0 + 1;

        /* A = round(N / D)，再夹到 [1..65536] */
        uint64_t A = udiv_round_u64(N, D);
        if (A < 1u) A = 1u;
        if (A > TIM_ARR_MAX_PLUS1) A = TIM_ARR_MAX_PLUS1;

        uint64_t prod = D * A;                 /* ≤ 4.29e9，32 位亦可承载，但我们用 64 位更安全 */
        uint64_t err  = uabsdiff_u64(prod, N); /* 与目标总计数的差 */

        /* 选择误差更小者；若误差相同，则偏好 A 更大（ARR 分辨率更高） */
        if ( (err < best_err) || (err == best_err && A > best_A) ) {
            best_err = err; best_D = D; best_A = A;
        }
    }

    *psc_out = (uint16_t)(best_D - 1u);
    *arr_out = (uint16_t)(best_A - 1u);

    if (p_actual_ns) {
        /* 实际周期 = (best_D * best_A) / fclk_hz（秒）-> 换算为 ns 并四舍五入 */
        uint64_t ticks = best_D * best_A;                      /* ≤ 4.29e9 */
        uint64_t ns    = udiv_round_u64(ticks * 1000000000ull, (uint64_t)fclk_hz);
        *p_actual_ns = ns;
    }
    return rc;
}

void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* tim_baseHandle)
{

  if(tim_baseHandle->Instance==TIM4)
  {
  /* USER CODE BEGIN TIM4_MspDeInit 0 */

  /* USER CODE END TIM4_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_TIM4_CLK_DISABLE();

    /**TIM4 GPIO Configuration
    PB6     ------> TIM4_CH1
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_6);

    /* TIM4 interrupt Deinit */
    HAL_NVIC_DisableIRQ(TIM4_IRQn);
  /* USER CODE BEGIN TIM4_MspDeInit 1 */

  /* USER CODE END TIM4_MspDeInit 1 */
  }
}

/**
  * @brief This function handles TIM4 global interrupt.
  */
void TIM4_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim4);
}

/**
  * @brief This function handles TIM6 global interrupt, DAC1 and DAC3 channel underrun error interrupts.
  */
void TIM6_DAC_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim6);
}

/**
  * @brief This function handles TIM7 global interrupt.
  */
void TIM7_DAC_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim7);
}

/**
  * @brief This function handles TIM15 global interrupt.
  */
void TIM1_BRK_TIM15_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim15);
}

/* Private functions ---------------------------------------------------------*/


