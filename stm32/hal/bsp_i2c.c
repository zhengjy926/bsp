/**
 ******************************************************************************
 * @file        : bsp_i2c.c
 * @author      : ZJY
 * @version     : V1.0
 * @date        : 2025-01-XX
 * @brief       : STM32 I2C BSP驱动实现 (LL库实现)
 * @attention   : None
 ******************************************************************************
 * @history     :
 *         V1.0 : 1. Complete I2C BSP driver with LL library
 *                2. Support I2C framework interface
 *                3. Support 7-bit and 10-bit addressing
 *                4. Support polling and DMA transfer modes
 *
 ******************************************************************************
 */
/* Includes ------------------------------------------------------------------*/
#include "bsp_i2c.h"
#include "i2c.h"
#include "errno-base.h"
#include "bsp_conf.h"
#include "system_stm32f1xx.h"  /* For SystemCoreClock */

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
    I2C_TypeDef *instance;          /**< I2C peripheral instance */
    IRQn_Type irq_type;             /**< I2C interrupt type */
    uint32_t pclk_freq;             /**< Peripheral clock frequency */
    uint32_t max_speed_hz;          /**< Maximum I2C speed */
    const char *name;               /**< Adapter name */
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
#define STM32_I2C_MAX_SPEED_HZ           (400000U)  /* 400 kHz - Fast mode maximum */
#define STM32_I2C_STANDARD_SPEED_HZ      (100000U)  /* 100 kHz - Standard mode maximum */
#define I2C_TIMEOUT_MS                   (1000U)    /* Default timeout 1 second */
#define I2C_TIMEOUT_LOOPS                (100000U)  /* Timeout loop counter */

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* I2C hardware data array */
static struct stm32_i2c_hw stm32_i2c_hw[I2C_INDEX_MAX] = {
#ifdef BSP_USING_I2C1
    {
        .instance = I2C1,
        .irq_type = I2C1_EV_IRQn,
        .pclk_freq = 0U,  /* Will be calculated in init */
        .max_speed_hz = STM32_I2C_MAX_SPEED_HZ,
        .name = "i2c1"
    },
#endif
#ifdef BSP_USING_I2C2
    {
        .instance = I2C2,
        .irq_type = I2C2_EV_IRQn,
        .pclk_freq = 0U,
        .max_speed_hz = STM32_I2C_MAX_SPEED_HZ,
        .name = "i2c2"
    },
#endif
};

/* I2C adapter array */
static struct i2c_adapter stm32_i2c_adapter[sizeof(stm32_i2c_hw) / sizeof(stm32_i2c_hw[0])];

/* Private function prototypes -----------------------------------------------*/
static int stm32_i2c_master_xfer(struct i2c_adapter *adap, 
                                  struct i2c_msg *msgs, 
                                  int num);
static int stm32_i2c_master_xfer_dma(struct i2c_adapter *adap,
                                     struct i2c_msg *msgs,
                                     int num);
static int stm32_i2c_gpio_init(I2C_TypeDef *i2c_instance);
static int stm32_i2c_config_speed(I2C_TypeDef *i2c_instance, 
                                  uint32_t pclk_freq, 
                                  uint32_t speed_hz,
                                  uint32_t *actual_speed);
static uint32_t stm32_i2c_get_pclk_freq(I2C_TypeDef *i2c_instance);
static int stm32_i2c_wait_flag_timeout(I2C_TypeDef *I2Cx, 
                                       uint32_t flag_mask,
                                       uint32_t timeout_ms);
static int stm32_i2c_wait_busy_timeout(I2C_TypeDef *I2Cx, uint32_t timeout_ms);
static int stm32_i2c_check_error(I2C_TypeDef *I2Cx);
static int stm32_i2c_write_7bit_addr(I2C_TypeDef *I2Cx, 
                                     struct i2c_msg *msg,
                                     uint32_t timeout_ms);
static int stm32_i2c_write_10bit_addr(I2C_TypeDef *I2Cx, 
                                      struct i2c_msg *msg,
                                      uint32_t timeout_ms);
static int stm32_i2c_write_data(I2C_TypeDef *I2Cx, 
                                const uint8_t *buf, 
                                uint16_t len,
                                uint32_t timeout_ms);
static int stm32_i2c_read_data(I2C_TypeDef *I2Cx, 
                               uint8_t *buf, 
                               uint16_t len,
                               uint32_t timeout_ms);

/* Exported functions --------------------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

/**
 * @brief Get PCLK frequency for I2C peripheral
 * @param i2c_instance I2C peripheral instance
 * @return PCLK frequency in Hz
 */
static uint32_t stm32_i2c_get_pclk_freq(I2C_TypeDef *i2c_instance)
{
    uint32_t pclk_freq = 0U;
    LL_RCC_ClocksTypeDef rcc_clocks;
    
    if (i2c_instance == NULL) {
        return 0U;
    }
    
    LL_RCC_GetSystemClocksFreq(&rcc_clocks);
    
    /* I2C is on APB1 bus */
    if (i2c_instance == I2C1) {
        pclk_freq = rcc_clocks.PCLK1_Frequency;
    }
#ifdef I2C2
    else if (i2c_instance == I2C2) {
        pclk_freq = rcc_clocks.PCLK1_Frequency;
    }
#endif
    else {
        pclk_freq = 0U;
    }
    
    return pclk_freq;
}

/**
 * @brief Wait for I2C flag with timeout
 * @param I2Cx I2C instance
 * @param flag_mask Flag mask to check
 * @param timeout_ms Timeout in milliseconds
 * @return 0 on success, -ETIMEOUT on timeout, -EIO on error
 */
static int stm32_i2c_wait_flag_timeout(I2C_TypeDef *I2Cx, 
                                       uint32_t flag_mask,
                                       uint32_t timeout_ms)
{
    int ret = 0;
    
    if (I2Cx == NULL) {
        return -EINVAL;
    }
    
    /* Use loop counter for timeout (simple and reliable) */
    {
        uint32_t loop_count = 0U;
        uint32_t max_loops = timeout_ms * (SystemCoreClock / 1000U / 100U);  /* Approximate loops */
        if (max_loops == 0U) {
            max_loops = I2C_TIMEOUT_LOOPS;
        }
        
        while ((READ_REG(I2Cx->SR1) & flag_mask) == 0U) {
            if (loop_count++ > max_loops) {
                return -ETIMEOUT;
            }
            
            ret = stm32_i2c_check_error(I2Cx);
            if (ret != 0) {
                return ret;
            }
        }
    }
    
    return 0;
}

/**
 * @brief Wait for I2C bus to be free
 * @param I2Cx I2C instance
 * @param timeout_ms Timeout in milliseconds
 * @return 0 on success, -ETIMEOUT on timeout
 */
static int stm32_i2c_wait_busy_timeout(I2C_TypeDef *I2Cx, uint32_t timeout_ms)
{
    
    if (I2Cx == NULL) {
        return -EINVAL;
    }
    
    /* Use loop counter for timeout */
    {
        uint32_t loop_count = 0U;
        uint32_t max_loops = timeout_ms * (SystemCoreClock / 1000U / 100U);  /* Approximate loops */
        if (max_loops == 0U) {
            max_loops = I2C_TIMEOUT_LOOPS;
        }
        
        while (LL_I2C_IsActiveFlag_BUSY(I2Cx) != 0U) {
            if (loop_count++ > max_loops) {
                return -ETIMEOUT;
            }
        }
    }
    
    return 0;
}

/**
 * @brief Check and clear I2C error flags
 * @param I2Cx I2C instance
 * @return 0 on success, error code on failure
 */
static int stm32_i2c_check_error(I2C_TypeDef *I2Cx)
{
    if (I2Cx == NULL) {
        return -EINVAL;
    }
    
    /* Check bus error */
    if (LL_I2C_IsActiveFlag_BERR(I2Cx) != 0U) {
        LL_I2C_ClearFlag_BERR(I2Cx);
        return -EIO;
    }
    
    /* Check arbitration lost */
    if (LL_I2C_IsActiveFlag_ARLO(I2Cx) != 0U) {
        LL_I2C_ClearFlag_ARLO(I2Cx);
        return -EIO;
    }
    
    /* Check acknowledge failure */
    if (LL_I2C_IsActiveFlag_AF(I2Cx) != 0U) {
        LL_I2C_ClearFlag_AF(I2Cx);
        return -EIO;
    }
    
    /* Check overrun */
    if (LL_I2C_IsActiveFlag_OVR(I2Cx) != 0U) {
        LL_I2C_ClearFlag_OVR(I2Cx);
        return -EIO;
    }
    
    return 0;
}

/**
 * @brief Write 7-bit address
 * @param I2Cx I2C instance
 * @param msg Message pointer
 * @param timeout_ms Timeout in milliseconds
 * @return 0 on success, error code on failure
 */
static int stm32_i2c_write_7bit_addr(I2C_TypeDef *I2Cx, 
                                     struct i2c_msg *msg,
                                     uint32_t timeout_ms)
{
    uint8_t addr_byte = 0U;
    int ret = 0;
    
    if ((I2Cx == NULL) || (msg == NULL)) {
        return -EINVAL;
    }
    
    /* Prepare address byte: 7-bit address + R/W bit */
    addr_byte = (uint8_t)((msg->addr << 1U) | ((msg->flags & I2C_M_RD) ? 1U : 0U));
    
    /* Wait for START bit flag */
    ret = stm32_i2c_wait_flag_timeout(I2Cx, LL_I2C_SR1_SB, timeout_ms);
    if (ret != 0) {
        return ret;
    }
    
    /* Send address */
    LL_I2C_TransmitData8(I2Cx, addr_byte);
    
    /* Wait for address sent flag */
    ret = stm32_i2c_wait_flag_timeout(I2Cx, LL_I2C_SR1_ADDR, timeout_ms);
    if (ret != 0) {
        return ret;
    }
    
    /* Clear ADDR flag by reading SR1 and SR2 */
    (void)READ_REG(I2Cx->SR1);
    (void)READ_REG(I2Cx->SR2);
    
    return 0;
}

/**
 * @brief Write 10-bit address
 * @param I2Cx I2C instance
 * @param msg Message pointer
 * @param timeout_ms Timeout in milliseconds
 * @return 0 on success, error code on failure
 */
static int stm32_i2c_write_10bit_addr(I2C_TypeDef *I2Cx, 
                                      struct i2c_msg *msg,
                                      uint32_t timeout_ms)
{
    uint8_t header = 0U;
    uint8_t addr_low = 0U;
    int ret = 0;
    
    if ((I2Cx == NULL) || (msg == NULL)) {
        return -EINVAL;
    }
    
    /* Wait for START bit flag */
    ret = stm32_i2c_wait_flag_timeout(I2Cx, LL_I2C_SR1_SB, timeout_ms);
    if (ret != 0) {
        return ret;
    }
    
    /* Send 10-bit address header (write) */
    header = __LL_I2C_10BIT_HEADER_WRITE(msg->addr);
    LL_I2C_TransmitData8(I2Cx, header);
    
    /* Wait for ADD10 flag */
    ret = stm32_i2c_wait_flag_timeout(I2Cx, LL_I2C_SR1_ADD10, timeout_ms);
    if (ret != 0) {
        return ret;
    }
    
    /* Clear ADD10 flag by reading SR1 and SR2 */
    (void)READ_REG(I2Cx->SR1);
    (void)READ_REG(I2Cx->SR2);
    
    /* Send address low 8 bits */
    addr_low = __LL_I2C_10BIT_ADDRESS(msg->addr);
    LL_I2C_TransmitData8(I2Cx, addr_low);
    
    /* Wait for address sent flag */
    ret = stm32_i2c_wait_flag_timeout(I2Cx, LL_I2C_SR1_ADDR, timeout_ms);
    if (ret != 0) {
        return ret;
    }
    
    /* Clear ADDR flag */
    (void)READ_REG(I2Cx->SR1);
    (void)READ_REG(I2Cx->SR2);
    
    /* If read operation, need repeated START */
    if ((msg->flags & I2C_M_RD) != 0U) {
        /* Generate repeated START */
        LL_I2C_GenerateStartCondition(I2Cx);
        
        /* Wait for START bit flag */
        ret = stm32_i2c_wait_flag_timeout(I2Cx, LL_I2C_SR1_SB, timeout_ms);
        if (ret != 0) {
            return ret;
        }
        
        /* Send 10-bit address header (read) */
        header = __LL_I2C_10BIT_HEADER_READ(msg->addr);
        LL_I2C_TransmitData8(I2Cx, header);
        
        /* Wait for address sent flag */
        ret = stm32_i2c_wait_flag_timeout(I2Cx, LL_I2C_SR1_ADDR, timeout_ms);
        if (ret != 0) {
            return ret;
        }
        
        /* Clear ADDR flag */
        (void)READ_REG(I2Cx->SR1);
        (void)READ_REG(I2Cx->SR2);
    }
    
    return 0;
}

/**
 * @brief Write data bytes
 * @param I2Cx I2C instance
 * @param buf Data buffer
 * @param len Data length
 * @param timeout_ms Timeout in milliseconds
 * @return 0 on success, error code on failure
 */
static int stm32_i2c_write_data(I2C_TypeDef *I2Cx, 
                                const uint8_t *buf, 
                                uint16_t len,
                                uint32_t timeout_ms)
{
    uint16_t i = 0U;
    int ret = 0;
    
    if ((I2Cx == NULL) || (buf == NULL) || (len == 0U)) {
        return -EINVAL;
    }
    
    for (i = 0U; i < len; i++) {
        /* Wait for TXE flag */
        ret = stm32_i2c_wait_flag_timeout(I2Cx, LL_I2C_SR1_TXE, timeout_ms);
        if (ret != 0) {
            return ret;
        }
        
        /* Send data byte */
        LL_I2C_TransmitData8(I2Cx, buf[i]);
        
        /* Check error */
        ret = stm32_i2c_check_error(I2Cx);
        if (ret != 0) {
            return ret;
        }
    }
    
    /* Wait for BTF flag (byte transfer finished) */
    ret = stm32_i2c_wait_flag_timeout(I2Cx, LL_I2C_SR1_BTF, timeout_ms);
    if (ret != 0) {
        return ret;
    }
    
    return 0;
}

/**
 * @brief Read data bytes
 * @param I2Cx I2C instance
 * @param buf Data buffer
 * @param len Data length
 * @param timeout_ms Timeout in milliseconds
 * @return 0 on success, error code on failure
 */
static int stm32_i2c_read_data(I2C_TypeDef *I2Cx, 
                               uint8_t *buf, 
                               uint16_t len,
                               uint32_t timeout_ms)
{
    uint16_t i = 0U;
    int ret = 0;
    
    if ((I2Cx == NULL) || (buf == NULL) || (len == 0U)) {
        return -EINVAL;
    }
    
    if (len == 1U) {
        /* Single byte read: disable ACK before reading */
        LL_I2C_AcknowledgeNextData(I2Cx, LL_I2C_NACK);
        
        /* Wait for RXNE flag */
        ret = stm32_i2c_wait_flag_timeout(I2Cx, LL_I2C_SR1_RXNE, timeout_ms);
        if (ret != 0) {
            LL_I2C_AcknowledgeNextData(I2Cx, LL_I2C_ACK);
            return ret;
        }
        
        /* Read data */
        buf[0] = LL_I2C_ReceiveData8(I2Cx);
        
        /* Re-enable ACK */
        LL_I2C_AcknowledgeNextData(I2Cx, LL_I2C_ACK);
    } else {
        /* Multiple bytes read */
        for (i = 0U; i < len; i++) {
            if (i == (len - 2U)) {
                /* Second to last byte: disable ACK */
                LL_I2C_AcknowledgeNextData(I2Cx, LL_I2C_NACK);
            }
            
            /* Wait for RXNE flag */
            ret = stm32_i2c_wait_flag_timeout(I2Cx, LL_I2C_SR1_RXNE, timeout_ms);
            if (ret != 0) {
                LL_I2C_AcknowledgeNextData(I2Cx, LL_I2C_ACK);
                return ret;
            }
            
            /* Read data */
            buf[i] = LL_I2C_ReceiveData8(I2Cx);
            
            /* Check error */
            ret = stm32_i2c_check_error(I2Cx);
            if (ret != 0) {
                LL_I2C_AcknowledgeNextData(I2Cx, LL_I2C_ACK);
                return ret;
            }
        }
        
        /* Re-enable ACK */
        LL_I2C_AcknowledgeNextData(I2Cx, LL_I2C_ACK);
    }
    
    return 0;
}

/**
 * @brief Initialize I2C GPIO pins
 * @param i2c_instance I2C peripheral instance
 * @return 0 on success, error code on failure
 */
static int stm32_i2c_gpio_init(I2C_TypeDef *i2c_instance)
{
    LL_GPIO_InitTypeDef gpio_init = {0};
    
    if (i2c_instance == NULL) {
        return -EINVAL;
    }
    
#ifdef BSP_USING_I2C1
    if (i2c_instance == I2C1) {
        /* Enable I2C1 and GPIOB clocks */
        LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_I2C1);
        LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOB);
        
        /* Configure I2C1 SCL pin (PB6) */
        gpio_init.Pin = BSP_I2C1_SCL_PIN;
        gpio_init.Mode = LL_GPIO_MODE_ALTERNATE;
        gpio_init.Speed = LL_GPIO_SPEED_FREQ_HIGH;
        gpio_init.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
        gpio_init.Pull = LL_GPIO_PULL_UP;
        LL_GPIO_Init(BSP_I2C1_SCL_PORT, &gpio_init);
        
        /* Configure I2C1 SDA pin (PB7) */
        gpio_init.Pin = BSP_I2C1_SDA_PIN;
        LL_GPIO_Init(BSP_I2C1_SDA_PORT, &gpio_init);
        
        return 0;
    }
#endif /* BSP_USING_I2C1 */
    
#ifdef BSP_USING_I2C2
    if (i2c_instance == I2C2) {
        /* Enable I2C2 and GPIOB clocks */
        LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_I2C2);
        
        /* Configure I2C2 SCL pin (PB10) */
        gpio_init.Pin = BSP_I2C2_SCL_PIN;
        gpio_init.Mode = LL_GPIO_MODE_ALTERNATE;
        gpio_init.Speed = LL_GPIO_SPEED_FREQ_HIGH;
        gpio_init.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
        gpio_init.Pull = LL_GPIO_PULL_UP;
        LL_GPIO_Init(BSP_I2C2_SCL_PORT, &gpio_init);
        
        /* Configure I2C2 SDA pin (PB11) */
        gpio_init.Pin = BSP_I2C2_SDA_PIN;
        LL_GPIO_Init(BSP_I2C2_SDA_PORT, &gpio_init);
        
        return 0;
    }
#endif /* BSP_USING_I2C2 */
    
    return -EINVAL;
}

/**
 * @brief Configure I2C speed
 * @param i2c_instance I2C peripheral instance
 * @param pclk_freq Peripheral clock frequency
 * @param speed_hz Requested speed in Hz
 * @param actual_speed Output actual speed in Hz
 * @return 0 on success, error code on failure
 */
static int stm32_i2c_config_speed(I2C_TypeDef *i2c_instance, 
                                  uint32_t pclk_freq, 
                                  uint32_t speed_hz,
                                  uint32_t *actual_speed)
{
    uint32_t duty_cycle = 0U;
    uint32_t ccr_value = 0U;
    
    if ((i2c_instance == NULL) || (actual_speed == NULL)) {
        return -EINVAL;
    }
    
    if ((pclk_freq == 0U) || (speed_hz == 0U)) {
        return -EINVAL;
    }
    
    /* Limit speed to maximum */
    if (speed_hz > STM32_I2C_MAX_SPEED_HZ) {
        speed_hz = STM32_I2C_MAX_SPEED_HZ;
    }
    
    /* Select duty cycle for fast mode */
    if (speed_hz > STM32_I2C_STANDARD_SPEED_HZ) {
        duty_cycle = LL_I2C_DUTYCYCLE_2;  /* Fast mode, duty cycle 2 */
    } else {
        duty_cycle = LL_I2C_DUTYCYCLE_2;  /* Standard mode, duty cycle not used */
    }
    
    /* Disable I2C before configuration */
    if (LL_I2C_IsEnabled(i2c_instance) != 0U) {
        LL_I2C_Disable(i2c_instance);
        /* Wait for I2C to be disabled */
        while (LL_I2C_IsEnabled(i2c_instance) != 0U) {
            /* Wait */
        }
    }
    
    /* Configure speed using LL library */
    LL_I2C_ConfigSpeed(i2c_instance, pclk_freq, speed_hz, duty_cycle);
    
    /* Calculate actual speed */
    if (speed_hz > STM32_I2C_STANDARD_SPEED_HZ) {
        /* Fast mode */
        if (duty_cycle == LL_I2C_DUTYCYCLE_2) {
            ccr_value = __LL_I2C_SPEED_FAST_TO_CCR(pclk_freq, speed_hz, LL_I2C_DUTYCYCLE_2);
            *actual_speed = (pclk_freq) / (ccr_value * 3U);
        } else {
            ccr_value = __LL_I2C_SPEED_FAST_TO_CCR(pclk_freq, speed_hz, LL_I2C_DUTYCYCLE_16_9);
            *actual_speed = (pclk_freq) / (ccr_value * 25U);
        }
    } else {
        /* Standard mode */
        ccr_value = __LL_I2C_SPEED_STANDARD_TO_CCR(pclk_freq, speed_hz);
        *actual_speed = (pclk_freq) / (ccr_value * 2U);
    }
    
    /* Enable I2C */
    LL_I2C_Enable(i2c_instance);
    
    return 0;
}

/**
 * @brief Execute I2C transfer (polling mode)
 * @param adap Adapter pointer
 * @param msgs Messages array
 * @param num Number of messages
 * @return Number of messages transferred on success, error code on failure
 */
static int stm32_i2c_master_xfer(struct i2c_adapter *adap, 
                                  struct i2c_msg *msgs, 
                                  int num)
{
    struct stm32_i2c_hw *hw = NULL;
    I2C_TypeDef *I2Cx = NULL;
    int i = 0;
    int ret = 0;
    uint32_t timeout_ms = 0U;
    uint8_t send_stop = 0U;
    
    if ((adap == NULL) || (msgs == NULL) || (num <= 0)) {
        return -EINVAL;
    }
    
    hw = (struct stm32_i2c_hw *)adap->algo_data;
    if (hw == NULL) {
        return -EINVAL;
    }
    
    I2Cx = hw->instance;
    timeout_ms = adap->timeout_ms;
    if (timeout_ms == 0U) {
        timeout_ms = I2C_TIMEOUT_MS;
    }
    
    /* Wait for bus to be free */
    ret = stm32_i2c_wait_busy_timeout(I2Cx, timeout_ms);
    if (ret != 0) {
        return ret;
    }
    
    /* Process each message */
    for (i = 0; i < num; i++) {
        /* Determine if we need to send STOP after this message */
        send_stop = ((msgs[i].flags & I2C_M_STOP) != 0U) || (i == (num - 1));
        
        /* Generate START condition */
        if ((i == 0) || ((msgs[i].flags & I2C_M_NOSTART) == 0U)) {
            LL_I2C_GenerateStartCondition(I2Cx);
        }
        
        /* Send address */
        if ((msgs[i].flags & I2C_M_TEN) != 0U) {
            ret = stm32_i2c_write_10bit_addr(I2Cx, &msgs[i], timeout_ms);
        } else {
            ret = stm32_i2c_write_7bit_addr(I2Cx, &msgs[i], timeout_ms);
        }
        
        if (ret != 0) {
            /* Generate STOP on error */
            LL_I2C_GenerateStopCondition(I2Cx);
            return ret;
        }
        
        /* Transfer data */
        if ((msgs[i].flags & I2C_M_RD) != 0U) {
            /* Read operation */
            ret = stm32_i2c_read_data(I2Cx, msgs[i].buf, msgs[i].len, timeout_ms);
        } else {
            /* Write operation */
            ret = stm32_i2c_write_data(I2Cx, msgs[i].buf, msgs[i].len, timeout_ms);
        }
        
        if (ret != 0) {
            /* Generate STOP on error */
            LL_I2C_GenerateStopCondition(I2Cx);
            return ret;
        }
        
        /* Generate STOP condition if needed */
        if (send_stop != 0U) {
            LL_I2C_GenerateStopCondition(I2Cx);
            
            /* Wait for STOP flag */
            ret = stm32_i2c_wait_flag_timeout(I2Cx, LL_I2C_SR1_STOPF, timeout_ms);
            if (ret != 0) {
                return ret;
            }
            
            /* Clear STOP flag */
            LL_I2C_ClearFlag_STOP(I2Cx);
        }
    }
    
    return num;
}

/**
 * @brief Execute I2C transfer using DMA (optional implementation)
 * @param adap Adapter pointer
 * @param msgs Messages array
 * @param num Number of messages
 * @return Number of messages transferred on success, error code on failure
 * @note This is a placeholder - DMA implementation requires DMA configuration
 */
static int stm32_i2c_master_xfer_dma(struct i2c_adapter *adap,
                                     struct i2c_msg *msgs,
                                     int num)
{
    /* DMA implementation is complex and requires DMA channel configuration */
    /* For now, fall back to polling mode */
    return stm32_i2c_master_xfer(adap, msgs, num);
}

/**
 * @brief I2C algorithm structure
 */
static const struct i2c_algorithm stm32_i2c_algorithm = {
    .master_xfer = stm32_i2c_master_xfer,
    .master_xfer_dma = stm32_i2c_master_xfer_dma,
    .functionality = I2C_FUNC_I2C | 
                     I2C_FUNC_10BIT_ADDR,
                     /* Note: DMA support can be added when DMA is implemented */
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
    LL_I2C_InitTypeDef i2c_init = {0};
    uint32_t actual_speed = 0U;
    
    /* Initialize each I2C adapter */
    for (i = 0U; i < (uint8_t)(sizeof(stm32_i2c_hw) / sizeof(stm32_i2c_hw[0])); i++) {
        hw = &stm32_i2c_hw[i];
        adap = &stm32_i2c_adapter[i];
        
        /* Get PCLK frequency */
        hw->pclk_freq = stm32_i2c_get_pclk_freq(hw->instance);
        if (hw->pclk_freq == 0U) {
            LOG_E("Failed to get PCLK frequency for %s", hw->name);
            continue;
        }
        
        /* Initialize GPIO */
        ret = stm32_i2c_gpio_init(hw->instance);
        if (ret != 0) {
            LOG_E("Failed to initialize GPIO for %s", hw->name);
            continue;
        }
        
        /* Initialize I2C peripheral */
        LL_I2C_StructInit(&i2c_init);
        i2c_init.PeripheralMode = LL_I2C_MODE_I2C;
        i2c_init.ClockSpeed = STM32_I2C_STANDARD_SPEED_HZ;  /* Default 100kHz */
        i2c_init.DutyCycle = LL_I2C_DUTYCYCLE_2;
        i2c_init.OwnAddress1 = 0U;
        i2c_init.TypeAcknowledge = LL_I2C_ACK;
        i2c_init.OwnAddrSize = LL_I2C_OWNADDRESS1_7BIT;
        
        if (LL_I2C_Init(hw->instance, &i2c_init) != SUCCESS) {
            LOG_E("Failed to initialize %s", hw->name);
            continue;
        }
        
        /* Configure speed */
        ret = stm32_i2c_config_speed(hw->instance, 
                                     hw->pclk_freq, 
                                     STM32_I2C_STANDARD_SPEED_HZ,
                                     &actual_speed);
        if (ret != 0) {
            LOG_E("Failed to configure speed for %s", hw->name);
            continue;
        }
        
        /* Set algorithm data */
        adap->algo_data = hw;
        
        /* Configure thread safety */
        #if defined(USING_FREERTOS) || defined(configUSE_MUTEXES)
        /* RTOS environment: create mutex */
        {
            static SemaphoreHandle_t i2c_mutex[I2C_INDEX_MAX] = {NULL};
            if (i2c_mutex[i] == NULL) {
                i2c_mutex[i] = xSemaphoreCreateMutex();
            }
            adap->lock = (void (*)(void *))xSemaphoreTake;
            adap->unlock = (void (*)(void *))xSemaphoreGive;
            adap->lock_data = i2c_mutex[i];
        }
        #else
        /* Bare-metal environment: use interrupt lock */
        adap->irq_disable = __disable_irq;
        adap->irq_enable = __enable_irq;
        #endif
        
        /* Register adapter */
        ret = i2c_add_adapter(adap, hw->name, &stm32_i2c_algorithm);
        if (ret != 0) {
            LOG_E("Failed to register %s adapter", hw->name);
            continue;
        }
        
        /* Set default configuration */
        adap->speed_hz = actual_speed;
        adap->timeout_ms = I2C_TIMEOUT_MS;
        adap->addr_width = 7U;  /* Default 7-bit */
        
        LOG_I("I2C adapter %s registered, speed: %lu Hz", hw->name, actual_speed);
    }
    
    return 0;
}

#endif /* BSP_USING_I2C1 || BSP_USING_I2C2 */
