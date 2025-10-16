/**
  ******************************************************************************
  * @copyright   : Copyright To Hangzhou Dinova EP Technology Co.,Ltd
  * @file        : bsp_pwm.c
  * @author      : ZJY
  * @version     : V1.0
  * @date        : 2025-10-16
  * @brief       : STM32 PWM驱动实现
  * @attention   : None
  ******************************************************************************
  * @history     :
  *         V1.0 : 1.Initial version
  *
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "bsp_pwm.h"
#include "pwm.h"
#include "bsp_conf.h"
#include "mymath.h"

/* Private typedef -----------------------------------------------------------*/
/**
 * @brief STM32 PWM控制器结构体
 */
struct stm32_pwm {
    TIM_HandleTypeDef htim;         /**< STM32 HAL TIM句柄 */
    uint32_t max_arr;               /**< ARR寄存器最大值 */
    bool have_complementary_output; /**< 是否有互补输出 */
    uint32_t clock;
};

/* Private define ------------------------------------------------------------*/
#define MAX_BREAKINPUT 2

#define TIM_CH_TO_HAL_CHANNEL(ch)  ((uint32_t)((ch - 1) << 2))  // 等价于 (ch-1)*4
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Exported variables  -------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void HAL_TIM_MspPostInit(TIM_HandleTypeDef* timHandle);
static int stm32_pwm_init(struct stm32_pwm *hw_pwm);
/* Exported functions --------------------------------------------------------*/
// 判断有效电平是哪个
static uint32_t active_channels(struct stm32_pwm *dev)
{
	uint32_t temp_ccer;
    
    temp_ccer = dev->htim.Instance->CCER;

	return temp_ccer & TIM_CCER_CCxE_MASK;
}

/**
 * @brief 配置PWM参数
 * @param priv PWM控制器指针
 * @param ch 通道号
 * @param duty_ns 占空比（纳秒）
 * @param period_ns 周期（纳秒）
 * @return 成功返回0，失败返回负的错误码
 */
static int stm32_pwm_config(struct stm32_pwm *priv, uint32_t ch,
                            uint32_t duty_ns, uint32_t period_ns)
{
	uint32_t prd, dty;
	uint32_t prescaler;

	prescaler = ((uint64_t)period_ns * priv->clock) / ((uint64_t)1000000000 * ((uint64_t)priv->max_arr + 1));
	if (prescaler > 0xFFFF)
		return -EINVAL;

	prd = ((uint64_t)period_ns * priv->clock) / ((uint64_t)1000000000 * (prescaler + 1));
	if (!prd)
		return -EINVAL;

	/*
	 * All channels share the same prescaler and counter so when two
	 * channels are active at the same time we can't change them
	 */
	if (active_channels(priv) & ~(1 << ch * 4)) {
		uint32_t psc, arr;
        
        psc = priv->htim.Instance->PSC;
        arr = priv->htim.Instance->ARR;

		if ((psc != prescaler) || (arr != prd - 1))
			return -EBUSY;
	}

	__HAL_TIM_SET_PRESCALER(&priv->htim, prescaler);
    priv->htim.Instance->ARR = prd - 1;

	/* Calculate the duty cycles */
	dty = ((uint64_t)duty_ns * priv->clock) / ((uint64_t)1000000000 * (prescaler + 1));
    
    ch = TIM_CH_TO_HAL_CHANNEL(ch);
    __HAL_TIM_SET_COMPARE(&priv->htim, ch, dty);

	return 0;
}

/**
 * @brief 设置PWM极性
 * @param priv PWM控制器指针
 * @param ch 通道号
 * @param polarity 极性
 * @return 成功返回0，失败返回负的错误码
 */
static int stm32_pwm_set_polarity(struct stm32_pwm *priv, unsigned int ch,
                                  enum pwm_polarity polarity)
{
    TIM_HandleTypeDef *htim = &priv->htim;
    TIM_TypeDef *tim = htim->Instance;
    uint32_t polarity_mask;
    uint32_t ch_offset = TIM_CH_TO_HAL_CHANNEL(ch);

    // 验证通道号合法性（1~4对应CH1~CH4）
    if (ch < 1 || ch > 4) {
        return -EINVAL; // 返回无效参数错误
    }

    // 计算极性位的掩码
    polarity_mask = TIM_CCER_CC1P << ch_offset; // CCxP掩码（极性位）

    // 设置极性（修改CCxP位）
    if (polarity == PWM_POLARITY_INVERSED) {
        tim->CCER |= polarity_mask; // 极性反转（低电平有效）
    } else {
        tim->CCER &= ~polarity_mask; // 正常极性（高电平有效）
    }
    
    // 如果有互补输出，也需要设置互补通道的极性
    if (priv->have_complementary_output) {
        uint32_t npolarity_mask = TIM_CCER_CC1NP << ch_offset; // CCxNP掩码（互补极性位）
        
        if (polarity == PWM_POLARITY_INVERSED) {
            tim->CCER |= npolarity_mask; // 互补极性反转
        } else {
            tim->CCER &= ~npolarity_mask; // 互补极性正常
        }
    }

    return 0; // 成功返回
}

/**
 * @brief 使能PWM输出
 * @param chip PWM控制器
 * @param pwm PWM设备
 * @return 成功返回0，失败返回负的错误码
 */
static int stm32_pwm_enable(struct pwm_chip *chip, struct pwm_device *pwm)
{
    struct stm32_pwm *priv = (struct stm32_pwm*)chip->hw_data;
    HAL_StatusTypeDef hal_ret;
    uint32_t ch_offset = TIM_CH_TO_HAL_CHANNEL(pwm->hwpwm);
    
    if (!priv || !pwm || pwm->hwpwm < 1 || pwm->hwpwm > 4)
        return -EINVAL;
    
    hal_ret = HAL_TIM_PWM_Start(&priv->htim, ch_offset);
    
    return (hal_ret == HAL_OK) ? 0 : -EIO;
}

/**
 * @brief 禁用PWM输出
 * @param chip PWM控制器
 * @param pwm PWM设备
 * @return 成功返回0，失败返回负的错误码
 */
static int stm32_pwm_disable(struct pwm_chip *chip, struct pwm_device *pwm)
{
    struct stm32_pwm *priv = (struct stm32_pwm*)chip->hw_data;
    HAL_StatusTypeDef hal_ret;
    uint32_t ch_offset = TIM_CH_TO_HAL_CHANNEL(pwm->hwpwm);
    
    if (!priv || !pwm || pwm->hwpwm < 1 || pwm->hwpwm > 4)
        return -EINVAL;
    
    hal_ret = HAL_TIM_PWM_Stop(&priv->htim, ch_offset);
    
    return (hal_ret == HAL_OK) ? 0 : -EIO;
}

/**
 * @brief 设置PWM参数
 * @param chip PWM控制器
 * @param pwm PWM设备
 * @param state 要设置的状态
 * @return 成功返回0，失败返回负的错误码
 */
static int stm32_pwm_set(struct pwm_chip *chip, struct pwm_device *pwm,
                           const struct pwm_state *state)
{
    int ret;
	struct stm32_pwm *priv = (struct stm32_pwm*)chip->hw_data;

    if (!priv || !pwm || !state)
        return -EINVAL;

	if (state->polarity != pwm->state.polarity)
		stm32_pwm_set_polarity(priv, pwm->hwpwm, state->polarity);

	ret = stm32_pwm_config(priv, pwm->hwpwm, state->duty_cycle, state->period);
    
    if (ret)
		return ret;
    
	return 0;
}

/**
 * @brief 初始化PWM硬件
 * @param hw_pwm PWM控制器指针
 * @return 成功返回0，失败返回负的错误码
 */
static int stm32_pwm_init(struct stm32_pwm *hw_pwm)
{
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    TIM_OC_InitTypeDef sConfigOC = {0};

    HAL_StatusTypeDef hal_ret;

    if (!hw_pwm || !hw_pwm->htim.Instance)
        return -EINVAL;

    hw_pwm->htim.Init.Prescaler = 1;
    hw_pwm->htim.Init.CounterMode = TIM_COUNTERMODE_UP;
    hw_pwm->htim.Init.Period = 10000;
    hw_pwm->htim.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    hw_pwm->htim.Init.RepetitionCounter = 0;
    hw_pwm->htim.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    
    hal_ret = HAL_TIM_Base_Init(&hw_pwm->htim);
    if (hal_ret != HAL_OK)
        return -EIO;
    
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&hw_pwm->htim, &sClockSourceConfig) != HAL_OK)
    {
        return -EIO;
    }
    
    hal_ret = HAL_TIM_PWM_Init(&hw_pwm->htim);
    if (hal_ret != HAL_OK)
        return -EIO;
    
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&hw_pwm->htim, &sMasterConfig) != HAL_OK)
    {
        return -EIO;
    }
    
    // 配置默认PWM通道参数
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 2000;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    
    // 配置通道3为PWM模式
    hal_ret = HAL_TIM_PWM_ConfigChannel(&hw_pwm->htim, &sConfigOC, TIM_CHANNEL_3);
    if (hal_ret != HAL_OK)
        return -EIO;
    
    // 配置通道4为输出比较模式
    sConfigOC.OCMode = TIM_OCMODE_TIMING;
    hal_ret = HAL_TIM_OC_ConfigChannel(&hw_pwm->htim, &sConfigOC, TIM_CHANNEL_4);
    if (hal_ret != HAL_OK)
        return -EIO;
    
    // 初始化GPIO
    HAL_TIM_MspPostInit(&hw_pwm->htim);
    
    return 0;
}

/**
 * @brief TIM基本初始化回调
 * @param tim_baseHandle TIM句柄指针
 */
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* tim_baseHandle)
{
    if(tim_baseHandle->Instance==TIM1)
    {
        /* TIM1 clock enable */
        __HAL_RCC_TIM1_CLK_ENABLE();
    }
    else if(tim_baseHandle->Instance==TIM3)
    {
        /* TIM3 clock enable */
        __HAL_RCC_TIM3_CLK_ENABLE();
    }
    else if(tim_baseHandle->Instance==TIM5)
    {
        /* TIM5 clock enable */
        __HAL_RCC_TIM5_CLK_ENABLE();
    }
}

/**
 * @brief TIM MSP后初始化回调（GPIO配置）
 * @param timHandle TIM句柄指针
 */
void HAL_TIM_MspPostInit(TIM_HandleTypeDef* timHandle)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    if(timHandle->Instance == TIM1)
    {
        __HAL_RCC_GPIOE_CLK_ENABLE();
            
        /* TIM1 GPIO Configuration */
        GPIO_InitStruct.Pin = GPIO_PIN_9;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;
        HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
    }
    else if(timHandle->Instance == TIM3)
    {
        __HAL_RCC_GPIOB_CLK_ENABLE();
        
        /* TIM3 GPIO Configuration */
        GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    }
    else if(timHandle->Instance == TIM5)
    {
        __HAL_RCC_GPIOA_CLK_ENABLE();

        /* TIM5 GPIO Configuration */
        GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF2_TIM5;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    }
}

/* PWM操作函数表 */
static struct pwm_ops stm32_pwm_ops = {
    .apply     = stm32_pwm_set,
    .capture   = NULL,
};

/* stm32 pwm 硬件驱动 */
static struct stm32_pwm __stm32_pwm[] = {
    {
        .htim.Instance = TIM3,
        .max_arr = 0xFFFF,
        .have_complementary_output = false,
        .clock = 90000000,
    },
};

/* stm32 pwm 控制器定义 */
static struct pwm_chip pwm_chip[] = {
    {
        .ops = &stm32_pwm_ops,
        .id = 3,
        .npwm = 4,
        .atomic = false,
        .operational = true,
        .hw_data = &__stm32_pwm[0],
    },
};

/* stm32 pwm 设备定义 */
static struct pwm_device pwm_dev[] = {
    {
        .label = "tim3_ch3",
        .hwpwm = 3,
        .chip = &pwm_chip[0],
        .state = {
            .period = 500,
            .duty_cycle = 250,
            .polarity = PWM_POLARITY_NORMAL
        },
    },
    
    {
        .label = "tim3_ch4",
        .hwpwm = 4,
        .chip = &pwm_chip[0],
        .state = {
            .period = 500,
            .duty_cycle = 250,
            .polarity = PWM_POLARITY_NORMAL
        },
    },
};

/**
 * @brief 初始化TIM3 PWM并注册设备
 * @return 成功返回0，失败返回负的错误码
 */
int stm32_pwm_tim3_init(void)
{
    int ret;

    // 初始化PWM硬件
    ret = stm32_pwm_init(&__stm32_pwm[0]);
    if (ret)
        return ret;
    
    // 注册PWM设备
    ret = pwm_register_device(&pwm_dev[0]);
    if (ret)
        return ret;
    
    ret = pwm_register_device(&pwm_dev[1]);
    if (ret)
        return ret;
    
    return 0;
}

/**
 * @brief 初始化指定定时器的PWM
 * @param tim_num 定时器编号(1,2,3,...)
 * @return 成功返回0，失败返回负的错误码
 * 
 * 这是一个通用的初始化接口，根据定时器号初始化对应的PWM
 */
int stm32_pwm_init_timer(uint8_t tim_num)
{
    switch(tim_num) {
        case 3:
            return stm32_pwm_tim3_init();
        // 可以扩展其他定时器
        default:
            return -EINVAL;
    }
}

/* Private functions ---------------------------------------------------------*/
