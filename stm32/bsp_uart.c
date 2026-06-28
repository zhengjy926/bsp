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
#include "bsp_uart.h"
#include "main.h"
#include "bsp_conf.h"
#include "dev_uart.h"
#include "errno-base.h"

#define  LOG_TAG             "bsp_uart"
#define  LOG_LVL             ELOG_LVL_DEBUG
#include "elog.h"

#if defined(HAL_UART_MODULE_ENABLED)

/* Private typedef -----------------------------------------------------------*/
struct stm32_uart {
    UART_HandleTypeDef *huart;
    uint8_t *rx_cache_buf;
    uint16_t rx_cache_bufsz;
    uint16_t last_pos;
    Uart_t *port;               ///< Back-pointer to generic uart device, set by BSP_UART_Init
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

/* 串口缓冲区定义 */
#if defined(BSP_USING_UART1)
    extern UART_HandleTypeDef huart1;
    static uint8_t uart1_rx_buf[UART1_RX_BUF_SIZE] = {0};
    static uint8_t uart1_tx_buf[UART1_TX_BUF_SIZE] = {0};
    static uint8_t uart1_rx_cache_buf[UART1_RX_CACHE_BUF_SIZE] = {0};
#endif // BSP_USING_UART1
#if defined(BSP_USING_UART2)
    extern UART_HandleTypeDef huart2;
    static uint8_t uart2_rx_buf[UART2_RX_BUF_SIZE] = {0};
    static uint8_t uart2_tx_buf[UART2_TX_BUF_SIZE] = {0};
    static uint8_t uart2_rx_cache_buf[UART2_RX_CACHE_BUF_SIZE] = {0};
#endif // BSP_USING_UART2
#if defined(BSP_USING_UART3)
    extern UART_HandleTypeDef huart3;
    static uint8_t uart3_rx_buf[UART3_RX_BUF_SIZE] = {0};
    static uint8_t uart3_tx_buf[UART3_TX_BUF_SIZE] = {0};
    static uint8_t uart3_rx_cache_buf[UART3_RX_CACHE_BUF_SIZE] = {0};
#endif // BSP_USING_UART3
#if defined(BSP_USING_UART4)
    extern UART_HandleTypeDef huart4;
    static uint8_t uart4_rx_buf[UART4_RX_BUF_SIZE] = {0};
    static uint8_t uart4_tx_buf[UART4_TX_BUF_SIZE] = {0};
    static uint8_t uart4_rx_cache_buf[UART4_RX_CACHE_BUF_SIZE] = {0};
#endif // BSP_USING_UART4
#if defined(BSP_USING_UART5)
    extern UART_HandleTypeDef huart5;
    static uint8_t uart5_rx_buf[UART5_RX_BUF_SIZE] = {0};
    static uint8_t uart5_tx_buf[UART5_TX_BUF_SIZE] = {0};
    static uint8_t uart5_rx_cache_buf[UART5_RX_CACHE_BUF_SIZE] = {0};
#endif // BSP_USING_UART5
#if defined(BSP_USING_UART6)
    extern UART_HandleTypeDef huart6;
    static uint8_t uart6_rx_buf[UART6_RX_BUF_SIZE] = {0};
    static uint8_t uart6_tx_buf[UART6_TX_BUF_SIZE] = {0};
    static uint8_t uart6_rx_cache_buf[UART6_RX_CACHE_BUF_SIZE] = {0};
#endif // BSP_USING_UART6
#if defined(BSP_USING_UART7)
    extern UART_HandleTypeDef huart7;
    static uint8_t uart7_rx_buf[UART7_RX_BUF_SIZE] = {0};
    static uint8_t uart7_tx_buf[UART7_TX_BUF_SIZE] = {0};
    static uint8_t uart7_rx_cache_buf[UART7_RX_CACHE_BUF_SIZE] = {0};
#endif // BSP_USING_UART7
#if defined(BSP_USING_UART8)
    extern UART_HandleTypeDef huart8;
    static uint8_t uart8_rx_buf[UART8_RX_BUF_SIZE] = {0};
    static uint8_t uart8_tx_buf[UART8_TX_BUF_SIZE] = {0};
    static uint8_t uart8_rx_cache_buf[UART8_RX_CACHE_BUF_SIZE] = {0};
#endif // BSP_USING_UART8
#if defined(BSP_USING_LPUART1)
    extern UART_HandleTypeDef lphuart1;
    static uint8_t lpuart1_rx_buf[LPUART1_RX_BUF_SIZE] = {0};
    static uint8_t lpuart1_tx_buf[LPUART1_TX_BUF_SIZE] = {0};
    static uint8_t lpuart1_rx_cache_buf[LPUART1_RX_CACHE_BUF_SIZE] = {0};
#endif // BSP_USING_LPUART1

/* 平台专有驱动结构体变量定义 */
static struct stm32_uart stm_uart_drv[UART_INDEX_MAX] =
{
#ifdef BSP_USING_UART1
    {
        .huart          = &huart1,
        .rx_cache_buf   = uart1_rx_cache_buf,
        .rx_cache_bufsz = UART1_RX_CACHE_BUF_SIZE,
        .port           = NULL,
    },
#endif

#ifdef BSP_USING_UART2
    {
        .huart          = &huart2,
        .rx_cache_buf   = uart2_rx_cache_buf,
        .rx_cache_bufsz = UART2_RX_CACHE_BUF_SIZE,
        .port           = NULL,
    },
#endif

#ifdef BSP_USING_UART3
    {
        .huart          = &huart3,
        .rx_cache_buf   = uart3_rx_cache_buf,
        .rx_cache_bufsz = UART3_RX_CACHE_BUF_SIZE,
        .port           = NULL,
    },
#endif

#ifdef BSP_USING_UART4
    {
        .huart          = &huart4,
        .rx_cache_buf   = uart4_rx_cache_buf,
        .rx_cache_bufsz = UART4_RX_CACHE_BUF_SIZE,
        .port           = NULL,
    },
#endif
#ifdef BSP_USING_UART5
    {
        .huart          = &huart5,
        .rx_cache_buf   = uart5_rx_cache_buf,
        .rx_cache_bufsz = UART5_RX_CACHE_BUF_SIZE,
        .port           = NULL,
    },
#endif
#ifdef BSP_USING_UART6
    {
        .huart          = &huart6,
        .rx_cache_buf   = uart6_rx_cache_buf,
        .rx_cache_bufsz = UART6_RX_CACHE_BUF_SIZE,
        .port           = NULL,
    },
#endif
#ifdef BSP_USING_UART7
    {
        .huart          = &huart7,
        .rx_cache_buf   = uart7_rx_cache_buf,
        .rx_cache_bufsz = UART7_RX_CACHE_BUF_SIZE,
        .port           = NULL,
    },
#endif
#ifdef BSP_USING_UART8
    {
        .huart          = &huart8,
        .rx_cache_buf   = uart8_rx_cache_buf,
        .rx_cache_bufsz = UART8_RX_CACHE_BUF_SIZE,
        .port           = NULL,
    },
#endif
#ifdef BSP_USING_LPUART1
    {
        .huart          = &lphuart1,
        .rx_cache_buf   = lpuart1_rx_cache_buf,
        .rx_cache_bufsz = LPUART1_RX_CACHE_BUF_SIZE,
        .port           = NULL,
    },
#endif
};

/* Function prototypes */
static int32_t STM32_UART_Init(Uart_t *port);
static int32_t STM32_UART_Transmit(Uart_t *port, const void *buf, size_t size);
static int32_t STM32_UART_StartReceive(Uart_t *port);
static int32_t STM32_UART_Config(Uart_t *port, struct uart_configure *cfg);
static bool    STM32_UART_TxIsBusy(struct uart *port);
static struct stm32_uart *stm32_uart_from_handle(const UART_HandleTypeDef *huart);

/* Serial operations structure */
static const Uart_Ops_t stm_uart_ops = {
    .init = STM32_UART_Init,
    .send = STM32_UART_Transmit,
    .configure = STM32_UART_Config,
    .start_rx = STM32_UART_StartReceive,
    .tx_is_busy = STM32_UART_TxIsBusy,
};

/* Private function prototypes -----------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

/**
  * @brief  STM32 USART initialization function
  * @param  port: Pointer to uart device
  * @retval 0 on success, negative error code on failure
  */
static int32_t STM32_UART_Init(Uart_t *port)
{
    return 0;
}

/**
  * @brief  Configure UART parameters (not yet implemented)
  * @param  port: Pointer to uart device
  * @param  cfg: Pointer to configuration structure
  * @retval -ERR_NOSYS (runtime re-configuration not implemented)
  */
static int32_t STM32_UART_Config(Uart_t *port, struct uart_configure *cfg)
{
    (void)port;
    (void)cfg;
    return -ERR_NOSYS;
}

static bool STM32_UART_TxIsBusy(struct uart *port)
{
    struct stm32_uart *stm_uart = (struct stm32_uart *)port->prv_data;

    return (stm_uart->huart->gState == HAL_UART_STATE_BUSY_TX) ? true : false;
}

/**
  * @brief  UART transmit using DMA or interrupt mode
  * @param  port: Pointer to uart device
  * @param  buf: Pointer to data buffer
  * @param  size: Number of bytes to transmit
  * @retval 0 on success, negative error code on failure
  */
static int32_t STM32_UART_Transmit(Uart_t *port, const void *buf, size_t size)
{
    struct stm32_uart *stm_uart;
    HAL_StatusTypeDef status;
    const uint8_t *data;

    /* port, buf and size are guaranteed valid by start_transfer (the sole caller):
     *   - port:    non-NULL, validated upstream
     *   - buf:     points into the static TX FIFO buffer, never NULL
     *   - size:    start_transfer skips the call when len==0
     *   - prv_data: always set to a static stm_uart_drv[] element in BSP_UART_Init
     * Only guard against a size that would overflow the HAL uint16_t parameter. */
    if (size > (size_t)UINT16_MAX) {
        return -ERR_INVAL;
    }

    stm_uart = (struct stm32_uart *)port->prv_data;
    data     = (const uint8_t *)buf;

    if (stm_uart->huart->hdmatx != NULL) {
        status = HAL_UART_Transmit_DMA(stm_uart->huart, data, (uint16_t)size);
    } else {
        status = HAL_UART_Transmit_IT(stm_uart->huart, data, (uint16_t)size);
    }

    if (status != HAL_OK) {
        log_e("HAL_UART_Transmit_DMA/IT failed");
        return -ERR_IO;
    }

    return 0;
}

/**
  * @brief  Start UART DMA/Interrupt reception
  * @param  port: Pointer to uart device
  * @retval 0 on success, negative error code on failure
  */
static int32_t STM32_UART_StartReceive(Uart_t *port)
{
    struct stm32_uart *stm_uart = (struct stm32_uart *)port->prv_data;
    HAL_StatusTypeDef status;

    // 重置位置计数器
    stm_uart->last_pos = 0;

    // 开启中断接收
    if (stm_uart->huart->hdmarx != NULL) {
        /* 使用DMA模式接收 */
        status = HAL_UARTEx_ReceiveToIdle_DMA(stm_uart->huart,
                                             stm_uart->rx_cache_buf,
                                             stm_uart->rx_cache_bufsz);
    } else {
        /* 使用中断模式接收，单字节接收 */
        status = HAL_UARTEx_ReceiveToIdle_IT(stm_uart->huart, stm_uart->rx_cache_buf, stm_uart->rx_cache_bufsz);
    }

    if (status != HAL_OK) {
        log_e("HAL_UART_Receive_DMA/IT failed");
        return -ERR_IO;
    }

    return 0;
}

/**
  * @brief  Initialize STM32 USART hardware
  * @retval 0 on success, negative error code on failure
  */
int BSP_UART_Init(void)
{
    int32_t ret;
    Uart_t *port;
    uint8_t i;

    /* Compile-time guard: bsp_conf.h enabled UART count must equal DEV_UART_MAX */
    _Static_assert((int32_t)UART_INDEX_MAX == (int32_t)DEV_UART_MAX,
                   "BSP UART count (UART_INDEX_MAX) != DEV_UART_MAX in dev_cfg.h");

    /* Bind platform driver context and ops to each generic port */
    for (i = 0U; i < (uint8_t)UART_INDEX_MAX; i++) {
        port = Uart_Find(i);
        if (port == NULL) {
            return -ERR_IO;
        }
        port->prv_data       = (void *)&stm_uart_drv[i];
        port->ops            = &stm_uart_ops;
        stm_uart_drv[i].port = port;
    }

    /* Register each port with its static backing buffers (also inits FIFOs) */
#if defined(BSP_USING_UART1)
    ret = Uart_Register(Uart_Find(UART1_INDEX), (uint8_t)UART1_INDEX,
                        uart1_rx_buf, (uint16_t)sizeof(uart1_rx_buf),
                        uart1_tx_buf, (uint16_t)sizeof(uart1_tx_buf));
    if (ret != 0) { return (int)ret; }
#endif
#if defined(BSP_USING_UART2)
    ret = Uart_Register(Uart_Find(UART2_INDEX), (uint8_t)UART2_INDEX,
                        uart2_rx_buf, (uint16_t)sizeof(uart2_rx_buf),
                        uart2_tx_buf, (uint16_t)sizeof(uart2_tx_buf));
    if (ret != 0) { return (int)ret; }
#endif
#if defined(BSP_USING_UART3)
    ret = Uart_Register(Uart_Find(UART3_INDEX), (uint8_t)UART3_INDEX,
                        uart3_rx_buf, (uint16_t)sizeof(uart3_rx_buf),
                        uart3_tx_buf, (uint16_t)sizeof(uart3_tx_buf));
    if (ret != 0) { return (int)ret; }
#endif
#if defined(BSP_USING_UART4)
    ret = Uart_Register(Uart_Find(UART4_INDEX), (uint8_t)UART4_INDEX,
                        uart4_rx_buf, (uint16_t)sizeof(uart4_rx_buf),
                        uart4_tx_buf, (uint16_t)sizeof(uart4_tx_buf));
    if (ret != 0) { return (int)ret; }
#endif
#if defined(BSP_USING_UART5)
    ret = Uart_Register(Uart_Find(UART5_INDEX), (uint8_t)UART5_INDEX,
                        uart5_rx_buf, (uint16_t)sizeof(uart5_rx_buf),
                        uart5_tx_buf, (uint16_t)sizeof(uart5_tx_buf));
    if (ret != 0) { return (int)ret; }
#endif
#if defined(BSP_USING_UART6)
    ret = Uart_Register(Uart_Find(UART6_INDEX), (uint8_t)UART6_INDEX,
                        uart6_rx_buf, (uint16_t)sizeof(uart6_rx_buf),
                        uart6_tx_buf, (uint16_t)sizeof(uart6_tx_buf));
    if (ret != 0) { return (int)ret; }
#endif
#if defined(BSP_USING_UART7)
    ret = Uart_Register(Uart_Find(UART7_INDEX), (uint8_t)UART7_INDEX,
                        uart7_rx_buf, (uint16_t)sizeof(uart7_rx_buf),
                        uart7_tx_buf, (uint16_t)sizeof(uart7_tx_buf));
    if (ret != 0) { return (int)ret; }
#endif
#if defined(BSP_USING_UART8)
    ret = Uart_Register(Uart_Find(UART8_INDEX), (uint8_t)UART8_INDEX,
                        uart8_rx_buf, (uint16_t)sizeof(uart8_rx_buf),
                        uart8_tx_buf, (uint16_t)sizeof(uart8_tx_buf));
    if (ret != 0) { return (int)ret; }
#endif
#if defined(BSP_USING_LPUART1)
    ret = Uart_Register(Uart_Find(LPUART1_INDEX), (uint8_t)LPUART1_INDEX,
                        lpuart1_rx_buf, (uint16_t)sizeof(lpuart1_rx_buf),
                        lpuart1_tx_buf, (uint16_t)sizeof(lpuart1_tx_buf));
    if (ret != 0) { return (int)ret; }
#endif
    return 0;
}

/**
  * @brief  Get stm32_uart context from HAL UART handle
  * @param  huart HAL UART handle pointer
  * @retval stm32_uart pointer, NULL if not found
  */
static struct stm32_uart *stm32_uart_from_handle(const UART_HandleTypeDef *huart)
{
    if (huart == NULL) {
        return NULL;
    }

#ifdef BSP_USING_UART1
    if (huart == stm_uart_drv[UART1_INDEX].huart) {
        return &stm_uart_drv[UART1_INDEX];
    }
#endif
#ifdef BSP_USING_UART2
    if (huart == stm_uart_drv[UART2_INDEX].huart) {
        return &stm_uart_drv[UART2_INDEX];
    }
#endif
#ifdef BSP_USING_UART3
    if (huart == stm_uart_drv[UART3_INDEX].huart) {
        return &stm_uart_drv[UART3_INDEX];
    }
#endif
#ifdef BSP_USING_UART4
    if (huart == stm_uart_drv[UART4_INDEX].huart) {
        return &stm_uart_drv[UART4_INDEX];
    }
#endif
#ifdef BSP_USING_UART5
    if (huart == stm_uart_drv[UART5_INDEX].huart) {
        return &stm_uart_drv[UART5_INDEX];
    }
#endif
#ifdef BSP_USING_UART6
    if (huart == stm_uart_drv[UART6_INDEX].huart) {
        return &stm_uart_drv[UART6_INDEX];
    }
#endif
#ifdef BSP_USING_UART7
    if (huart == stm_uart_drv[UART7_INDEX].huart) {
        return &stm_uart_drv[UART7_INDEX];
    }
#endif
#ifdef BSP_USING_UART8
    if (huart == stm_uart_drv[UART8_INDEX].huart) {
        return &stm_uart_drv[UART8_INDEX];
    }
#endif
#ifdef BSP_USING_LPUART1
    if (huart == stm_uart_drv[LPUART1_INDEX].huart) {
        return &stm_uart_drv[LPUART1_INDEX];
    }
#endif

    return NULL;
}

/* HAL Callback Functions ----------------------------------------------------*/
/**
  * @brief  Tx Transfer completed callback
  * @param  huart: Pointer to UART handle
  * @retval None
  */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    struct stm32_uart *stm_uart = stm32_uart_from_handle(huart);

    if ((stm_uart != NULL) && (stm_uart->port != NULL)) {
        Uart_TxIsrHook(stm_uart->port);
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
    struct stm32_uart *stm_uart = stm32_uart_from_handle(huart);
    uint16_t process_size;

    if ((stm_uart == NULL) || (stm_uart->port == NULL)) {
        return;
    }

    /* Guard against underflow (should not occur in DMA_NORMAL mode) */
    if (Size < stm_uart->last_pos) {
        stm_uart->last_pos = 0U;
        (void)STM32_UART_StartReceive(stm_uart->port);
        return;
    }

    process_size = Size - stm_uart->last_pos;

    if (huart->RxEventType == HAL_UART_RXEVENT_HT) {
        /* Half-transfer: forward new data, keep last_pos for the TC/IDLE half */
        if (process_size > 0U) {
            Uart_RxIsrHook(stm_uart->port,
                           stm_uart->rx_cache_buf + stm_uart->last_pos,
                           process_size);
            stm_uart->last_pos = Size;
        }
    } else if ((huart->RxEventType == HAL_UART_RXEVENT_TC) ||
               (huart->RxEventType == HAL_UART_RXEVENT_IDLE)) {
        /* Transfer complete or idle: forward remaining data, then restart.
         * STM32_UART_StartReceive resets last_pos to 0 internally. */
        if (process_size > 0U) {
            Uart_RxIsrHook(stm_uart->port,
                           stm_uart->rx_cache_buf + stm_uart->last_pos,
                           process_size);
        }
        (void)STM32_UART_StartReceive(stm_uart->port);
    } else {
        /* Unknown event type: restart to recover */
        stm_uart->last_pos = 0U;
        (void)STM32_UART_StartReceive(stm_uart->port);
    }
}

/**
  * @brief  UART error callback.
  * @param  huart UART handle.
  * @retval None
  */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    struct stm32_uart *stm_uart = stm32_uart_from_handle(huart);

    if ((stm_uart == NULL) || (stm_uart->port == NULL)) {
        return;
    }

    log_d("UART errno code = %d", huart->ErrorCode);

    (void)STM32_UART_StartReceive(stm_uart->port);
}

#endif /* defined(HAL_UART_MODULE_ENABLED) */
