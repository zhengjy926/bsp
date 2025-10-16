/**
  ******************************************************************************
  * @file        : bsp_spi.c
  * @author      : ZJY
  * @version     : V1.0
  * @date        : 2025-10-16
  * @brief       : STM32 SPI驱动实现
  * @attention   : None
  ******************************************************************************
  * @history     :
  *         V1.0 : 1.Initial version
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "bsp_spi.h"
#include "bsp_dma.h"
#include "spi.h"
#include "bsp_conf.h"
#include "gpio.h"
#include <stdlib.h>
#include <assert.h>

//#define  DEBUG_TAG                  "stm32_spi"
//#define  FILE_DEBUG_LEVEL           3
//#define  FILE_ASSERT_ENABLED        1
//#include "debug.h"

#if defined(HAL_SPI_MODULE_ENABLED)
/* Private typedef -----------------------------------------------------------*/
struct stm32_spi {
    SPI_HandleTypeDef hspi;
    SPI_TypeDef *instance;
    char *name;
    IRQn_Type irq_type;
    struct dma_config rx_dma;
    struct dma_config tx_dma;
    uint8_t using_rx_dma;
    uint8_t using_tx_dma;
};

/* Private define ------------------------------------------------------------*/
#define SPI_USING_RX_DMA_FLAG   (1<<0)
#define SPI_USING_TX_DMA_FLAG   (1<<1)
/* Private macro -------------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

/* Private functions ---------------------------------------------------------*/
#if defined(BSP_USING_SPI1) || defined(BSP_USING_SPI2) || defined(BSP_USING_SPI3) \
    || defined(BSP_USING_SPI4) || defined(BSP_USING_SPI5) || defined(BSP_USING_SPI6)
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
#ifdef BSP_USING_SPI4
    SPI4_INDEX,
#endif
#ifdef BSP_USING_SPI5
    SPI5_INDEX,
#endif
#ifdef BSP_USING_SPI6
    SPI6_INDEX,
#endif
};

static struct stm32_spi stm_spi_drv[] =
{
#ifdef BSP_USING_SPI1
    {
        .instance = SPI1,
        .irq_type = SPI1_IRQn,
        .hdma_tx = &hdma_spi1_tx,
        .hdma_rx = &hdma_spi1_rx,
        .dma_tx_channel = DMA_CHANNEL_4,
        .dma_rx_channel = DMA_CHANNEL_4
    },
#endif

#ifdef BSP_USING_SPI2
    {
        .instance = SPI2,
        .irq_type = SPI2_IRQn,
        .hdma_tx = &hdma_spi2_tx,
        .hdma_rx = &hdma_spi2_rx,
        .dma_tx_channel = DMA_CHANNEL_4,
        .dma_rx_channel = DMA_CHANNEL_4
    },
#endif

#ifdef BSP_USING_SPI3
    {
        .instance = SPI3,
        .irq_type = SPI3_IRQn,
        .hdma_tx = &hdma_spi3_tx,
        .hdma_rx = &hdma_spi3_rx,
        .dma_tx_channel = DMA_CHANNEL_4,
        .dma_rx_channel = DMA_CHANNEL_4
    },
#endif

#ifdef BSP_USING_SPI4
    {
        .instance = SPI4,
        .irq_type = SPI4_IRQn,
        .name     = "spi4",
        .using_rx_dma = 1,
        .using_tx_dma = 1,
        .rx_dma = {
            .Instance = DMA2_Stream0,
            .channel = DMA_CHANNEL_4,
            .dma_irq = DMA2_Stream0_IRQn
        },
        .tx_dma = {
            .Instance = DMA2_Stream1,
            .channel = DMA_CHANNEL_4,
            .dma_irq = DMA2_Stream1_IRQn
        }
    },
#endif

#ifdef BSP_USING_SPI5
    {
        .instance = SPI5,
        .irq_type = SPI5_IRQn,
        .name     = "spi5",
        .using_rx_dma = 1,
        .using_tx_dma = 1,
        .rx_dma = {
            .Instance = DMA2_Stream3,
            .channel = DMA_CHANNEL_2,
            .dma_irq = DMA2_Stream3_IRQn
        },
        .tx_dma = {
            .Instance = DMA2_Stream4,
            .channel = DMA_CHANNEL_2,
            .dma_irq = DMA2_Stream4_IRQn
        }
    },
#endif

#ifdef BSP_USING_SPI6
    {
        .instance = SPI6,
        .irq_type = SPI6_IRQn,
        .hdma_tx = &hdma_spi6_tx,
        .hdma_rx = &hdma_spi6_rx,
        .dma_tx_channel = DMA_CHANNEL_4,
        .dma_rx_channel = DMA_CHANNEL_4
    },
#endif
};

static int stm32_spi_dma_init(struct stm32_spi *spi)
{
    if (!spi->using_rx_dma && !spi->using_tx_dma)
        return 0;

    /* Enable DMA clock */
    __HAL_RCC_DMA2_CLK_ENABLE();

    /* Configure RX DMA if needed */
    if (spi->using_rx_dma) {
        /* Configure DMA handle */
        spi->rx_dma.hdma.Instance = spi->rx_dma.Instance;
        spi->rx_dma.hdma.Init.Channel = spi->rx_dma.channel;
        spi->rx_dma.hdma.Init.Direction = DMA_PERIPH_TO_MEMORY;
        spi->rx_dma.hdma.Init.PeriphInc = DMA_PINC_DISABLE;
        spi->rx_dma.hdma.Init.MemInc = DMA_MINC_ENABLE;
        spi->rx_dma.hdma.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        spi->rx_dma.hdma.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
        spi->rx_dma.hdma.Init.Mode = DMA_NORMAL;
        spi->rx_dma.hdma.Init.Priority = DMA_PRIORITY_LOW;
        spi->rx_dma.hdma.Init.FIFOMode = DMA_FIFOMODE_DISABLE;

        if (HAL_DMA_Init(&spi->rx_dma.hdma) != HAL_OK)
            return -EIO;

        __HAL_LINKDMA(&spi->hspi, hdmarx, spi->rx_dma.hdma);

        /* Enable DMA IRQ */
        HAL_NVIC_SetPriority(spi->rx_dma.dma_irq, 0, 0);
        HAL_NVIC_EnableIRQ(spi->rx_dma.dma_irq);
    }

    /* Configure TX DMA if needed */
    if (spi->using_tx_dma) {
        /* Configure DMA handle */
        spi->tx_dma.hdma.Instance = spi->tx_dma.Instance;
        spi->tx_dma.hdma.Init.Channel = spi->tx_dma.channel;
        spi->tx_dma.hdma.Init.Direction = DMA_MEMORY_TO_PERIPH;
        spi->tx_dma.hdma.Init.PeriphInc = DMA_PINC_DISABLE;
        spi->tx_dma.hdma.Init.MemInc = DMA_MINC_ENABLE;
        spi->tx_dma.hdma.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        spi->tx_dma.hdma.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
        spi->tx_dma.hdma.Init.Mode = DMA_NORMAL;
        spi->tx_dma.hdma.Init.Priority = DMA_PRIORITY_LOW;
        spi->tx_dma.hdma.Init.FIFOMode = DMA_FIFOMODE_DISABLE;

        if (HAL_DMA_Init(&spi->tx_dma.hdma) != HAL_OK)
            return -EIO;

        __HAL_LINKDMA(&spi->hspi, hdmatx, spi->tx_dma.hdma);

        /* Enable DMA IRQ */
        HAL_NVIC_SetPriority(spi->tx_dma.dma_irq, 0, 0);
        HAL_NVIC_EnableIRQ(spi->tx_dma.dma_irq);
    }

    return 0;
}

static int stm32_spi_gpio_init(struct stm32_spi *spi)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    if(spi->instance == SPI2)
    {
        /* SPI2 clock enable */
        __HAL_RCC_SPI2_CLK_ENABLE();
    }
    
    if(spi->instance == SPI4)
    {
        /* SPI2 clock enable */
        __HAL_RCC_SPI4_CLK_ENABLE();

        /* SPI2 GPIO Configuration */
        GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_5|GPIO_PIN_6;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF5_SPI4;
        HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
        
        HAL_NVIC_SetPriority(spi->irq_type, 0, 0);
        HAL_NVIC_EnableIRQ(spi->irq_type);
        
        return 0;
    }
    
    if (spi->instance == SPI5)
    {
        /* SPI5 clock enable */
        __HAL_RCC_SPI5_CLK_ENABLE();
        
        GPIO_InitStruct.Pin = GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF5_SPI5;
        HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
        
        /* SPI5 interrupt Init */
        HAL_NVIC_SetPriority(spi->irq_type, 0, 0);
        HAL_NVIC_EnableIRQ(spi->irq_type);
        
        return 0;
    }
    
    return -EIO;
}

static int stm32_spi_configure(struct spi_device *dev)
{
    struct spi_bus *bus;
    struct stm32_spi *spi;
    SPI_HandleTypeDef *handle;
    
    if (!dev || !dev->bus || !dev->bus->hw_data)
        return -EINVAL;
    
    bus = dev->bus;
    spi = bus->hw_data;
    handle = &spi->hspi;
    
    handle->Instance = spi->instance;
    handle->Init.Mode = SPI_MODE_MASTER;
    handle->Init.Direction = (dev->mode & SPI_MODE_3WIRE) ? 
                            SPI_DIRECTION_1LINE : SPI_DIRECTION_2LINES;
    handle->Init.DataSize = (dev->data_width == 8) ? 
                            SPI_DATASIZE_8BIT : SPI_DATASIZE_16BIT;
    handle->Init.CLKPolarity = (dev->mode & SPI_CPOL) ? 
                            SPI_POLARITY_HIGH : SPI_POLARITY_LOW;
    handle->Init.CLKPhase = (dev->mode & SPI_CPHA) ? 
                            SPI_PHASE_2EDGE : SPI_PHASE_1EDGE;
    handle->Init.NSS = (dev->mode & SPI_MODE_HW_CS) ? 
                            SPI_NSS_HARD_OUTPUT : SPI_NSS_SOFT;
    
    /* Calculate baudrate prescaler */
    if (dev->max_hz >= bus->max_hz) {
        handle->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
        bus->actual_hz = bus->max_hz;
    } else {
        uint32_t div = (bus->max_hz + dev->max_hz - 1) / dev->max_hz;
        if (div <= 2) {
            handle->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
            bus->actual_hz = bus->max_hz >> 1;
        } else if (div <= 4) {
            handle->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
            bus->actual_hz = bus->max_hz >> 2;
        } else if (div <= 8) {
            handle->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
            bus->actual_hz = bus->max_hz >> 3;
        } else if (div <= 16) {
            handle->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
            bus->actual_hz = bus->max_hz >> 4;
        } else if (div <= 32) {
            handle->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
            bus->actual_hz = bus->max_hz >> 5;
        } else if (div <= 64) {
            handle->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_128;
            bus->actual_hz = bus->max_hz >> 6;
        } else {
            handle->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
            bus->actual_hz = bus->max_hz >> 7;
        }
    }
    
    handle->Init.FirstBit = (dev->mode & SPI_MODE_MSB) ? 
                            SPI_FIRSTBIT_MSB : SPI_FIRSTBIT_LSB;
    handle->Init.TIMode = SPI_TIMODE_DISABLE;
    handle->Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    handle->Init.CRCPolynomial = 10;

    return (HAL_SPI_Init(handle) == HAL_OK) ? 0 : -EIO;
}

/**
 * @brief  设置SPI片选信号
 * @param  spi: SPI设备结构体指针
 * @param  enable: true-使能，false-禁止
 */
static void stm32_spi_set_cs(struct spi_device *dev, uint8_t enable)
{
    if (dev && dev->cs_pin)
        gpio_write(dev->cs_pin, enable ? 0 : 1);
}

static ssize_t stm32_spi_xfer(struct spi_device *dev, struct spi_message *message)
{
    HAL_StatusTypeDef state;
    struct spi_bus *bus;
    struct stm32_spi *spi;
    SPI_HandleTypeDef *handle;
    size_t total_len = 0;
    
    if (!dev || !dev->bus || !message)
        return -EINVAL;
        
    bus = dev->bus;
    spi = bus->hw_data;
    handle = &spi->hspi;
    
    const uint8_t *tx_buf = message->send_buf;
    uint8_t *rx_buf = message->recv_buf;
    size_t len = message->length;
    
    while (len > 0)
    {
        size_t chunk_len = (len > 65535) ? 65535 : len;
        
        if (tx_buf && rx_buf)
            state = HAL_SPI_TransmitReceive_DMA(handle, (uint8_t*)tx_buf, rx_buf, chunk_len);
        else if (tx_buf)
            state = HAL_SPI_Transmit_DMA(handle, (uint8_t*)tx_buf, chunk_len);
        else {
            state = HAL_SPI_Receive_DMA(handle, rx_buf, chunk_len);
        }
        
        if (state != HAL_OK) {
            handle->State = HAL_SPI_STATE_READY;
            return -EIO;
        }
        
        while (HAL_SPI_GetState(handle) != HAL_SPI_STATE_READY);
        
        if (tx_buf) tx_buf += chunk_len;
        if (rx_buf) rx_buf += chunk_len;
        len -= chunk_len;
        total_len += chunk_len;
    }
    
    return total_len;
}

static const struct spi_ops stm_spi_ops = {
    .configure = stm32_spi_configure,
    .set_cs = stm32_spi_set_cs,
    .xfer = stm32_spi_xfer,
};

static struct spi_bus spi_bus[sizeof(stm_spi_drv)/sizeof(stm_spi_drv[0])];

int bsp_spi_init(void)
{
    int ret = 0;
    
    for (uint8_t i = 0; i < sizeof(stm_spi_drv)/sizeof(stm_spi_drv[0]); i++)
    {
        spi_bus[i].hw_data = &stm_spi_drv[i];
        spi_bus[i].ops = &stm_spi_ops;
        
        if (stm_spi_drv[i].instance == SPI1 || stm_spi_drv[i].instance == SPI4 || \
            stm_spi_drv[i].instance == SPI5 || stm_spi_drv[i].instance == SPI6) {
            spi_bus[i].max_hz = SystemCoreClock >> 1;
        } else {
            spi_bus[i].max_hz = SystemCoreClock >> 2;
        }

        /* Initialize GPIO for each SPI */
        ret = stm32_spi_gpio_init(&stm_spi_drv[i]);
        if (ret != 0)
            return ret;

        /* Initialize DMA if needed */
        ret = stm32_spi_dma_init(&stm_spi_drv[i]);
        if (ret != 0)
            return ret;

        ret = spi_bus_register(&spi_bus[i], stm_spi_drv[i].name, &stm_spi_ops);
        assert(ret == 0);
    }
    return 0;
}

#if defined(BSP_USING_SPI4)
void SPI4_IRQHandler(void)
{
    HAL_SPI_IRQHandler(&stm_spi_drv[SPI4_INDEX].hspi);
}
#endif  /* BSP_USING_SPI4 */

#if defined(BSP_USING_SPI4) && defined(BSP_SPI4_RX_USING_DMA)
void DMA2_Stream0_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&stm_spi_drv[SPI4_INDEX].rx_dma.hdma);
}
#endif  /* BSP_USING_SPI4 && BSP_SPI4_RX_USING_DMA */

#if defined(BSP_USING_SPI4) && defined(BSP_SPI4_TX_USING_DMA)
void DMA2_Stream1_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&stm_spi_drv[SPI4_INDEX].tx_dma.hdma);
}
#endif  /* BSP_USING_SPI4 && BSP_SPI4_TX_USING_DMA */

#if defined(BSP_USING_SPI5)
void SPI5_IRQHandler(void)
{
    HAL_SPI_IRQHandler(&stm_spi_drv[SPI5_INDEX].hspi);
}
#endif  /* BSP_USING_SPI5 */

#if defined(BSP_USING_SPI5) && defined(BSP_SPI5_RX_USING_DMA)
void DMA2_Stream3_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&stm_spi_drv[SPI5_INDEX].rx_dma.hdma);
}
#endif  /* BSP_USING_SPI5 && BSP_SPI5_RX_USING_DMA */

#if defined(BSP_USING_SPI5) && defined(BSP_SPI5_TX_USING_DMA)
void DMA2_Stream4_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&stm_spi_drv[SPI5_INDEX].tx_dma.hdma);
}
#endif  /* BSP_USING_SPI5 && BSP_SPI5_TX_USING_DMA */

#endif /* BSP_USING_SPI1 || BSP_USING_SPI2 || BSP_USING_SPI3 || BSP_USING_SPI4 || BSP_USING_SPI5 */
#endif /* USING_SPI */
