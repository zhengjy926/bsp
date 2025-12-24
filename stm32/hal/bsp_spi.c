/**
  ******************************************************************************
  * @file        : bsp_spi.c
  * @author      : ZJY
  * @version     : V1.0
  * @date        : 2025-01-XX
  * @brief       : STM32 SPI BSP驱动实现 (LL库实现)
  * @attention   : None
  ******************************************************************************
  * @history     :
  *         V1.0 : 1. Complete refactoring with LL library
  *                2. Support new SPI framework interface
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/

#include "bsp_spi.h"
#include "spi.h"
#include "gpio.h"
#include "errno-base.h"
#include "bsp_conf.h"
#include "system_stm32f1xx.h"  /* For SystemCoreClock and APBPrescTable */

#define  LOG_TAG             "bsp_spi"
#define  LOG_LVL             4
#include "log.h"

/* Private typedef -----------------------------------------------------------*/

#if defined(BSP_USING_SPI1) || defined(BSP_USING_SPI2) || defined(BSP_USING_SPI3)

/**
 * @brief STM32 SPI hardware data structure
 */
struct stm32_spi_hw {
    SPI_TypeDef *instance;          /**< SPI peripheral instance */
    IRQn_Type irq_type;             /**< SPI interrupt type */
    uint32_t pclk_freq;              /**< Peripheral clock frequency */
    uint32_t max_speed_hz;           /**< Maximum SPI speed */
    const char *name;                /**< Controller name */
};

enum
{
#ifdef BSP_USING_SPI1
    SPI1_INDEX,
#endif

#ifdef BSP_USING_SPI2
    SPI2_INDEX,
#endif

#ifdef BSP_USING_SPI3
    SPI3_INDEX,
#endif
    SPI_INDEX_MAX,
};

/* Private define ------------------------------------------------------------*/

/* STM32F1 SPI Maximum Speed Limits (from datasheet) */
/* According to STM32F1 datasheet, SPI maximum speed is typically 18MHz */
/* However, the actual limit depends on the specific chip variant */
/* For safety, we use a conservative value: 18MHz */
#define STM32_SPI_MAX_SPEED_HZ          (18000000U)  /* 18 MHz - datasheet limit */

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* SPI hardware data array */
static struct stm32_spi_hw stm32_spi_hw[SPI_INDEX_MAX] = {
#ifdef BSP_USING_SPI1
    {
        .instance = SPI1,
        .irq_type = SPI1_IRQn,
        .pclk_freq = 0U,  /* Will be calculated in init */
        .max_speed_hz = 0U,
        .name = "spi1"
    },
#endif
#ifdef BSP_USING_SPI2
    {
        .instance = SPI2,
        .irq_type = SPI2_IRQn,
        .pclk_freq = 0U,
        .max_speed_hz = 0U,
        .name = "spi2"
    },
#endif
#ifdef BSP_USING_SPI3
    {
        .instance = SPI3,
        .irq_type = SPI3_IRQn,
        .pclk_freq = 0U,
        .max_speed_hz = 0U,
        .name = "spi3"
    },
#endif
};

/* SPI controller array */
static struct spi_controller stm32_spi_controller[sizeof(stm32_spi_hw) / sizeof(stm32_spi_hw[0])];

/* Private function prototypes -----------------------------------------------*/
static int stm32_spi_setup(struct spi_controller *ctrl, struct spi_device *dev);
static void stm32_spi_set_cs(struct spi_controller *ctrl, struct spi_device *dev, uint8_t enable);
static ssize_t stm32_spi_transfer_one(struct spi_controller *ctrl, 
                                       struct spi_device *dev,
                                       struct spi_transfer *transfer);
static int stm32_spi_gpio_init(SPI_TypeDef *spi_instance);
static uint32_t stm32_spi_calculate_prescaler(uint32_t pclk_freq, uint32_t max_speed_hz, uint32_t *actual_speed);
static uint32_t stm32_spi_get_pclk_freq(SPI_TypeDef *spi_instance);

/* Exported functions --------------------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

/**
 * @brief Calculate SPI prescaler and actual speed
 * @param pclk_freq Peripheral clock frequency
 * @param max_speed_hz Maximum requested speed
 * @param actual_speed Output actual speed
 * @return Prescaler value (LL_SPI_BAUDRATEPRESCALER_xxx)
 */
static uint32_t stm32_spi_calculate_prescaler(uint32_t pclk_freq, uint32_t max_speed_hz, uint32_t *actual_speed)
{
    uint32_t prescaler;
    uint32_t div;
    
    if ((pclk_freq == 0U) || (max_speed_hz == 0U) || (actual_speed == NULL)) {
        return LL_SPI_BAUDRATEPRESCALER_DIV2;
    }
    
    /* Calculate division factor */
    div = (pclk_freq + max_speed_hz - 1U) / max_speed_hz;
    
    /* Select appropriate prescaler */
    if (div <= 2U) {
        prescaler = LL_SPI_BAUDRATEPRESCALER_DIV2;
        *actual_speed = pclk_freq >> 1U;
    } else if (div <= 4U) {
        prescaler = LL_SPI_BAUDRATEPRESCALER_DIV4;
        *actual_speed = pclk_freq >> 2U;
    } else if (div <= 8U) {
        prescaler = LL_SPI_BAUDRATEPRESCALER_DIV8;
        *actual_speed = pclk_freq >> 3U;
    } else if (div <= 16U) {
        prescaler = LL_SPI_BAUDRATEPRESCALER_DIV16;
        *actual_speed = pclk_freq >> 4U;
    } else if (div <= 32U) {
        prescaler = LL_SPI_BAUDRATEPRESCALER_DIV32;
        *actual_speed = pclk_freq >> 5U;
    } else if (div <= 64U) {
        prescaler = LL_SPI_BAUDRATEPRESCALER_DIV64;
        *actual_speed = pclk_freq >> 6U;
    } else if (div <= 128U) {
        prescaler = LL_SPI_BAUDRATEPRESCALER_DIV128;
        *actual_speed = pclk_freq >> 7U;
    } else {
        prescaler = LL_SPI_BAUDRATEPRESCALER_DIV256;
        *actual_speed = pclk_freq >> 8U;
    }
    
    return prescaler;
}

/**
 * @brief Initialize SPI GPIO pins
 * @param spi_instance SPI peripheral instance
 * @return 0 on success, error code on failure
 */
static int stm32_spi_gpio_init(SPI_TypeDef *spi_instance)
{
    LL_GPIO_InitTypeDef gpio_init = {0};
    
    if (spi_instance == NULL) {
        return -EINVAL;
    }
    
#if defined(BSP_USING_SPI1)
    if (spi_instance == SPI1) {
        /* Enable SPI1 and GPIOA clocks */
        LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SPI1);
        
        /* Configure SPI1 pins */
        gpio_init.Pin = BSP_SPI1_SCK_PIN;
        gpio_init.Mode = LL_GPIO_MODE_ALTERNATE;
        gpio_init.Speed = LL_GPIO_SPEED_FREQ_HIGH;
        gpio_init.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
        LL_GPIO_Init(BSP_SPI1_SCK_PORT, &gpio_init);

        gpio_init.Pin = BSP_SPI1_MOSI_PIN;
        LL_GPIO_Init(BSP_SPI1_MOSI_PORT, &gpio_init);

        gpio_init.Pin = BSP_SPI1_MISO_PIN;
        gpio_init.Mode = LL_GPIO_MODE_FLOATING;
        LL_GPIO_Init(BSP_SPI1_MISO_PORT, &gpio_init);
        
        return 0;
    }
#endif /* BSP_USING_SPI1 */
    
#if defined(BSP_USING_SPI2)
    if (spi_instance == SPI2) {
        /* Enable SPI2 and GPIOB clocks */
        LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_SPI2);
        
        /* Configure SPI2 pins */
        gpio_init.Pin = BSP_SPI2_SCK_PIN;
        gpio_init.Mode = LL_GPIO_MODE_ALTERNATE;
        gpio_init.Speed = LL_GPIO_SPEED_FREQ_HIGH;
        gpio_init.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
        LL_GPIO_Init(BSP_SPI2_SCK_PORT, &gpio_init);

        gpio_init.Pin = BSP_SPI2_MOSI_PIN;
        LL_GPIO_Init(BSP_SPI2_MOSI_PORT, &gpio_init);

        gpio_init.Pin = BSP_SPI2_MISO_PIN;
        gpio_init.Mode = LL_GPIO_MODE_FLOATING;
        LL_GPIO_Init(BSP_SPI2_MISO_PORT, &gpio_init);

        return 0;
    }
#endif /* BSP_USING_SPI2 */
    
#if defined(BSP_USING_SPI3)
    if (spi_instance == SPI3) {
        /* Enable SPI3, GPIOB, GPIOA and AFIO clocks */
        LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_SPI3);
        
        /* Configure SPI3 pins */
        gpio_init.Pin = BSP_SPI3_SCK_PIN;
        gpio_init.Mode = LL_GPIO_MODE_ALTERNATE;
        gpio_init.Speed = LL_GPIO_SPEED_FREQ_HIGH;
        gpio_init.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
        LL_GPIO_Init(BSP_SPI3_SCK_PORT, &gpio_init);

        gpio_init.Pin = BSP_SPI3_MOSI_PIN;
        LL_GPIO_Init(BSP_SPI3_MOSI_PORT, &gpio_init);

        gpio_init.Pin = BSP_SPI3_MISO_PIN;
        gpio_init.Mode = LL_GPIO_MODE_FLOATING;
        LL_GPIO_Init(BSP_SPI3_MISO_PORT, &gpio_init);

        return 0;
    }
#endif /* BSP_USING_SPI3 */
    
    return -EINVAL;
}

/**
 * @brief Setup SPI controller (spi_controller_ops implementation)
 * @param ctrl Controller pointer
 * @param dev Device pointer
 * @return 0 on success, error code on failure
 */
static int stm32_spi_setup(struct spi_controller *ctrl, struct spi_device *dev)
{
    struct stm32_spi_hw *hw;
    LL_SPI_InitTypeDef spi_init = {0};
    uint32_t prescaler;
    uint32_t actual_speed;
    uint32_t polarity;
    uint32_t phase;
    uint32_t bit_order;
    uint32_t transfer_dir;
    
    if ((ctrl == NULL) || (dev == NULL) || (ctrl->priv == NULL)) {
        return -EINVAL;
    }
    
    hw = (struct stm32_spi_hw *)ctrl->priv;
    
    /* Disable SPI before configuration */
    if (LL_SPI_IsEnabled(hw->instance) != 0U) {
        LL_SPI_Disable(hw->instance);
        /* Wait for SPI to be disabled */
        while (LL_SPI_IsEnabled(hw->instance) != 0U) {
            /* Wait */
        }
    }
    
    /* Configure transfer direction */
    if ((dev->mode & SPI_MODE_3WIRE) != 0U) {
        transfer_dir = LL_SPI_HALF_DUPLEX_TX;  /* 3-wire mode */
    } else {
        transfer_dir = LL_SPI_FULL_DUPLEX;     /* 4-wire mode */
    }
    
    /* Configure clock polarity */
    if ((dev->mode & SPI_CPOL) != 0U) {
        polarity = LL_SPI_POLARITY_HIGH;
    } else {
        polarity = LL_SPI_POLARITY_LOW;
    }
    
    /* Configure clock phase */
    if ((dev->mode & SPI_CPHA) != 0U) {
        phase = LL_SPI_PHASE_2EDGE;
    } else {
        phase = LL_SPI_PHASE_1EDGE;
    }
    
    /* Configure bit order */
    if ((dev->mode & SPI_MODE_MSB) != 0U) {
        bit_order = LL_SPI_MSB_FIRST;
    } else {
        bit_order = LL_SPI_LSB_FIRST;
    }
    
    /* Limit requested speed to hardware maximum */
    uint32_t requested_speed = dev->max_speed_hz;
    if (requested_speed > hw->max_speed_hz) {
        LOG_W("SPI%s: Requested speed %lu Hz exceeds maximum %lu Hz, limiting to maximum",
              hw->name, requested_speed, hw->max_speed_hz);
        requested_speed = hw->max_speed_hz;
    }
    
    /* Calculate prescaler */
    prescaler = stm32_spi_calculate_prescaler(hw->pclk_freq, requested_speed, &actual_speed);
    
    /* Verify actual speed does not exceed hardware limit */
    if (actual_speed > hw->max_speed_hz) {
        LOG_E("SPI%s: Calculated speed %lu Hz exceeds hardware limit %lu Hz",
              hw->name, actual_speed, hw->max_speed_hz);
        return -EINVAL;
    }
    
    /* Fill SPI init structure */
    LL_SPI_StructInit(&spi_init);
    
    spi_init.TransferDirection = transfer_dir;
    spi_init.Mode = LL_SPI_MODE_MASTER;
    spi_init.DataWidth = (dev->bits_per_word == 16U) ? 
                        LL_SPI_DATAWIDTH_16BIT : LL_SPI_DATAWIDTH_8BIT;
    spi_init.ClockPolarity = polarity;
    spi_init.ClockPhase = phase;
    spi_init.NSS = LL_SPI_NSS_SOFT;  /* Always use software NSS */
    spi_init.BaudRate = prescaler;
    spi_init.BitOrder = bit_order;
    spi_init.CRCCalculation = LL_SPI_CRCCALCULATION_DISABLE;
    spi_init.CRCPoly = 7U;
    
    /* Initialize SPI */
    if (LL_SPI_Init(hw->instance, &spi_init) != SUCCESS) {
        LOG_E("Failed to initialize SPI%s", hw->name);
        return -EIO;
    }
    
    /* Enable SPI */
    LL_SPI_Enable(hw->instance);
    
    /* Save actual speed to controller (will be read by framework) */
    ctrl->actual_speed_hz = actual_speed;
    
    return 0;
}

/**
 * @brief Set chip select state (spi_controller_ops implementation)
 * @param ctrl Controller pointer
 * @param dev Device pointer
 * @param enable 1=activate (pull low), 0=release (pull high)
 */
static void stm32_spi_set_cs(struct spi_controller *ctrl, struct spi_device *dev, uint8_t enable)
{
    if ((ctrl == NULL) || (dev == NULL)) {
        return;
    }
    
    if ((dev->mode & SPI_MODE_HW_CS) == 0U) {
        /* Software CS: control GPIO */
        /* enable=1 means CS active (low), enable=0 means CS inactive (high) */
        gpio_write(dev->cs_pin, (enable != 0U) ? 0U : 1U);
    } else {
        /* Hardware CS: controlled by hardware NSS pin */
        /* For STM32F1, hardware NSS is managed automatically when NSS is configured */
        /* No additional action needed */
    }
}

/**
 * @brief Execute single transfer (spi_controller_ops implementation)
 * @param ctrl Controller pointer
 * @param dev Device pointer
 * @param transfer Transfer descriptor pointer
 * @return Number of bytes transferred on success, error code on failure
 */
static ssize_t stm32_spi_transfer_one(struct spi_controller *ctrl,
                                     struct spi_device *dev,
                                     struct spi_transfer *transfer)
{
    struct stm32_spi_hw *hw;
    SPI_TypeDef *spi;
    const uint8_t *tx_buf;
    uint8_t *rx_buf;
    size_t len;
    size_t i;
    uint8_t tx_byte;
    uint8_t rx_byte;
    uint32_t timeout;
    
    if ((ctrl == NULL) || (dev == NULL) || (transfer == NULL) || (ctrl->priv == NULL)) {
        return -EINVAL;
    }
    
    if (transfer->len == 0U) {
        return 0;
    }
    
    hw = (struct stm32_spi_hw *)ctrl->priv;
    spi = hw->instance;
    tx_buf = (const uint8_t *)transfer->tx_buf;
    rx_buf = (uint8_t *)transfer->rx_buf;
    len = transfer->len;
    
    /* Check for errors */
    if (LL_SPI_IsActiveFlag_OVR(spi) != 0U) {
        LL_SPI_ClearFlag_OVR(spi);
        return -EIO;
    }
    
    if (LL_SPI_IsActiveFlag_MODF(spi) != 0U) {
        LL_SPI_ClearFlag_MODF(spi);
        return -EIO;
    }
    
    /* Wait for SPI to be ready */
    timeout = 10000U;  /* Timeout counter */
    while ((LL_SPI_IsActiveFlag_BSY(spi) != 0U) && (timeout > 0U)) {
        timeout--;
    }
    
    if (timeout == 0U) {
        return -EIO;
    }
    
    /* Perform transfer */
    for (i = 0U; i < len; i++) {
        /* Prepare TX data */
        if (tx_buf != NULL) {
            tx_byte = tx_buf[i];
        } else {
            tx_byte = 0x00U;  /* Dummy byte for read */
        }
        
        /* Wait for TX buffer empty */
        timeout = 10000U;
        while ((LL_SPI_IsActiveFlag_TXE(spi) == 0U) && (timeout > 0U)) {
            timeout--;
        }
        
        if (timeout == 0U) {
            return (ssize_t)i;  /* Return bytes transferred so far */
        }
        
        /* Send data */
        LL_SPI_TransmitData8(spi, tx_byte);
        
        /* Wait for RX buffer not empty */
        timeout = 10000U;
        while ((LL_SPI_IsActiveFlag_RXNE(spi) == 0U) && (timeout > 0U)) {
            timeout--;
        }
        
        if (timeout == 0U) {
            return (ssize_t)i;  /* Return bytes transferred so far */
        }
        
        /* Read data */
        rx_byte = LL_SPI_ReceiveData8(spi);
        
        /* Store RX data if buffer provided */
        if (rx_buf != NULL) {
            rx_buf[i] = rx_byte;
        }
    }
    
    /* Wait for transfer to complete */
    timeout = 10000U;
    while ((LL_SPI_IsActiveFlag_BSY(spi) != 0U) && (timeout > 0U)) {
        timeout--;
    }
    
    /* Check for overrun error */
    if (LL_SPI_IsActiveFlag_OVR(spi) != 0U) {
        LL_SPI_ClearFlag_OVR(spi);
        return -EIO;
    }
    
    return (ssize_t)len;
}

/**
 * @brief SPI controller operations
 */
static const struct spi_controller_ops stm32_spi_ops = {
    .setup = stm32_spi_setup,
    .set_cs = stm32_spi_set_cs,
    .transfer_one = stm32_spi_transfer_one,
};

/**
 * @brief Get peripheral clock frequency for SPI
 * @param spi_instance SPI peripheral instance
 * @return Clock frequency in Hz, 0 on error
 * @note Uses LL library macros for cleaner implementation
 */
static uint32_t stm32_spi_get_pclk_freq(SPI_TypeDef *spi_instance)
{
    uint32_t hclk_freq;
    uint32_t apb_prescaler;
    
    if (spi_instance == NULL) {
        return 0U;
    }
    
    /* Get HCLK frequency (SystemCoreClock) */
    hclk_freq = SystemCoreClock;
    
#if defined(BSP_USING_SPI1)
    if (spi_instance == SPI1) {
        /* SPI1 is on APB2 */
        apb_prescaler = LL_RCC_GetAPB2Prescaler();
        /* Use LL library macro to calculate PCLK2 frequency */
        return __LL_RCC_CALC_PCLK2_FREQ(hclk_freq, apb_prescaler);
    }
#endif
    
#if defined(BSP_USING_SPI2) || defined(BSP_USING_SPI3)
    if ((spi_instance == SPI2) || (spi_instance == SPI3)) {
        /* SPI2/3 are on APB1 */
        apb_prescaler = LL_RCC_GetAPB1Prescaler();
        /* Use LL library macro to calculate PCLK1 frequency */
        return __LL_RCC_CALC_PCLK1_FREQ(hclk_freq, apb_prescaler);
    }
#endif
    
    return 0U;
}

/**
 * @brief Initialize STM32 SPI BSP driver
 * @return 0 on success, error code on failure
 */
int bsp_spi_init(void)
{
    int ret;
    uint8_t i;
    uint32_t pclk_freq;
    uint32_t max_speed_hz;
    size_t spi_count;
    
    spi_count = sizeof(stm32_spi_hw) / sizeof(stm32_spi_hw[0]);
    
    for (i = 0U; i < spi_count; i++) {
        /* Get peripheral clock frequency */
        pclk_freq = stm32_spi_get_pclk_freq(stm32_spi_hw[i].instance);
        if (pclk_freq == 0U) {
            LOG_E("Failed to get PCLK frequency for SPI%u", i + 1U);
            return -EIO;
        }
        
        stm32_spi_hw[i].pclk_freq = pclk_freq;
        
        /* Calculate maximum speed */
        /* STM32F1: In master mode with polling, max speed is PCLK/2 */
        /* However, we must also respect datasheet limit (typically 18MHz) */
        max_speed_hz = pclk_freq >> 1U;  /* PCLK/2 - hardware limit */
        
        /* Apply datasheet maximum speed limit */
        if (max_speed_hz > STM32_SPI_MAX_SPEED_HZ) {
            max_speed_hz = STM32_SPI_MAX_SPEED_HZ;
        }
        
        stm32_spi_hw[i].max_speed_hz = max_speed_hz;
        
        /* Initialize GPIO */
        ret = stm32_spi_gpio_init(stm32_spi_hw[i].instance);
        if (ret != 0) {
            LOG_E("Failed to initialize GPIO for SPI%u", i + 1U);
            return ret;
        }
        
        /* Register controller */
        ret = spi_controller_register(&stm32_spi_controller[i], 
                                     stm32_spi_hw[i].name, 
                                     &stm32_spi_ops);
        if (ret != 0) {
            LOG_E("Failed to register SPI controller '%s'", stm32_spi_hw[i].name);
            return ret;
        }
        
        /* Set controller private data */
        stm32_spi_controller[i].priv = &stm32_spi_hw[i];
        
        /* Configure thread safety (bare-metal: use interrupt control) */
        stm32_spi_controller[i].irq_disable = __disable_irq;
        stm32_spi_controller[i].irq_enable = __enable_irq;
        
        LOG_I("SPI controller '%s' initialized (max speed: %lu Hz)", 
              stm32_spi_hw[i].name, max_speed_hz);
    }
    
    return 0;
}

#endif
