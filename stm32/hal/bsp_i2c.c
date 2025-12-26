/**
 ******************************************************************************
 * @file        : bsp_i2c.c
 * @author      : ZJY
 * @version     : V1.0
 * @date        : 2025-01-XX
 * @brief       : STM32 I2C BSP驱动实现 (HAL库实现)
 * @attention   : None
 ******************************************************************************
 * @history     :
 *         V1.0 : 1. Refactored to use HAL library
 *                2. Support I2C framework interface
 *                3. Support 7-bit and 10-bit addressing
 *                4. Support multi-message transfers
 *
 ******************************************************************************
 */
/* Includes ------------------------------------------------------------------*/
#include "bsp_i2c.h"
#include "i2c.h"
#include "errno-base.h"
#include "bsp_conf.h"
#include "stm32f1xx_hal.h"

/* FreeRTOS support */
#if defined(USING_FREERTOS) || defined(configUSE_MUTEXES)
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#endif

#define  LOG_TAG             "bsp_i2c"
#define  LOG_LVL             4
#include "log.h"

/* Private typedef -----------------------------------------------------------*/

#if defined(BSP_USING_I2C1) || defined(BSP_USING_I2C2)

/**
 * @brief STM32 I2C hardware data structure
 */
struct stm32_i2c_hw {
    I2C_HandleTypeDef hi2c;            /**< HAL I2C handle */
    const char *name;                  /**< Adapter name */
};

enum
{
#ifdef BSP_USING_I2C1
    I2C1_INDEX,
#endif
#ifdef BSP_USING_I2C2
    I2C2_INDEX,
#endif
    I2C_INDEX_MAX,
};

/* Private define ------------------------------------------------------------*/
#define I2C_TIMEOUT_MS                   (1000U)    /* Default timeout 1 second */

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* I2C hardware data array */
static struct stm32_i2c_hw stm32_i2c_hw[I2C_INDEX_MAX];

/* I2C adapter array */
static struct i2c_adapter stm32_i2c_adapter[I2C_INDEX_MAX];

/* Private function prototypes -----------------------------------------------*/
static int stm32_i2c_xfer(struct i2c_adapter *adap, struct i2c_msg *msgs, uint32_t num);

/* Exported functions --------------------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

/**
 * @brief HAL I2C MSP Initialization
 * @param hi2c I2C handle pointer
 * @note This function overrides the weak HAL_I2C_MspInit function
 */
void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c)
{
    GPIO_InitTypeDef gpio_init = {0};
    
    if (hi2c == NULL) {
        return;
    }
    
#ifdef BSP_USING_I2C1
    if (hi2c->Instance == I2C1) {
        /* Enable I2C1 and GPIOB clocks */
        __HAL_RCC_I2C1_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();
        
        /* Configure I2C1 SCL pin */
        gpio_init.Pin = BSP_I2C1_SCL_PIN;
        gpio_init.Mode = GPIO_MODE_AF_OD;
        gpio_init.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(BSP_I2C1_SCL_PORT, &gpio_init);
        
        /* Configure I2C1 SDA pin */
        gpio_init.Pin = BSP_I2C1_SDA_PIN;
        HAL_GPIO_Init(BSP_I2C1_SDA_PORT, &gpio_init);
    }
#endif /* BSP_USING_I2C1 */
    
#ifdef BSP_USING_I2C2
    if (hi2c->Instance == I2C2) {
        /* Enable I2C2 and GPIOB clocks */
        __HAL_RCC_I2C2_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();
        
        /* Configure I2C2 SCL pin */
        gpio_init.Pin = BSP_I2C2_SCL_PIN;
        gpio_init.Mode = GPIO_MODE_AF_OD;
        gpio_init.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(BSP_I2C2_SCL_PORT, &gpio_init);
        
        /* Configure I2C2 SDA pin */
        gpio_init.Pin = BSP_I2C2_SDA_PIN;
        HAL_GPIO_Init(BSP_I2C2_SDA_PORT, &gpio_init);
    }
#endif /* BSP_USING_I2C2 */
}

/**
 * @brief HAL I2C MSP De-Initialization
 * @param hi2c I2C handle pointer
 * @note This function overrides the weak HAL_I2C_MspDeInit function
 */
void HAL_I2C_MspDeInit(I2C_HandleTypeDef *hi2c)
{
    if (hi2c == NULL) {
        return;
    }
    
#ifdef BSP_USING_I2C1
    if (hi2c->Instance == I2C1) {
        /* Disable I2C1 */
        __HAL_RCC_I2C1_CLK_DISABLE();
        
        /* Deinitialize GPIO */
        HAL_GPIO_DeInit(BSP_I2C1_SCL_PORT, BSP_I2C1_SCL_PIN);
        HAL_GPIO_DeInit(BSP_I2C1_SDA_PORT, BSP_I2C1_SDA_PIN);
    }
#endif /* BSP_USING_I2C1 */
    
#ifdef BSP_USING_I2C2
    if (hi2c->Instance == I2C2) {
        /* Disable I2C2 */
        __HAL_RCC_I2C2_CLK_DISABLE();
        
        /* Deinitialize GPIO */
        HAL_GPIO_DeInit(BSP_I2C2_SCL_PORT, BSP_I2C2_SCL_PIN);
        HAL_GPIO_DeInit(BSP_I2C2_SDA_PORT, BSP_I2C2_SDA_PIN);
    }
#endif /* BSP_USING_I2C2 */
}

/**
 * @brief Execute I2C transfer using HAL library
 * @param adap Adapter pointer
 * @param msgs Messages array
 * @param num Number of messages
 * @return Number of messages transferred on success, error code on failure
 */
static int stm32_i2c_xfer(struct i2c_adapter *adap, struct i2c_msg *msgs, uint32_t num)
{
    struct stm32_i2c_hw *hw = NULL;
    I2C_HandleTypeDef *hi2c = NULL;
    uint32_t i = 0U;
    HAL_StatusTypeDef hal_status = HAL_OK;
    uint32_t timeout_ms = 0U;
    uint16_t dev_addr = 0U;
    uint16_t flags = 0U;
    
    if ((adap == NULL) || (msgs == NULL) || (num == 0U)) {
        return -EINVAL;
    }
    
    hw = (struct stm32_i2c_hw *)adap->algo_data;
    if (hw == NULL) {
        return -EINVAL;
    }
    
    hi2c = &hw->hi2c;
    timeout_ms = adap->timeout_ms;
    if (timeout_ms == 0U) {
        timeout_ms = I2C_TIMEOUT_MS;
    }
    
    /* Process each message */
    for (i = 0U; i < num; i++) {
        dev_addr = msgs[i].addr;
        flags = msgs[i].flags;
        
        /* Check if we need to send STOP after this message */
        /* For multi-message transfers, HAL library handles repeated START automatically */
        /* But we need to handle STOP flag explicitly */
        
        /* Configure addressing mode for this transfer if needed */
        /* Note: HAL library requires AddressingMode to be set before Init */
        /* For efficiency, we only re-init if addressing mode changes */
        uint32_t new_addressing_mode = ((flags & I2C_M_TEN) != 0U) ? 
                                       I2C_ADDRESSINGMODE_10BIT : I2C_ADDRESSINGMODE_7BIT;
        
        if (hi2c->Init.AddressingMode != new_addressing_mode) {
            hi2c->Init.AddressingMode = new_addressing_mode;
            /* Re-initialize I2C with new addressing mode */
            hal_status = HAL_I2C_Init(hi2c);
            if (hal_status != HAL_OK) {
                return -EIO;
            }
        }
        
        /* Perform transfer */
        if ((msgs[i].flags & I2C_M_RD) != 0U) {
            /* Read operation */
            /* HAL library handles address format automatically based on AddressingMode */
            hal_status = HAL_I2C_Master_Receive(hi2c, dev_addr<<1, msgs[i].buf, msgs[i].len, timeout_ms);
        } else {
            /* Write operation */
            /* HAL library handles address format automatically based on AddressingMode */
            hal_status = HAL_I2C_Master_Transmit(hi2c, dev_addr<<1, msgs[i].buf, msgs[i].len, timeout_ms);
        }
        
        /* Check HAL status */
        if (hal_status != HAL_OK) {
            /* Convert HAL error to framework error */
            if (hal_status == HAL_TIMEOUT) {
                return -ETIMEOUT;
            } else if (hal_status == HAL_ERROR) {
                /* Check specific error code */
                if ((hi2c->ErrorCode & HAL_I2C_ERROR_AF) != 0U) {
                    return -EIO;  /* NACK error */
                } else if ((hi2c->ErrorCode & HAL_I2C_ERROR_BERR) != 0U) {
                    return -EIO;  /* Bus error */
                } else if ((hi2c->ErrorCode & HAL_I2C_ERROR_ARLO) != 0U) {
                    return -EIO;  /* Arbitration lost */
                } else if ((hi2c->ErrorCode & HAL_I2C_ERROR_OVR) != 0U) {
                    return -EIO;  /* Overrun error */
                } else {
                    return -EIO;
                }
            } else {
                return -EIO;
            }
        }
        
        /* If STOP flag is set or this is the last message, HAL already sent STOP */
        /* For multi-message transfers without STOP flag, HAL uses repeated START */
        if (((flags & I2C_M_STOP) != 0U) || (i == (num - 1U))) {
            /* STOP already sent by HAL */
        } else {
            /* For next message, HAL will use repeated START automatically */
            /* No additional action needed */
        }
    }
    
    return (int)num;
}

/**
 * @brief I2C algorithm structure
 */
static const struct i2c_algorithm stm32_i2c_algorithm = {
    .xfer = stm32_i2c_xfer,
};

/**
 * @brief Initialize STM32 I2C BSP driver
 * @return 0 on success, error code on failure
 */
int bsp_i2c_init(void)
{
    int ret = 0;
    uint8_t i = 0U;
    struct stm32_i2c_hw *hw = NULL;
    struct i2c_adapter *adap = NULL;
    HAL_StatusTypeDef hal_status = HAL_OK;
    uint32_t speed_hz = I2C_MAX_STANDARD_MODE_FREQ;
    
    /* Initialize each I2C adapter */
    for (i = 0U; i < I2C_INDEX_MAX; i++) {
        hw = &stm32_i2c_hw[i];
        adap = &stm32_i2c_adapter[i];
        
        /* Initialize HAL handle structure */
        (void)memset(hw, 0, sizeof(struct stm32_i2c_hw));
        
#ifdef BSP_USING_I2C1
        if (i == I2C1_INDEX) {
            hw->hi2c.Instance = I2C1;
            hw->name = "i2c1";
        }
#endif
#ifdef BSP_USING_I2C2
        if (i == I2C2_INDEX) {
            hw->hi2c.Instance = I2C2;
            hw->name = "i2c2";
        }
#endif
        
        /* Configure I2C initialization structure */
        hw->hi2c.Init.ClockSpeed = speed_hz;
        hw->hi2c.Init.DutyCycle = I2C_DUTYCYCLE_2;
        hw->hi2c.Init.OwnAddress1 = 0U;
        hw->hi2c.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
        hw->hi2c.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
        hw->hi2c.Init.OwnAddress2 = 0U;
        hw->hi2c.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
        hw->hi2c.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
        
        /* Initialize I2C peripheral using HAL */
        hal_status = HAL_I2C_Init(&hw->hi2c);
        if (hal_status != HAL_OK) {
            LOG_E("Failed to initialize %s: HAL status %d", hw->name, hal_status);
            continue;
        }
        
        /* Set algorithm data */
        adap->algo_data = hw;
        
        /* Register adapter */
        ret = i2c_add_adapter(adap, hw->name, &stm32_i2c_algorithm);
        if (ret != 0) {
            LOG_E("Failed to register %s adapter", hw->name);
            continue;
        }
        
        /* Set default configuration */
        adap->speed_hz = speed_hz;
        adap->timeout_ms = I2C_TIMEOUT_MS;
        adap->addr_width = 7U;  /* Default 7-bit */
        adap->retries = 3U;     /* Default retries */
        
        LOG_I("I2C adapter %s registered, speed: %lu Hz", hw->name, speed_hz);
    }
    
    return 0;
}

#endif /* BSP_USING_I2C1 || BSP_USING_I2C2 */
