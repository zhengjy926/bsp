/**
  ******************************************************************************
  * @copyright   : Copyright To Hangzhou Dinova EP Technology Co.,Ltd
  * @file        : usart_config.h
  * @author      : ZJY
  * @version     : V1.0
  * @date        : 2025-01-27
  * @brief       : USART configuration header file
  * @attention   : None
  ******************************************************************************
  */
#ifndef __USART_CONFIG_H__
#define __USART_CONFIG_H__

#ifdef __cplusplus
 extern "C" {
#endif /* __cplusplus */

/* Includes ------------------------------------------------------------------*/
#include "board.h"

#if defined(SOC_SERIES_STM32F1)
    #include "stm32f1xx.h"
#elif defined(SOC_SERIES_STM32F4)
    #include "stm32f4xx.h"
#elif defined(SOC_SERIES_STM32G4)
    #include "stm32g4xx.h"
#else
#error "Please select first the soc series used in your application!"    
#endif

/* Exported define -----------------------------------------------------------*/
#if defined(HAL_UART_MODULE_ENABLED)
    #define BSP_USING_UART1
    #define BSP_UART1_RX_USING_DMA
    #define BSP_UART1_TX_USING_DMA

    #define BSP_USING_UART2
    #define BSP_UART2_RX_USING_DMA
    #define BSP_UART2_TX_USING_DMA

    #define BSP_USING_UART3
    #define BSP_UART3_RX_USING_DMA
    #define BSP_UART3_TX_USING_DMA

    #define BSP_USING_UART4
    #define BSP_UART4_RX_USING_DMA
    #define BSP_UART4_TX_USING_DMA

    // #define BSP_USING_UART5
    // #define BSP_UART5_RX_USING_DMA
    // #define BSP_UART5_TX_USING_DMA

    // #define BSP_USING_UART6
    // #define BSP_UART6_RX_USING_DMA
    // #define BSP_UART6_TX_USING_DMA

    // #define BSP_USING_UART7
    // #define BSP_UART7_RX_USING_DMA
    // #define BSP_UART7_TX_USING_DMA

/* UART1 Configuration */
#if defined(BSP_USING_UART1)
    #define UART1_CONFIG                                                \
        {                                                               \
            .name = "uart1",                                            \
            .Instance = USART1,                                         \
            .irq_type = USART1_IRQn,                                    \
            .rx_dma   = {                                               \
                .Instance = DMA2_Stream2,                               \
                .channel  = DMA_CHANNEL_4,                              \
                .dma_irq  = DMA2_Stream2_IRQn                           \
            },                                                          \
            .tx_dma   = {                                               \
                .Instance = DMA2_Stream7,                               \
                .channel  = DMA_CHANNEL_4,                              \
                .dma_irq  = DMA2_Stream7_IRQn                           \
            },                                                          \
            .using_rx_dma = 1,                                          \
            .using_tx_dma = 1,                                          \
        }

    #define UART1_RX_BUF_SIZE           (1024)
    #define UART1_TX_BUF_SIZE           (1024)
    #define UART1_RX_TEMP_BUF_SIZE      (128)
        
    #define UART1_DMA_RX_IRQHandler     DMA2_Stream2_IRQHandler
    #define UART1_DMA_TX_IRQHandler     DMA2_Stream7_IRQHandler

    #define UART1_GPIO_PORT             GPIOA
    #define UART1_TX_PIN                GPIO_PIN_9
    #define UART1_RX_PIN                GPIO_PIN_10
#endif // BSP_USING_UART1


#if defined(BSP_USING_UART2)
    #define UART2_CONFIG                                                \
        {                                                               \
            .name = "uart2",                                            \
            .Instance = USART2,                                         \
            .irq_type = USART2_IRQn,                                    \
            .rx_dma   = {                                               \
                .Instance = DMA1_Stream5,                               \
                .channel  = DMA_CHANNEL_4,                              \
                .dma_irq  = DMA1_Stream5_IRQn                           \
            },                                                          \
            .tx_dma   = {                                               \
                .Instance = DMA1_Stream6,                               \
                .channel  = DMA_CHANNEL_4,                              \
                .dma_irq  = DMA1_Stream6_IRQn                           \
            },                                                          \
            .using_rx_dma = 1,                                          \
            .using_tx_dma = 1,                                          \
        }

    #define UART2_RX_BUF_SIZE           (512)
    #define UART2_TX_BUF_SIZE           (512)
    #define UART2_RX_TEMP_BUF_SIZE      (128)
        
    #define UART2_DMA_RX_IRQHandler     DMA1_Stream5_IRQHandler
    #define UART2_DMA_TX_IRQHandler     DMA1_Stream6_IRQHandler

    #define UART2_GPIO_PORT             GPIOA
    #define UART2_TX_PIN                GPIO_PIN_2
    #define UART2_RX_PIN                GPIO_PIN_3
#endif // BSP_USING_UART2

#if defined(BSP_USING_UART3)
    #define UART3_CONFIG                                                \
        {                                                               \
            .name = "uart3",                                            \
            .Instance = USART3,                                         \
            .irq_type = USART3_IRQn,                                    \
            .rx_dma   = {                                               \
                .Instance = DMA1_Stream1,                               \
                .channel  = DMA_CHANNEL_4,                              \
                .dma_irq  = DMA1_Stream1_IRQn                           \
            },                                                          \
            .tx_dma   = {                                               \
                .Instance = DMA1_Stream3,                               \
                .channel  = DMA_CHANNEL_4,                              \
                .dma_irq  = DMA1_Stream3_IRQn                           \
            },                                                          \
            .using_rx_dma = 1,                                          \
            .using_tx_dma = 1,                                          \
        }

    #define UART3_RX_BUF_SIZE           (1024)
    #define UART3_TX_BUF_SIZE           (1024)
    #define UART3_RX_TEMP_BUF_SIZE      (128)
        
    #define UART3_DMA_RX_IRQHandler     DMA1_Stream1_IRQHandler
    #define UART3_DMA_TX_IRQHandler     DMA1_Stream3_IRQHandler

    #define UART3_GPIO_PORT             GPIOB
    #define UART3_TX_PIN                GPIO_PIN_10
    #define UART3_RX_PIN                GPIO_PIN_11
#endif // BSP_USING_UART3

#if defined(BSP_USING_UART4)
    #define UART4_CONFIG                                                \
        {                                                               \
            .name = "uart4",                                            \
            .Instance = UART4,                                          \
            .irq_type = UART4_IRQn,                                     \
            .rx_dma   = {                                               \
                .Instance = DMA1_Stream2,                               \
                .channel  = DMA_CHANNEL_4,                              \
                .dma_irq  = DMA1_Stream2_IRQn                           \
            },                                                          \
            .tx_dma   = {                                               \
                .Instance = DMA1_Stream4,                               \
                .channel  = DMA_CHANNEL_4,                              \
                .dma_irq  = DMA1_Stream4_IRQn                           \
            },                                                          \
            .using_rx_dma = 1,                                          \
            .using_tx_dma = 1,                                          \
        }
    #define UART4_RX_BUF_SIZE           (1024)
    #define UART4_TX_BUF_SIZE           (1024)
    #define UART4_RX_TEMP_BUF_SIZE      (128)
        
    #define UART4_DMA_RX_IRQHandler     DMA1_Stream2_IRQHandler
    #define UART4_DMA_TX_IRQHandler     DMA1_Stream4_IRQHandler

    #define UART4_GPIO_PORT             GPIOA
    #define UART4_TX_PIN                GPIO_PIN_0
    #define UART4_RX_PIN                GPIO_PIN_1
#endif // BSP_USING_UART4
#endif // defined(HAL_UART_MODULE_ENABLED)

/* Exported variable prototypes ----------------------------------------------*/
/* Exported function prototypes ----------------------------------------------*/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __USART_CONFIG_H__ */
