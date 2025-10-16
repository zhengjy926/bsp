/**
  ******************************************************************************
  * @copyright   : Copyright To Hangzhou Dinova EP Technology Co.,Ltd
  * @file        : bsp_usart.c
  * @author      : ZJY
  * @version     : V1.0
  * @date        : 2025-10-16
  * @brief       : STM32 USART driver implementation
  * @attention   : None
  ******************************************************************************
  * @history     :
  *         V1.0 : 1.Initial version
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "bsp_usart.h"
#include "bsp_conf.h"
#include "bsp_dma.h"
#include "serial.h"
#include "errno-base.h"
#include <stdio.h>

#define  LOG_TAG             "bsp_uart"
#define  LOG_LVL             4
#include "log.h"

#if defined(HAL_UART_MODULE_ENABLED)

/* Private typedef -----------------------------------------------------------*/
struct stm32_uart {
    UART_HandleTypeDef huart;
    USART_TypeDef *Instance;
    DMA_HandleTypeDef hdma_uart_rx;
    DMA_HandleTypeDef hdma_uart_tx;
    char *name;
    uint8_t *rx_dma_buf;
    uint16_t rx_dma_bufsz;
    uint16_t last_pos;
    uint8_t index;
};

/* Private define ------------------------------------------------------------*/
enum
{
#ifdef BSP_USING_UART1
    UART1_INDEX,
#endif

#ifdef BSP_USING_UART2
    UART2_INDEX,
#endif

#ifdef BSP_USING_UART3
    UART3_INDEX,
#endif

#ifdef BSP_USING_UART4
    UART4_INDEX,
#endif

#ifdef BSP_USING_UART5
    UART5_INDEX,
#endif

#ifdef BSP_USING_UART6
    UART6_INDEX,
#endif

#ifdef BSP_USING_UART7
    UART7_INDEX,
#endif

#ifdef BSP_USING_UART8
    UART8_INDEX,
#endif

#ifdef BSP_USING_LPUART1
    LPUART1_INDEX,
#endif
    UART_INDEX_MAX,
};

/* Private variables ---------------------------------------------------------*/
static serial_t serial_dev[UART_INDEX_MAX]; /* 串口设备数组 */

/* 串口缓冲区定义 */
#if defined(BSP_USING_UART1)
    static uint8_t uart1_rx_buf[UART1_RX_BUF_SIZE]              = {0};
    static uint8_t uart1_tx_buf[UART1_TX_BUF_SIZE]              = {0};
    static uint8_t uart1_rx_dma_buf[UART1_RX_TEMP_BUF_SIZE]     = {0};
#endif // BSP_USING_UART1
#if defined(BSP_USING_UART2)
    static uint8_t uart2_rx_buf[UART2_RX_BUF_SIZE]              = {0};
    static uint8_t uart2_tx_buf[UART2_TX_BUF_SIZE]              = {0};
    static uint8_t uart2_rx_dma_buf[UART2_RX_TEMP_BUF_SIZE]     = {0};
#endif // BSP_USING_UART2
#if defined(BSP_USING_UART3)
    static uint8_t uart3_rx_buf[UART3_RX_BUF_SIZE]              = {0};
    static uint8_t uart3_tx_buf[UART3_TX_BUF_SIZE]              = {0};
    static uint8_t uart3_rx_dma_buf[UART3_RX_TEMP_BUF_SIZE]     = {0};
#endif // BSP_USING_UART3
#if defined(BSP_USING_UART4)
    static uint8_t uart4_rx_buf[UART4_RX_BUF_SIZE]              = {0};
    static uint8_t uart4_tx_buf[UART4_TX_BUF_SIZE]              = {0};
    static uint8_t uart4_rx_dma_buf[UART4_RX_TEMP_BUF_SIZE]     = {0};
#endif // BSP_USING_UART4
#if defined(BSP_USING_UART5)
    static uint8_t uart5_rx_buf[UART5_RX_BUF_SIZE]              = {0};
    static uint8_t uart5_tx_buf[UART5_TX_BUF_SIZE]              = {0};
    static uint8_t uart5_rx_dma_buf[UART5_RX_TEMP_BUF_SIZE]     = {0};
#endif // BSP_USING_UART5
#if defined(BSP_USING_UART6)
    static uint8_t uart6_rx_buf[UART6_RX_BUF_SIZE]              = {0};
    static uint8_t uart6_tx_buf[UART6_TX_BUF_SIZE]              = {0};
    static uint8_t uart6_rx_dma_buf[UART6_RX_TEMP_BUF_SIZE]     = {0};
#endif // BSP_USING_UART6
#if defined(BSP_USING_UART7)
    static uint8_t uart7_rx_buf[UART7_RX_BUF_SIZE]              = {0};
    static uint8_t uart7_tx_buf[UART7_TX_BUF_SIZE]              = {0};
    static uint8_t uart7_rx_dma_buf[UART7_RX_TEMP_BUF_SIZE]     = {0};
#endif // BSP_USING_UART7
#if defined(BSP_USING_UART8)
    static uint8_t uart8_rx_buf[UART8_RX_BUF_SIZE]              = {0};
    static uint8_t uart8_tx_buf[UART8_TX_BUF_SIZE]              = {0};
    static uint8_t uart8_rx_dma_buf[UART8_RX_TEMP_BUF_SIZE]     = {0};
#endif // BSP_USING_UART8
#if defined(BSP_USING_LPUART1)
    static uint8_t lpuart1_rx_buf[LPUART1_RX_BUF_SIZE]          = {0};
    static uint8_t lpuart1_tx_buf[LPUART1_TX_BUF_SIZE]          = {0};
    static uint8_t lpuart1_rx_dma_buf[LPUART1_RX_TEMP_BUF_SIZE] = {0};
#endif // BSP_USING_LPUART1

/* 平台专有驱动结构体变量定义 */
static struct stm32_uart stm_uart_drv[UART_INDEX_MAX] =
{
#ifdef BSP_USING_UART1
    {
        .Instance = USART1,
        .rx_dma_buf = uart1_rx_buf,
        .rx_dma_bufsz = UART1_RX_BUF_SIZE,
        .index = UART1_INDEX,
        .name = "uart1",
    },
#endif
    
#ifdef BSP_USING_UART2
    {
        .Instance = USART2,
        .rx_dma_buf = uart2_rx_buf,
        .rx_dma_bufsz = UART2_RX_BUF_SIZE,
        .index = UART2_INDEX,
        .name = "uart2",
    },
#endif

#ifdef BSP_USING_UART3
    {
        .Instance = USART3,
        .rx_dma_buf = uart3_rx_buf,
        .rx_dma_bufsz = UART3_RX_BUF_SIZE,
        .index = UART3_INDEX,
        .name = "uart3",
    },
#endif

#ifdef BSP_USING_UART4
    {
        .Instance = UART4,
        .rx_dma_buf = uart4_rx_buf,
        .rx_dma_bufsz = UART4_RX_BUF_SIZE,
        .index = UART4_INDEX,
        .name = "uart4",
    },
#endif
#ifdef BSP_USING_UART5
    {
        .Instance = UART5,
        .rx_dma_buf = uart5_rx_buf,
        .rx_dma_bufsz = UART5_RX_BUF_SIZE,
        .index = UART5_INDEX,
        .name = "uart5",
    },
#endif
#ifdef BSP_USING_UART6
    {
        .Instance = UART6,
        .rx_dma_buf = uart6_rx_buf,
        .rx_dma_bufsz = UART6_RX_BUF_SIZE,
        .index = UART6_INDEX,
        .name = "uart6",
    },
#endif
#ifdef BSP_USING_UART7
    {
        .Instance = UART7,
        .rx_dma_buf = uart7_rx_buf,
        .rx_dma_bufsz = UART7_RX_BUF_SIZE,
        .index = UART7_INDEX,
        .name = "uart7",
    },
#endif
#ifdef BSP_USING_UART8
    {
        .Instance = UART8,
        .rx_dma_buf = uart8_rx_buf,
        .rx_dma_bufsz = UART8_RX_BUF_SIZE,
        .index = UART8_INDEX,
        .name = "uart8",
    },
#endif
#ifdef BSP_USING_LPUART1
    {
        .Instance = LPUART1,
        .rx_dma_buf = lpuart1_rx_buf,
        .rx_dma_bufsz = LPUART1_RX_BUF_SIZE,
        .index = LPUART1_INDEX,
        .name = "lpuart1",
    },
#endif
};

/* Function prototypes */
static int stm32_usart_init(serial_t *port);
static int stm32_uart_tx(serial_t *port, const void *buf, size_t size);
static int stm32_uart_start_rx(serial_t *port);
static int stm32_uart_configure(serial_t *port, struct serial_configure *cfg);
static void stm32_uart_gpio_init(struct stm32_uart *uartHandle);
static void stm32_uart_dma_config(struct stm32_uart *uartHandle);
static bool stm32_uart_tx_is_busy(struct serial *port);

/* Serial operations structure */
static const serial_ops_t stm_uart_ops = {
    .init = stm32_usart_init,
    .send = stm32_uart_tx,
    .configure = stm32_uart_configure,
    .start_rx = stm32_uart_start_rx,
    .tx_is_busy = stm32_uart_tx_is_busy,
};

/* Private function prototypes -----------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

/**
  * @brief  STM32 USART initialization function
  * @param  port: Pointer to serial device
  * @retval 0 on success, negative error code on failure
  */
static int stm32_usart_init(serial_t *port)
{
    struct stm32_uart *stm_uart = (struct stm32_uart *)port->prv_data;
    int ret = 0;
    
    stm32_uart_gpio_init(stm_uart);
    stm32_uart_dma_config(stm_uart);
    
    /* Initialize port configuration with default values */
    struct serial_configure cfg = SERIAL_CONFIG_DEFAULT;
    port->config = cfg;
    
    /* Configure UART handle */
    stm_uart->huart.Instance = stm_uart->Instance;
    stm_uart->huart.Init.Mode = UART_MODE_TX_RX;
    stm_uart->huart.Init.OverSampling = UART_OVERSAMPLING_16;
#ifdef STM32G474xx
    stm_uart->huart.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    stm_uart->huart.Init.ClockPrescaler = UART_PRESCALER_DIV1;
    stm_uart->huart.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
#endif
    
    ret = stm32_uart_configure(port, &cfg);
    if (ret != 0)
        return ret;

#ifdef STM32G474xx
    /* Configure FIFO thresholds for STM32G4 series */
    if (HAL_UARTEx_SetTxFifoThreshold(&stm_uart->huart, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK) {
        return -EIO;
    }
    if (HAL_UARTEx_SetRxFifoThreshold(&stm_uart->huart, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK) {
        return -EIO;
    }
    if (HAL_UARTEx_DisableFifoMode(&stm_uart->huart) != HAL_OK) {
        return -EIO;
    }
#endif
    stm32_uart_start_rx(port);
    return 0;
}

/**
  * @brief  Configure UART parameters
  * @param  port: Pointer to serial device
  * @param  cfg: Pointer to configuration structure
  * @retval 0 on success, negative error code on failure
  */
static int stm32_uart_configure(serial_t *port, struct serial_configure *cfg)
{
    struct stm32_uart *stm_uart = (struct stm32_uart *)port->prv_data;
    
    if (!port || !cfg || !stm_uart) {
        return -EINVAL;
    }
    
    /* Configure baud rate */
    stm_uart->huart.Init.BaudRate = cfg->baud_rate;
    
    /* Configure data bits */
    switch (cfg->data_bits) {
        case DATA_BITS_8:
            stm_uart->huart.Init.WordLength = UART_WORDLENGTH_8B;
            break;
        case DATA_BITS_9:
            stm_uart->huart.Init.WordLength = UART_WORDLENGTH_9B;
            break;
        default:
            return -EINVAL;
    }
    
    /* Configure stop bits */
    switch (cfg->stop_bits) {
        case STOP_BITS_1:
            stm_uart->huart.Init.StopBits = UART_STOPBITS_1;
            break;
        case STOP_BITS_2:
            stm_uart->huart.Init.StopBits = UART_STOPBITS_2;
            break;
        default:
            return -EINVAL;
    }
    
    /* Configure parity */
    switch (cfg->parity) {
        case PARITY_NONE:
            stm_uart->huart.Init.Parity = UART_PARITY_NONE;
            break;
        case PARITY_ODD:
            stm_uart->huart.Init.Parity = UART_PARITY_ODD;
            break;
        case PARITY_EVEN:
            stm_uart->huart.Init.Parity = UART_PARITY_EVEN;
            break;
        default:
            return -EINVAL;
    }
    
    /* Configure flow control */
    if (cfg->flowcontrol == SERIAL_FLOWCONTROL_CTSRTS) {
        stm_uart->huart.Init.HwFlowCtl = UART_HWCONTROL_RTS_CTS;
    } else {
        stm_uart->huart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    }
    
    /* Apply configuration */
    if (HAL_UART_Init(&stm_uart->huart) != HAL_OK) {
        return -EIO;
    }
    
    return 0;
}

static inline void stm32_uart_rx_dma_config(struct stm32_uart *uartHandle)
{
    /* DMA RX配置 */
    uartHandle->hdma_uart_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    uartHandle->hdma_uart_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    uartHandle->hdma_uart_rx.Init.MemInc = DMA_MINC_ENABLE;
    uartHandle->hdma_uart_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    uartHandle->hdma_uart_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    uartHandle->hdma_uart_rx.Init.Mode = DMA_NORMAL;
    uartHandle->hdma_uart_rx.Init.Priority = DMA_PRIORITY_VERY_HIGH;
    
    if (HAL_DMA_Init(&uartHandle->hdma_uart_rx) != HAL_OK) {
        LOG_E("HAL_DMA_Init errno");
        while(1);
    }
    __HAL_LINKDMA(&uartHandle->huart, hdmarx, uartHandle->hdma_uart_rx);
}

static inline void stm32_uart_tx_dma_config(struct stm32_uart *uartHandle)
{
    /* DMA TX配置 */
    uartHandle->hdma_uart_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    uartHandle->hdma_uart_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    uartHandle->hdma_uart_tx.Init.MemInc = DMA_MINC_ENABLE;
    uartHandle->hdma_uart_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    uartHandle->hdma_uart_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    uartHandle->hdma_uart_tx.Init.Mode = DMA_NORMAL;
    uartHandle->hdma_uart_tx.Init.Priority = DMA_PRIORITY_VERY_HIGH;
    
    if (HAL_DMA_Init(&uartHandle->hdma_uart_tx) != HAL_OK) {
        LOG_E("HAL_DMA_Init errno");
        while(1);
    }
    __HAL_LINKDMA(&uartHandle->huart, hdmatx, uartHandle->hdma_uart_tx);
}

/**
 * @brief 配置DMA
 * @param uartHandle: UART句柄
 * @param uart_index: UART索引
 * @retval None
 */
static void stm32_uart_dma_config(struct stm32_uart *uartHandle)
{
    /* DMA controller clock enable */
    __HAL_RCC_DMAMUX1_CLK_ENABLE();
    __HAL_RCC_DMA1_CLK_ENABLE();
    __HAL_RCC_DMA2_CLK_ENABLE();
    
    switch (uartHandle->index) {
#ifdef BSP_USING_UART1
    case UART1_INDEX:
#ifdef BSP_UART1_RX_USING_DMA
        uartHandle->hdma_uart_rx.Instance = BSP_UART1_DMA_RX_INSTANCE;
        uartHandle->hdma_uart_rx.Init.Request = DMA_REQUEST_USART1_RX;
        stm32_uart_rx_dma_config(uartHandle);
        HAL_NVIC_SetPriority(BSP_UART1_DMA_RX_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(BSP_UART1_DMA_RX_IRQn);
#endif
#ifdef BSP_UART1_TX_USING_DMA
        uartHandle->hdma_uart_tx.Instance = BSP_UART1_DMA_TX_INSTANCE;
        uartHandle->hdma_uart_tx.Init.Request = DMA_REQUEST_USART1_TX;
        stm32_uart_tx_dma_config(uartHandle);
        HAL_NVIC_SetPriority(BSP_UART1_DMA_TX_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(BSP_UART1_DMA_TX_IRQn);
        break;
#endif
#endif
    
#ifdef BSP_USING_UART2
    case UART2_INDEX:
#ifdef BSP_UART2_RX_USING_DMA
        uartHandle->hdma_uart_rx.Instance = BSP_UART2_DMA_RX_INSTANCE;
        uartHandle->hdma_uart_rx.Init.Request = DMA_REQUEST_USART2_RX;
        stm32_uart_rx_dma_config(uartHandle);
        HAL_NVIC_SetPriority(BSP_UART2_DMA_RX_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(BSP_UART2_DMA_RX_IRQn);
#endif
#ifdef BSP_UART2_TX_USING_DMA
        uartHandle->hdma_uart_tx.Instance = BSP_UART2_DMA_TX_INSTANCE;
        uartHandle->hdma_uart_tx.Init.Request = DMA_REQUEST_USART2_TX;
        stm32_uart_tx_dma_config(uartHandle);
        HAL_NVIC_SetPriority(BSP_UART2_DMA_RX_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(BSP_UART2_DMA_RX_IRQn);
#endif
        break;
#endif
    
#ifdef BSP_USING_UART3
    case UART3_INDEX:
#ifdef BSP_UART3_RX_USING_DMA
        uartHandle->hdma_uart_rx.Instance = BSP_UART3_DMA_RX_INSTANCE;
        uartHandle->hdma_uart_rx.Init.Request = DMA_REQUEST_USART3_RX;
        stm32_uart_rx_dma_config(uartHandle);
        HAL_NVIC_SetPriority(BSP_UART3_DMA_RX_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(BSP_UART3_DMA_RX_IRQn);
#endif
#ifdef BSP_UART3_TX_USING_DMA
        uartHandle->hdma_uart_tx.Instance = BSP_UART3_DMA_TX_INSTANCE;
        uartHandle->hdma_uart_tx.Init.Request = DMA_REQUEST_USART3_TX;
        stm32_uart_tx_dma_config(uartHandle);
        HAL_NVIC_SetPriority(BSP_UART3_DMA_TX_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(BSP_UART3_DMA_TX_IRQn);
#endif
        break;
#endif

#ifdef BSP_USING_UART4
    case UART4_INDEX:
#ifdef BSP_UART4_RX_USING_DMA
        uartHandle->hdma_uart_rx.Instance = BSP_UART4_DMA_RX_INSTANCE;
        uartHandle->hdma_uart_rx.Init.Request = DMA_REQUEST_UART4_RX;
        stm32_uart_rx_dma_config(uartHandle);
        HAL_NVIC_SetPriority(BSP_UART4_DMA_RX_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(BSP_UART4_DMA_RX_IRQn);
#endif
#ifdef BSP_UART4_TX_USING_DMA
        uartHandle->hdma_uart_tx.Instance = BSP_UART4_DMA_TX_INSTANCE;
        uartHandle->hdma_uart_tx.Init.Request = DMA_REQUEST_UART4_TX;
        stm32_uart_tx_dma_config(uartHandle);
        HAL_NVIC_SetPriority(BSP_UART4_DMA_TX_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(BSP_UART4_DMA_TX_IRQn);
#endif
        break;
#endif

#ifdef BSP_USING_UART5
    case UART5_INDEX:
#ifdef BSP_UART5_RX_USING_DMA
        uartHandle->hdma_uart_rx.Instance = BSP_UART5_DMA_RX_INSTANCE;
        uartHandle->hdma_uart_rx.Init.Request = DMA_REQUEST_UART5_RX;
        stm32_uart_rx_dma_config(uartHandle);
        HAL_NVIC_SetPriority(BSP_UART5_DMA_RX_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(BSP_UART5_DMA_RX_IRQn);
#endif
#ifdef BSP_UART5_TX_USING_DMA
        uartHandle->hdma_uart_tx.Instance = BSP_UART5_DMA_TX_INSTANCE;
        uartHandle->hdma_uart_tx.Init.Request = DMA_REQUEST_UART5_TX;
        stm32_uart_tx_dma_config(uartHandle);
        HAL_NVIC_SetPriority(BSP_UART5_DMA_TX_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(BSP_UART5_DMA_TX_IRQn);
#endif
        break;
#endif

#ifdef BSP_USING_LPUART1
    case LPUART1_INDEX:
#ifdef BSP_LPUART1_RX_USING_DMA
        uartHandle->hdma_uart_rx.Instance = BSP_LPUART1_DMA_RX_INSTANCE;
        uartHandle->hdma_uart_rx.Init.Request = DMA_REQUEST_LPUART1_RX;
        stm32_uart_rx_dma_config(uartHandle);
        HAL_NVIC_SetPriority(BSP_LPUART1_DMA_RX_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(BSP_LPUART1_DMA_RX_IRQn);
#endif
#ifdef BSP_LPUART1_TX_USING_DMA
        uartHandle->hdma_uart_tx.Instance = BSP_LPUART1_DMA_TX_INSTANCE;
        uartHandle->hdma_uart_tx.Init.Request = DMA_REQUEST_LPUART1_TX;
        stm32_uart_tx_dma_config(uartHandle);
        HAL_NVIC_SetPriority(BSP_LPUART1_DMA_TX_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(BSP_LPUART1_DMA_TX_IRQn);
#endif
        break;
#endif
    default:
        return;
    }
}

/**
  * @brief  Initialize UART GPIO pins
  * @param  uartHandle: Pointer to UART handle structure
  * @retval None
  */
static void stm32_uart_gpio_init(struct stm32_uart *uartHandle)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
#if defined(SOC_SERIES_STM32G4) 
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
#endif
    
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    
    switch (uartHandle->index) {
#ifdef BSP_USING_UART1
    case UART1_INDEX:
#if defined(SOC_SERIES_STM32G4) 
        PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1;
        PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
        if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
        {
            LOG_E("HAL_RCCEx_PeriphCLKConfig errno");
            while(1);
        }
#endif
        __HAL_RCC_USART1_CLK_ENABLE();
        
        GPIO_InitStruct.Pin = BSP_UART1_TX_PIN;
        GPIO_InitStruct.Alternate = BSP_UART1_TX_AF;
        HAL_GPIO_Init(BSP_UART1_TX_PORT, &GPIO_InitStruct);
        
        GPIO_InitStruct.Pin = BSP_UART1_RX_PIN;
        GPIO_InitStruct.Alternate = BSP_UART1_RX_AF;
        HAL_GPIO_Init(BSP_UART1_RX_PORT, &GPIO_InitStruct);
        
        HAL_NVIC_SetPriority(USART1_IRQn, 0, BSP_UART1_IRQ_PRIORITY);
        HAL_NVIC_EnableIRQ(USART1_IRQn);
        break;
#endif
#ifdef BSP_USING_UART2
    case UART2_INDEX:
#if defined(SOC_SERIES_STM32G4) 
        PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART2;
        PeriphClkInit.Usart1ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
        if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
        {
            LOG_E("HAL_RCCEx_PeriphCLKConfig errno");
            while(1);
        }
#endif
        __HAL_RCC_USART2_CLK_ENABLE();
    
        GPIO_InitStruct.Pin = BSP_UART2_TX_PIN;
        GPIO_InitStruct.Alternate = BSP_UART2_TX_AF;
        HAL_GPIO_Init(BSP_UART2_TX_PORT, &GPIO_InitStruct);
        GPIO_InitStruct.Pin = BSP_UART2_RX_PIN;
        GPIO_InitStruct.Alternate = BSP_UART2_RX_AF;
        HAL_GPIO_Init(BSP_UART2_RX_PORT, &GPIO_InitStruct);
        HAL_NVIC_SetPriority(USART2_IRQn, 0, BSP_UART2_IRQ_PRIORITY);
        HAL_NVIC_EnableIRQ(USART2_IRQn);
        break;
#endif
#ifdef BSP_USING_UART3
    case UART3_INDEX:
        PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART3;
        PeriphClkInit.Usart1ClockSelection = RCC_USART3CLKSOURCE_PCLK1;
        if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
        {
            Error_Handler();
        }
        __HAL_RCC_USART3_CLK_ENABLE();
        
        GPIO_InitStruct.Pin = BSP_UART3_TX_PIN;
        GPIO_InitStruct.Alternate = BSP_UART3_TX_AF;
        HAL_GPIO_Init(BSP_UART3_TX_PORT, &GPIO_InitStruct);
        GPIO_InitStruct.Pin = BSP_UART3_RX_PIN;
        GPIO_InitStruct.Alternate = BSP_UART3_RX_AF;
        HAL_GPIO_Init(BSP_UART3_RX_PORT, &GPIO_InitStruct);
        HAL_NVIC_SetPriority(USART3_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(USART3_IRQn);
        break;
#endif
#ifdef BSP_USING_UART4
    case UART4_INDEX:
        PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_UART4;
        PeriphClkInit.Uart4ClockSelection = RCC_UART4CLKSOURCE_PCLK1;
        if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
        {
          Error_Handler();
        }
        __HAL_RCC_UART4_CLK_ENABLE();
        
        GPIO_InitStruct.Pin = BSP_UART4_TX_PIN;
        GPIO_InitStruct.Alternate = BSP_UART4_TX_AF;
        HAL_GPIO_Init(BSP_UART4_TX_PORT, &GPIO_InitStruct);
        GPIO_InitStruct.Pin = BSP_UART4_RX_PIN;
        GPIO_InitStruct.Alternate = BSP_UART4_RX_AF;
        HAL_GPIO_Init(BSP_UART4_RX_PORT, &GPIO_InitStruct);
        HAL_NVIC_SetPriority(UART4_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(UART4_IRQn);
        break;
#endif
#ifdef BSP_USING_UART5
    case UART5_INDEX:
        PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_UART5;
        PeriphClkInit.Uart5ClockSelection = RCC_UART5CLKSOURCE_PCLK1;
        if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
        {
            Error_Handler();
        }
        __HAL_RCC_UART5_CLK_ENABLE();
    
        GPIO_InitStruct.Pin = BSP_UART5_TX_PIN;
        GPIO_InitStruct.Alternate = BSP_UART5_TX_AF;
        HAL_GPIO_Init(BSP_UART5_TX_PORT, &GPIO_InitStruct);
        GPIO_InitStruct.Pin = BSP_UART5_RX_PIN;
        GPIO_InitStruct.Alternate = BSP_UART5_RX_AF;
        HAL_GPIO_Init(BSP_UART5_RX_PORT, &GPIO_InitStruct);
        HAL_NVIC_SetPriority(UART5_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(UART5_IRQn);
        break;
#endif
#ifdef BSP_USING_LPUART1
    case LPUART1_INDEX:
        PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_LPUART1;
        PeriphClkInit.Lpuart1ClockSelection = RCC_LPUART1CLKSOURCE_PCLK1;
        if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
        {
            Error_Handler();
        }
        __HAL_RCC_LPUART1_CLK_ENABLE();
        
        GPIO_InitStruct.Pin = BSP_LPUART1_TX_PIN;
        GPIO_InitStruct.Alternate = BSP_LPUART1_TX_AF;
        HAL_GPIO_Init(BSP_LPUART1_TX_PORT, &GPIO_InitStruct);
        GPIO_InitStruct.Pin = BSP_LPUART1_RX_PIN;
        GPIO_InitStruct.Alternate = BSP_LPUART1_RX_AF;
        HAL_GPIO_Init(BSP_LPUART1_RX_PORT, &GPIO_InitStruct);
        HAL_NVIC_SetPriority(LPUART1_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(LPUART1_IRQn);
        break;
#endif
    default:
        return;
    }
}

static bool stm32_uart_tx_is_busy(struct serial *port)
{
    struct stm32_uart *stm_uart = (struct stm32_uart *)port->prv_data;
    
    return (stm_uart->huart.gState == HAL_UART_STATE_BUSY_TX) ? true : false;
}

/**
  * @brief  UART transmit using DMA
  * @param  port: Pointer to serial device
  * @param  buf: Pointer to data buffer
  * @param  size: Number of bytes to transmit
  * @retval 0 on success, negative error code on failure
  */
static int stm32_uart_tx(serial_t *port, const void *buf, size_t size)
{
    struct stm32_uart *stm_uart = (struct stm32_uart *)port->prv_data;
    HAL_StatusTypeDef status;
    const uint8_t *data = (const uint8_t*)buf;
    
    if (!port || !buf || size == 0 || !stm_uart) {
        return -EINVAL;
    }
    
    if (size > UINT16_MAX) {
        return -EINVAL;
    }
    
    status = HAL_UART_Transmit_DMA(&stm_uart->huart, data, (uint16_t)size);
    
    if (status != HAL_OK) {
        return -EIO;
    }
    
    return 0;
}

/**
  * @brief  Start UART DMA/Interrupt reception
  * @param  port: Pointer to serial device
  * @retval 0 on success, negative error code on failure
  */
static int stm32_uart_start_rx(serial_t *port)
{
    struct stm32_uart *stm_uart = (struct stm32_uart *)port->prv_data;
    HAL_StatusTypeDef status;
    
    if (!port || !stm_uart) {
        return -EINVAL;
    }
    
    // 重置位置计数器
    stm_uart->last_pos = 0;

    status = HAL_UARTEx_ReceiveToIdle_DMA(&stm_uart->huart, 
                                         stm_uart->rx_dma_buf, 
                                         stm_uart->rx_dma_bufsz);
    if (status != HAL_OK) {
        return -EIO;
    }
    
    return 0;
}

/**
  * @brief  Initialize STM32 USART hardware
  * @retval 0 on success, negative error code on failure
  */
int bsp_uart_init(void)
{
    int ret = 0;
    
    for (uint8_t i = 0; i < UART_INDEX_MAX; i++)
    {
        serial_dev[i].prv_data = &stm_uart_drv[i];
        serial_dev[i].ops = &stm_uart_ops;
        
        ret = hw_serial_register(&serial_dev[i], stm_uart_drv[i].name);
        if (ret != 0) {
            return ret;
        }
    }

#if defined(BSP_USING_UART1)
    stm_uart_drv[UART1_INDEX].rx_dma_buf = uart1_rx_dma_buf;
    stm_uart_drv[UART1_INDEX].rx_dma_bufsz = sizeof(uart1_rx_dma_buf);

    serial_dev[UART1_INDEX].rx_buf   = uart1_rx_buf;
    serial_dev[UART1_INDEX].rx_bufsz = sizeof(uart1_rx_buf);
    serial_dev[UART1_INDEX].tx_buf   = uart1_tx_buf;
    serial_dev[UART1_INDEX].tx_bufsz = sizeof(uart1_tx_buf);
#endif

#if defined(BSP_USING_UART2)
    stm_uart_drv[UART2_INDEX].rx_dma_buf = uart2_rx_dma_buf;
    stm_uart_drv[UART2_INDEX].rx_dma_bufsz = sizeof(uart2_rx_dma_buf);

    serial_dev[UART2_INDEX].rx_buf   = uart2_rx_buf;
    serial_dev[UART2_INDEX].rx_bufsz = sizeof(uart2_rx_buf);
    serial_dev[UART2_INDEX].tx_buf   = uart2_tx_buf;   
    serial_dev[UART2_INDEX].tx_bufsz = sizeof(uart2_tx_buf);
#endif

#if defined(BSP_USING_UART3)
    stm_uart_drv[UART3_INDEX].rx_dma_buf = uart3_rx_dma_buf;
    stm_uart_drv[UART3_INDEX].rx_dma_bufsz = sizeof(uart3_rx_dma_buf);   

    serial_dev[UART3_INDEX].rx_buf   = uart3_rx_buf;
    serial_dev[UART3_INDEX].rx_bufsz = sizeof(uart3_rx_buf);
    serial_dev[UART3_INDEX].tx_buf   = uart3_tx_buf;
    serial_dev[UART3_INDEX].tx_bufsz = sizeof(uart3_tx_buf);
#endif  

#if defined(BSP_USING_UART4)
    stm_uart_drv[UART4_INDEX].rx_dma_buf = uart4_rx_dma_buf;
    stm_uart_drv[UART4_INDEX].rx_dma_bufsz = sizeof(uart4_rx_dma_buf);

    serial_dev[UART4_INDEX].rx_buf   = uart4_rx_buf;   
    serial_dev[UART4_INDEX].rx_bufsz = sizeof(uart4_rx_buf);
    serial_dev[UART4_INDEX].tx_buf   = uart4_tx_buf;
    serial_dev[UART4_INDEX].tx_bufsz = sizeof(uart4_tx_buf);
#endif

#if defined(BSP_USING_UART5)    
    stm_uart_drv[UART5_INDEX].rx_dma_buf = uart5_rx_dma_buf;
    stm_uart_drv[UART5_INDEX].rx_dma_bufsz = sizeof(uart5_rx_dma_buf);

    serial_dev[UART5_INDEX].rx_buf   = uart5_rx_buf;
    serial_dev[UART5_INDEX].rx_bufsz = sizeof(uart5_rx_buf);
    serial_dev[UART5_INDEX].tx_buf   = uart5_tx_buf;   
    serial_dev[UART5_INDEX].tx_bufsz = sizeof(uart5_tx_buf);
#endif

#if defined(BSP_USING_UART6)
    stm_uart_drv[UART6_INDEX].rx_dma_buf = uart6_rx_dma_buf;
    stm_uart_drv[UART6_INDEX].rx_dma_bufsz = sizeof(usart6_rx_dma_buf);   

    serial_dev[UART6_INDEX].rx_buf   = uart6_rx_buf;
    serial_dev[UART6_INDEX].rx_bufsz = sizeof(uart6_rx_buf);
    serial_dev[UART6_INDEX].tx_buf   = uart6_tx_buf;
    serial_dev[UART6_INDEX].tx_bufsz = sizeof(uart6_tx_buf);
#endif  

#if defined(BSP_USING_UART7)    
    stm32_uart_drv[UART7_INDEX].rx_dma_buf = uart7_rx_dma_buf;
    stm32_uart_drv[UART7_INDEX].rx_dma_bufsz = sizeof(uart7_rx_dma_buf);

    serial_dev[UART7_INDEX].rx_buf   = uart7_rx_buf;
    serial_dev[UART7_INDEX].rx_bufsz = sizeof(uart7_rx_buf);
    serial_dev[UART7_INDEX].tx_buf   = uart7_tx_buf;   
    serial_dev[UART7_INDEX].tx_bufsz = sizeof(uart7_tx_buf);
#endif

#if defined(BSP_USING_UART8)
    stm_uart_drv[UART8_INDEX].rx_dma_buf = uart8_rx_dma_buf;
    stm_uart_drv[UART8_INDEX].rx_dma_bufsz = sizeof(uart8_rx_dma_buf);   

    serial_dev[UART8_INDEX].rx_buf   = uart8_rx_buf;
    serial_dev[UART8_INDEX].rx_bufsz = sizeof(usart8_rx_buf);
    serial_dev[UART8_INDEX].tx_buf   = uart8_tx_buf;
    serial_dev[UART8_INDEX].tx_bufsz = sizeof(uart8_tx_buf);
#endif  

#if defined(BSP_USING_LPUART1)
    stm_uart_drv[LPUART1_INDEX].rx_dma_buf = lpuart1_rx_dma_buf;
    stm_uart_drv[LPUART1_INDEX].rx_dma_bufsz = sizeof(lpuart1_rx_dma_buf);

    serial_dev[LPUART1_INDEX].rx_buf   = lpuart1_rx_buf;    
    serial_dev[LPUART1_INDEX].rx_bufsz = sizeof(lpuart1_rx_buf);
    serial_dev[LPUART1_INDEX].tx_buf   = lpuart1_tx_buf;
    serial_dev[LPUART1_INDEX].tx_bufsz = sizeof(lpuart1_tx_buf);
#endif

    return 0;
}

/* HAL Callback Functions ----------------------------------------------------*/
/**
  * @brief  Tx Transfer completed callback
  * @param  huart: Pointer to UART handle
  * @retval None
  */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    struct stm32_uart *stm_uart = container_of(huart, struct stm32_uart, huart);
    serial_t *port = &serial_dev[stm_uart->index];
    if (port) {
        hw_serial_tx_done_isr(port);
    }
}

/**
  * @brief  Reception Event Callback for DMA mode
  * @param  huart: UART handle
  * @param  Size: Number of data available in application reception buffer
  * @retval None
  */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    struct stm32_uart *stm_uart = container_of(huart, struct stm32_uart, huart);
    serial_t *port = &serial_dev[stm_uart->index];
    uint16_t process_size;

    if (!port) {
        return;
    }

    if (huart->RxEventType == HAL_UART_RXEVENT_HT) 
    {
        // 半满中断 - 处理前半部分数据
        process_size = Size - stm_uart->last_pos;  // 计算需要处理的数据量
        if (process_size > 0) {
            hw_serial_rx_done_isr(port, stm_uart->rx_dma_buf + stm_uart->last_pos, process_size);
            stm_uart->last_pos = Size;  // 更新已处理位置
        }
    }
    else if (huart->RxEventType == HAL_UART_RXEVENT_TC || 
             huart->RxEventType == HAL_UART_RXEVENT_IDLE)
    {
        // 传输完成或空闲中断 - 处理剩余数据
        process_size = Size - stm_uart->last_pos;  // 计算剩余的数据量
        if (process_size > 0) {
            hw_serial_rx_done_isr(port, stm_uart->rx_dma_buf + stm_uart->last_pos, process_size);
        }
        
        // 重置位置计数器并重新启动接收
        stm_uart->last_pos = 0;
        stm32_uart_start_rx(port);
    }
}

/**
  * @brief  UART error callback.
  * @param  huart UART handle.
  * @retval None
  */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    
}

/* Interrupt Service Routines -----------------------------------------------*/
#if defined(BSP_USING_UART1)
    void USART1_IRQHandler(void)
    {
        HAL_UART_IRQHandler(&stm_uart_drv[UART1_INDEX].huart);
    }
#if defined(BSP_UART1_RX_USING_DMA)
    void UART1_DMA_RX_IRQHandler(void)
    {
        HAL_DMA_IRQHandler(&stm_uart_drv[UART1_INDEX].hdma_uart_rx);
    }
#endif // BSP_UART1_RX_USING_DMA
#if defined(BSP_UART1_TX_USING_DMA)
    void UART1_DMA_TX_IRQHandler(void)
    {
        HAL_DMA_IRQHandler(&stm_uart_drv[UART1_INDEX].hdma_uart_tx);
    }
#endif // BSP_UART1_TX_USING_DMA
#endif // BSP_USING_UART1

    
#if defined(BSP_USING_UART2)
    void USART2_IRQHandler(void)
    {
        HAL_UART_IRQHandler(&stm_uart_drv[UART2_INDEX].huart);
    }
#if defined(BSP_UART2_RX_USING_DMA)
    void UART2_DMA_RX_IRQHandler(void)
    {
        HAL_DMA_IRQHandler(&stm_uart_drv[UART2_INDEX].hdma_uart_rx);
    }
#endif // BSP_UART2_RX_USING_DMA
#if defined(BSP_UART2_TX_USING_DMA)
    void UART2_DMA_TX_IRQHandler(void)
    {
        HAL_DMA_IRQHandler(&stm_uart_drv[UART2_INDEX].hdma_uart_tx);
    }
#endif // BSP_UART2_TX_USING_DMA
#endif // BSP_USING_UART2

#if defined(BSP_USING_UART3)
    void USART3_IRQHandler(void)
    {
        HAL_UART_IRQHandler(&stm_uart_drv[UART3_INDEX].huart);
    }
#if defined(BSP_UART3_RX_USING_DMA)
    void UART3_DMA_RX_IRQHandler(void)
    {
        HAL_DMA_IRQHandler(&stm_uart_drv[UART3_INDEX].hdma_uart_rx);
    }
#endif // BSP_UART3_RX_USING_DMA
#if defined(BSP_UART3_TX_USING_DMA)
    void UART3_DMA_TX_IRQHandler(void)
    {
        HAL_DMA_IRQHandler(&stm_uart_drv[UART3_INDEX].hdma_uart_tx);
    }
#endif // BSP_UART3_TX_USING_DMA
#endif // BSP_USING_UART3

#if defined(BSP_USING_UART4)
    void UART4_IRQHandler(void)
    {
        HAL_UART_IRQHandler(&stm_uart_drv[UART4_INDEX].huart);
    }
#if defined(BSP_UART4_RX_USING_DMA)
    void UART4_DMA_RX_IRQHandler(void)
    {
        HAL_DMA_IRQHandler(&stm_uart_drv[UART4_INDEX].hdma_uart_rx);
    }
#endif // BSP_UART4_RX_USING_DMA
#if defined(BSP_UART4_TX_USING_DMA)
    void UART4_DMA_TX_IRQHandler(void)
    {
        HAL_DMA_IRQHandler(&stm_uart_drv[UART4_INDEX].hdma_uart_tx);
    }
#endif // BSP_UART4_TX_USING_DMA
#endif // BSP_USING_UART4

#endif /* defined(HAL_UART_MODULE_ENABLED) */
