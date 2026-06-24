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
 *
 *
 ******************************************************************************
 */
/* Includes ------------------------------------------------------------------*/
#include "bsp_i2c.h"
#include "i2c.h"
#include "bsp_conf.h"
#include "stm32f1xx_hal_i2c.h"
#include "errno-base.h"

/* FreeRTOS support */
#if defined(USING_FREERTOS) || defined(configUSE_MUTEXES)
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#endif

#define  LOG_TAG             "bsp_i2c"
#define  LOG_LVL             3
#include "log.h"

/* Private typedef -----------------------------------------------------------*/
/**
 * @brief STM32 I2C sequential transfer context
 * @note  One context per hardware instance, used to implement
 *        Linux-style combined transactions with HAL sequential API.
 */
struct stm32_i2c_seq_ctx {
    struct i2c_msg       *msgs;       /**< Pointer to message array */
    uint16_t              num;        /**< Number of messages in array */
    uint16_t              idx;        /**< Index of current message */
    volatile uint8_t      active;     /**< Context in use flag */
    volatile uint8_t      done;       /**< Transfer finished flag */
    volatile HAL_StatusTypeDef result;/**< Result of last HAL operation */
};

/**
 * @brief STM32 I2C hardware data structure
 */
struct stm32_i2c_hw {
    I2C_HandleTypeDef      hi2c;      /**< HAL I2C handle */
    struct stm32_i2c_seq_ctx seq;     /**< Sequential transfer context */
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

#ifndef I2C_TIMEOUT_MS
    #define I2C_TIMEOUT_MS  (50U)
#endif

/* Private macro -------------------------------------------------------------*/



/* Private variables ---------------------------------------------------------*/



/* Exported variables  -------------------------------------------------------*/



/* Private function prototypes -----------------------------------------------*/



/* Exported functions --------------------------------------------------------*/
/**
 * @brief HAL I2C MSP Initialization
 * @param hi2c I2C handle pointer
 * @note This function overrides the weak HAL_I2C_MspInit function
 */
void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c)
{
    GPIO_InitTypeDef gpio_init = {0};
    
#ifdef BSP_USING_I2C1
    if (hi2c->Instance == I2C1) {
        /* I2C1 GPIO Configuration */
        gpio_init.Pin = BSP_I2C1_SCL_PIN;
        gpio_init.Mode = GPIO_MODE_AF_OD;
        gpio_init.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(BSP_I2C1_SCL_PORT, &gpio_init);
        
        gpio_init.Pin = BSP_I2C1_SDA_PIN;
        HAL_GPIO_Init(BSP_I2C1_SDA_PORT, &gpio_init);
        
        /* I2C1 clock enable */
        __HAL_RCC_I2C1_CLK_ENABLE();

        /* I2C1 interrupt Init: event and error */
        HAL_NVIC_SetPriority(I2C1_EV_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(I2C1_EV_IRQn);
        HAL_NVIC_SetPriority(I2C1_ER_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(I2C1_ER_IRQn);
    }
#endif /* BSP_USING_I2C1 */
    
#ifdef BSP_USING_I2C2
    if (hi2c->Instance == I2C2) {
        /* I2C2 GPIO Configuration */
        gpio_init.Pin = BSP_I2C2_SCL_PIN;
        gpio_init.Mode = GPIO_MODE_AF_OD;
        gpio_init.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(BSP_I2C2_SCL_PORT, &gpio_init);
        
        gpio_init.Pin = BSP_I2C2_SDA_PIN;
        HAL_GPIO_Init(BSP_I2C2_SDA_PORT, &gpio_init);
        
        /* I2C1 clock enable */
        __HAL_RCC_I2C2_CLK_ENABLE();

        /* I2C2 interrupt Init: event and error */
        HAL_NVIC_SetPriority(I2C2_EV_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(I2C2_EV_IRQn);
        HAL_NVIC_SetPriority(I2C2_ER_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(I2C2_ER_IRQn);
    }
#endif /* BSP_USING_I2C2 */
}

void HAL_I2C_MspDeInit(I2C_HandleTypeDef* i2cHandle)
{
#ifdef BSP_USING_I2C1
    if(i2cHandle->Instance == I2C1)
    {
        __HAL_RCC_I2C1_CLK_DISABLE();
        HAL_GPIO_DeInit(BSP_I2C1_SCL_PORT, BSP_I2C1_SCL_PIN);
        HAL_GPIO_DeInit(BSP_I2C1_SDA_PORT, BSP_I2C1_SDA_PIN);
        /* I2C1 interrupt Deinit */
        HAL_NVIC_DisableIRQ(I2C1_ER_IRQn);
    }
#endif /* BSP_USING_I2C1 */
    
#ifdef BSP_USING_I2C2
    if(i2cHandle->Instance == I2C2)
    {
        __HAL_RCC_I2C2_CLK_DISABLE();
        HAL_GPIO_DeInit(BSP_I2C2_SCL_PORT, BSP_I2C2_SCL_PIN);
        HAL_GPIO_DeInit(BSP_I2C2_SDA_PORT, BSP_I2C2_SDA_PIN);
        /* I2C2 interrupt Deinit */
        HAL_NVIC_DisableIRQ(I2C2_ER_IRQn);
    }
#endif /* BSP_USING_I2C2 */
}

/**
 * @brief Execute I2C transfer using HAL library (sequential, combined)
 * @param adap Adapter pointer
 * @param msgs Messages array
 * @param num Number of messages
 * @return Number of messages transferred on success, error code on failure
 */
static int stm32_i2c_master_xfer(struct i2c_adapter *adap, struct i2c_msg *msgs, uint16_t num)
{
    struct stm32_i2c_hw *hw = NULL;
    I2C_HandleTypeDef *hi2c = NULL;
    struct i2c_msg *msg = NULL;
    uint32_t new_addressing_mode;
    uint16_t dev_addr;
    uint32_t xfer_opt;
    HAL_StatusTypeDef hal_status;

    if ((adap == NULL) || (msgs == NULL) || (num == 0U)) {
        return -EINVAL;
    }

    hw = (struct stm32_i2c_hw*)adap->hw_data;
    if (hw == NULL) {
        LOG_E("adap->hw_data is NULL!");
        return -EINVAL;
    }

    hi2c = &hw->hi2c;

    /* Initialize sequential context */
    hw->seq.msgs   = msgs;
    hw->seq.num    = num;
    hw->seq.idx    = 0U;
    hw->seq.done   = 0U;
    hw->seq.result = HAL_OK;
    hw->seq.active = 1U;

    /* Configure first message */
    msg = &msgs[0];
    new_addressing_mode = ((msg->flags & I2C_M_TEN) != 0U) ?
                           I2C_ADDRESSINGMODE_10BIT : I2C_ADDRESSINGMODE_7BIT;
    if (hi2c->Init.AddressingMode != new_addressing_mode) {
        hi2c->Init.AddressingMode = new_addressing_mode;
    }

    dev_addr = ((msg->flags & I2C_M_TEN) != 0U) ?
               msg->addr : (uint16_t)(msg->addr << 1U);

    if (num == 1U) {
        xfer_opt = I2C_FIRST_AND_LAST_FRAME;
    } else {
        xfer_opt = I2C_FIRST_FRAME;
    }

    if ((msg->flags & I2C_M_RD) != 0U) {
        hal_status = HAL_I2C_Master_Seq_Receive_IT(hi2c,
                                                   dev_addr,
                                                   msg->buf,
                                                   msg->len,
                                                   xfer_opt);
    } else {
        hal_status = HAL_I2C_Master_Seq_Transmit_IT(hi2c,
                                                    dev_addr,
                                                    msg->buf,
                                                    msg->len,
                                                    xfer_opt);
    }

    if (hal_status != HAL_OK) {
        hw->seq.active = 0U;
        LOG_E("I2C seq start failed, status=%d", (int)hal_status);
        return -EIO;
    }

    /* Wait for sequential transfer completion driven by callbacks */
    while (hw->seq.done == 0U) {
#if defined(USING_FREERTOS) || defined(configUSE_MUTEXES)
        taskYIELD();
#endif
    }

    hw->seq.active = 0U;
    
    if (hi2c->ErrorCode != HAL_I2C_ERROR_NONE) {
        if (hi2c->ErrorCode & HAL_I2C_ERROR_ARLO || hi2c->ErrorCode & HAL_I2C_ERROR_TIMEOUT) {
            return -EAGAIN;
        }
    }
    
    if (hw->seq.result != HAL_OK) {
        LOG_E("I2C sequential transfer failed, status=%d", (int)hw->seq.result);
        return -EIO;
    }

    return (int)num;
}

/**
 * @brief Prepare I2C1 for bus recovery: De-Init I2C, then init SCL/SDA as GPIO output OD (High)
 */
static void stm32_i2c_prepare_recovery(struct i2c_adapter *adap)
{
    struct stm32_i2c_hw *hw;

    hw = (struct stm32_i2c_hw *)adap->hw_data;
    if (hw == NULL) {
        return;
    }

    (void)HAL_I2C_DeInit(&hw->hi2c);
}

/**
 * @brief Unprepare I2C1 after recovery: Re-Init I2C (pins back to AF_OD, peripheral configured)
 */
static void stm32_i2c_unprepare_recovery(struct i2c_adapter *adap)
{
    struct stm32_i2c_hw *hw;
    
    hw = (struct stm32_i2c_hw *)adap->hw_data;
    if (hw == NULL) {
        return;
    }

    (void)HAL_I2C_Init(&hw->hi2c);
}

/**
 * @brief I2C algorithm structure
 */
static const struct i2c_algo stm32_i2c_algo = {
    .master_xfer = stm32_i2c_master_xfer,
};

/* I2C hardware data array */
static struct stm32_i2c_hw stm32_i2c_hw[I2C_INDEX_MAX] = {
#ifdef BSP_USING_I2C1
    {
        .hi2c.Instance = I2C1,
    },
#endif
#ifdef BSP_USING_I2C2
    {
        .hi2c.Instance = I2C2,
    },
#endif
};

/* I2C adapter array */
static i2c_adapter_t stm32_i2c_adapter[I2C_INDEX_MAX] = {
#ifdef BSP_USING_I2C1
    {
        .name = "i2c1",
    },
#endif
#ifdef BSP_USING_I2C2
    {
        .name = "i2c2",
    },
#endif
};

static struct i2c_bus_recovery_info stm32_i2c_recovery[I2C_INDEX_MAX] = {
#ifdef BSP_USING_I2C1
    {
        .prepare_recovery   = stm32_i2c_prepare_recovery,
        .unprepare_recovery = stm32_i2c_unprepare_recovery,
        .scl_pin_id         = BSP_I2C1_SCL_PIN_ID,
        .sda_pin_id         = BSP_I2C1_SDA_PIN_ID,
    },
#endif
#ifdef BSP_USING_I2C2
    {
        .prepare_recovery   = stm32_i2c_prepare_recovery,
        .unprepare_recovery = stm32_i2c_unprepare_recovery,
        .scl_pin_id         = BSP_I2C2_SCL_PIN_ID,
        .sda_pin_id         = BSP_I2C2_SDA_PIN_ID,
    },
#endif    
};

/* Private functions ---------------------------------------------------------*/

int bsp_i2c_init(void)
{
    int ret = 0;
    struct stm32_i2c_hw *hw = NULL;
    struct i2c_adapter *adap = NULL;
    HAL_StatusTypeDef hal_status = HAL_OK;
    
    for (uint32_t i = 0; i < I2C_INDEX_MAX; i++)
    {
        hw = &stm32_i2c_hw[i];
        /* Initialize sequential context */
        hw->seq.msgs   = NULL;
        hw->seq.num    = 0U;
        hw->seq.idx    = 0U;
        hw->seq.done   = 0U;
        hw->seq.active = 0U;
        hw->seq.result = HAL_OK;
        
        adap = &stm32_i2c_adapter[i];
        adap->algo    = &stm32_i2c_algo;
        adap->hw_data = &stm32_i2c_hw[i];
        adap->timeout = I2C_TIMEOUT_MS;
        adap->retries = 3;
        adap->bus_recovery_info = &stm32_i2c_recovery[i];
        
        /* Recovery bus */
        i2c_recovery_bus(adap);

        /* Configure I2C initialization structure */
        hw->hi2c.Init.ClockSpeed      = I2C_MAX_STANDARD_MODE_FREQ;
        hw->hi2c.Init.DutyCycle       = I2C_DUTYCYCLE_16_9;
        hw->hi2c.Init.OwnAddress1     = 0U;
        hw->hi2c.Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
        hw->hi2c.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
        hw->hi2c.Init.OwnAddress2     = 0U;
        hw->hi2c.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
        hw->hi2c.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;
        
        hal_status = HAL_I2C_Init(&hw->hi2c);
        if (hal_status != HAL_OK) {
            LOG_E("Failed to initialize: HAL status %d", hal_status);
            return -EIO;
        }
#ifdef I2C_FLTR_DNF
        /** Configure Analogue filter
        */
        if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
        {
            Error_Handler();
        }

        /** Configure Digital filter
        */
        if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
        {
            Error_Handler();
        }
#endif
        ret = i2c_register_adapter(adap);
        if (ret != 0) {
            LOG_E("Failed to register %s adapter", adap->name);
            return -EIO;
        }
        
        LOG_I("I2C adapter %s registered, speed: %lu Hz", adap->name, hw->hi2c.Init.ClockSpeed);
    }
    
    return 0;
}

/**
  * @brief  I2C error callback.
  * @param  hi2c Pointer to a I2C_HandleTypeDef structure that contains
  *                the configuration information for the specified I2C.
  * @retval None
  */
void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
    char *name = "Unknown_I2C";
    uint32_t error_code;
    uint32_t i;

    /* Update sequential context if any */
    for (i = 0U; i < (uint32_t)I2C_INDEX_MAX; i++) {
        if (hi2c == &stm32_i2c_hw[i].hi2c) {
            if (stm32_i2c_hw[i].seq.active != 0U) {
                stm32_i2c_hw[i].seq.result = HAL_ERROR;
                stm32_i2c_hw[i].seq.done   = 1U;
            }
            break;
        }
    }
    
    if (hi2c->Instance == I2C1){
        name = "i2c1";
    } else if (hi2c->Instance == I2C2) {
        name = "i2c2";
    }
    
    error_code = hi2c->ErrorCode;
    
    // 总线错误: 起始/停止条件位置非法 -> 通常需要重新初始化 i2c
    if (error_code & HAL_I2C_ERROR_BERR) {
        LOG_E("[%s] CRITICAL: Bus Error (BERR) - Electrical noise or bad sequence", name);
    }

    // 仲裁丢失: 多主机冲突或从机意外拉低 SDA -> 需要等待并重试，
    // 若单主机仍频繁 ARLO，要怀疑从机把 SDA 拉死，必要时也要做一次总线恢复
    if (error_code & HAL_I2C_ERROR_ARLO) {
        LOG_E("[%s] Arbitration Loss (ARLO) - Bus conflict", name);
    }

    // 应答失败: 从机无响应 -> 检查地址或从机是否忙
    if (error_code & HAL_I2C_ERROR_AF) {
        LOG_W("[%s] ACK Failure (AF) - Check Slave Address or wiring", name);
    }

    // 溢出/欠载: 软件处理太慢或中断优先级低 -> 丢包
    if (error_code & HAL_I2C_ERROR_OVR) {
        LOG_E("[%s] Overrun/Underrun (OVR) - Data lost, check ISR priority", name);
    }

    // DMA 传输错误
    if (error_code & HAL_I2C_ERROR_DMA) {
        LOG_E("[%s] DMA Transfer Error - Check DMA config", name);
    }

    // 软件超时
    if (error_code & HAL_I2C_ERROR_TIMEOUT) {
        LOG_E("[%s] Hardware Timeout - Bus likely stuck", name);
    }
}


#ifdef BSP_USING_I2C1
/**
  * @brief This function handles I2C1 event interrupt.
  */
void I2C1_EV_IRQHandler(void)
{
    HAL_I2C_EV_IRQHandler(&stm32_i2c_hw[I2C1_INDEX].hi2c);
}

/**
  * @brief This function handles I2C1 error interrupt.
  */
void I2C1_ER_IRQHandler(void)
{
    HAL_I2C_ER_IRQHandler(&stm32_i2c_hw[I2C1_INDEX].hi2c);
}
#endif

#ifdef BSP_USING_I2C2
/**
  * @brief This function handles I2C1 event interrupt.
  */
void I2C2_EV_IRQHandler(void)
{
    HAL_I2C_EV_IRQHandler(&stm32_i2c_hw[I2C1_INDEX].hi2c);
}

/**
  * @brief This function handles I2C2 error interrupt.
  */
void I2C2_ER_IRQHandler(void)
{
    HAL_I2C_ER_IRQHandler(&stm32_i2c_hw[I2C2_INDEX].hi2c);
}
#endif

/**
  * @brief  Master Tx Transfer completed callback.
  * @param  hi2c Pointer to a I2C handle structure that contains
  *         the configuration information for I2C module.
  * @retval None
  */
void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    uint32_t i;

    for (i = 0U; i < (uint32_t)I2C_INDEX_MAX; i++) {
        struct stm32_i2c_hw *hw = &stm32_i2c_hw[i];

        if (hi2c == &hw->hi2c) {
            if ((hw->seq.active != 0U) && (hw->seq.done == 0U)) {
                struct i2c_msg *msg;
                uint32_t mode;
                uint16_t dev_addr;
                uint32_t opt;
                HAL_StatusTypeDef status;

                hw->seq.idx++;
                if (hw->seq.idx >= hw->seq.num) {
                    hw->seq.result = HAL_OK;
                    hw->seq.done   = 1U;
                    return;
                }

                msg = &hw->seq.msgs[hw->seq.idx];

                mode = ((msg->flags & I2C_M_TEN) != 0U) ?
                       I2C_ADDRESSINGMODE_10BIT : I2C_ADDRESSINGMODE_7BIT;
                if (hi2c->Init.AddressingMode != mode) {
                    hi2c->Init.AddressingMode = mode;
                }

                dev_addr = ((msg->flags & I2C_M_TEN) != 0U) ?
                           msg->addr : (uint16_t)(msg->addr << 1U);

                if (hw->seq.idx == (uint16_t)(hw->seq.num - 1U)) {
                    opt = I2C_LAST_FRAME;
                } else {
                    opt = I2C_NEXT_FRAME;
                }

                if ((msg->flags & I2C_M_RD) != 0U) {
                    status = HAL_I2C_Master_Seq_Receive_IT(hi2c,
                                                           dev_addr,
                                                           msg->buf,
                                                           msg->len,
                                                           opt);
                } else {
                    status = HAL_I2C_Master_Seq_Transmit_IT(hi2c,
                                                            dev_addr,
                                                            msg->buf,
                                                            msg->len,
                                                            opt);
                }

                if (status != HAL_OK) {
                    hw->seq.result = status;
                    hw->seq.done   = 1U;
                }
            }
            return;
        }
    }
}

/**
  * @brief  Master Rx Transfer completed callback.
  * @param  hi2c Pointer to a I2C handle structure that contains
  *         the configuration information for I2C module.
  * @retval None
  */
void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    /* Reuse the same logic as Tx complete */
    HAL_I2C_MasterTxCpltCallback(hi2c);
}

