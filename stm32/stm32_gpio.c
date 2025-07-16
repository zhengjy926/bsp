/**
  ******************************************************************************
  * @copyright   : Copyright To Hangzhou Dinova EP Technology Co.,Ltd
  * @file        : xxxx.c
  * @author      : ZJY
  * @version     : V1.0
  * @date        : 20xx-xx-xx
  * @brief       : 
  * @attattention: None
  ******************************************************************************
  * @history     :
  *         V1.0 : 1.xxx
  *
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "stm32_gpio.h"
#include "gpio.h"
#include "string.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/*
 * Combine the port number (port) and the pin number (pin) into an 8-bit
 * unique identifier (pin_id), with the port number in the high 4 bits and
 * the pin number in the low 4 bits
 */
#define PIN_ID(port, pin) (((((port) & 0xFu) << 4) | ((pin) & 0xFu)))

/* Get the port number from the pin_id */
#define PIN_PORT(pin_id) ((uint8_t)(((pin_id) >> 4) & 0xFu))
/* Get the pin number from the pin_id */
#define PIN_NO(pin_id) ((uint8_t)((pin_id) & 0xFu))

#define PIN_STPORT(pin_id) ((GPIO_TypeDef *)(GPIOA_BASE + (0x400u * PIN_PORT(pin_id))))
#define PIN_STPIN(pin_id) ((uint16_t)(1u << PIN_NO(pin_id)))

#if defined(GPIOZ)
#define __STM32_PORT_MAX 12u
#elif defined(GPIOK)
#define __STM32_PORT_MAX 11u
#elif defined(GPIOJ)
#define __STM32_PORT_MAX 10u
#elif defined(GPIOI)
#define __STM32_PORT_MAX 9u
#elif defined(GPIOH)
#define __STM32_PORT_MAX 8u
#elif defined(GPIOG)
#define __STM32_PORT_MAX 7u
#elif defined(GPIOF)
#define __STM32_PORT_MAX 6u
#elif defined(GPIOE)
#define __STM32_PORT_MAX 5u
#elif defined(GPIOD)
#define __STM32_PORT_MAX 4u
#elif defined(GPIOC)
#define __STM32_PORT_MAX 3u
#elif defined(GPIOB)
#define __STM32_PORT_MAX 2u
#elif defined(GPIOA)
#define __STM32_PORT_MAX 1u
#else
#define __STM32_PORT_MAX 0u
#error Unsupported STM32 GPIO peripheral.
#endif

#define PIN_STPORT_MAX __STM32_PORT_MAX
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Exported variables  -------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
static int stm32_gpio_get(const char *name)
{
    size_t pin_id, name_len = 0;
    int hw_port_num, hw_pin_num = 0;

    name_len = strlen(name);

    if ((name_len < 4) || (name_len >= 6))
        return -EINVAL;
    
    if ((name[0] != 'P') || (name[2] != '.'))
        return -EINVAL;

    if ((name[1] >= 'A') && (name[1] <= 'Z')) {
        hw_port_num = (int)(name[1] - 'A');
    } else {
        return -EINVAL;
    }

    for (size_t i = 3; i < name_len; i++) {
        hw_pin_num *= 10;
        hw_pin_num += name[i] - '0';
    }

    pin_id = PIN_ID(hw_port_num, hw_pin_num);

    return pin_id;
}

static void stm32_gpio_mode(size_t pin_id, size_t mode, size_t pull_resistor)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    if (PIN_PORT(pin_id) >= PIN_STPORT_MAX)
    {
        return;
    }

    /* Configure GPIO_InitStructure */
    GPIO_InitStruct.Pin = PIN_STPIN(pin_id);
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    if (mode == PIN_MODE_OUTPUT_PP) {
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    } else if (mode == PIN_MODE_OUTPUT_OD) {
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    } else if (mode == PIN_MODE_INPUT) {
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    }
    
    if (pull_resistor == PIN_PULL_RESISTOR_UP) {
        GPIO_InitStruct.Pull = GPIO_PULLUP;
    } else if (pull_resistor == PIN_PULL_RESISTOR_DOWN) {
        GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    } else {
        GPIO_InitStruct.Pull = GPIO_NOPULL;
    }

    HAL_GPIO_Init(PIN_STPORT(pin_id), &GPIO_InitStruct);
}


static void stm32_gpio_write(size_t pin, uint8_t value)
{
    GPIO_TypeDef *gpio_port;
    uint16_t gpio_pin;

    if (PIN_PORT(pin) < PIN_STPORT_MAX)
    {
        gpio_port = PIN_STPORT(pin);
        gpio_pin = PIN_STPIN(pin);
        HAL_GPIO_WritePin(gpio_port, gpio_pin, (GPIO_PinState)value);
    }
}

static uint8_t stm32_gpio_read(size_t pin)
{
    GPIO_TypeDef *gpio_port;
    uint16_t gpio_pin;
    int value = PIN_LOW;

    if (PIN_PORT(pin) < PIN_STPORT_MAX)
    {
        gpio_port = PIN_STPORT(pin);
        gpio_pin = PIN_STPIN(pin);
        value = HAL_GPIO_ReadPin(gpio_port, gpio_pin);
    }
    
    return value;
}

const static struct gpio_ops _stm32_pin_ops =
{
    stm32_gpio_mode,
    stm32_gpio_write,
    stm32_gpio_read,
    NULL,
    NULL,
    NULL,
    stm32_gpio_get,
};

void stm32_gpio_init(void)
{
#if defined(__HAL_RCC_GPIOA_CLK_ENABLE)
    __HAL_RCC_GPIOA_CLK_ENABLE();
#endif

#if defined(__HAL_RCC_GPIOB_CLK_ENABLE)
    __HAL_RCC_GPIOB_CLK_ENABLE();
#endif

#if defined(__HAL_RCC_GPIOC_CLK_ENABLE)
    __HAL_RCC_GPIOC_CLK_ENABLE();
#endif

#if defined(__HAL_RCC_GPIOD_CLK_ENABLE)
    __HAL_RCC_GPIOD_CLK_ENABLE();
#endif

#if defined(__HAL_RCC_GPIOE_CLK_ENABLE)
    __HAL_RCC_GPIOE_CLK_ENABLE();
#endif

#if defined(__HAL_RCC_GPIOF_CLK_ENABLE)
    __HAL_RCC_GPIOF_CLK_ENABLE();
#endif

#if defined(__HAL_RCC_GPIOG_CLK_ENABLE)
    #ifdef SOC_SERIES_STM32L4
        HAL_PWREx_EnableVddIO2();
    #endif
    __HAL_RCC_GPIOG_CLK_ENABLE();
#endif

#if defined(__HAL_RCC_GPIOH_CLK_ENABLE)
    __HAL_RCC_GPIOH_CLK_ENABLE();
#endif

#if defined(__HAL_RCC_GPIOI_CLK_ENABLE)
    __HAL_RCC_GPIOI_CLK_ENABLE();
#endif

#if defined(__HAL_RCC_GPIOJ_CLK_ENABLE)
    __HAL_RCC_GPIOJ_CLK_ENABLE();
#endif

#if defined(__HAL_RCC_GPIOK_CLK_ENABLE)
    __HAL_RCC_GPIOK_CLK_ENABLE();
#endif
    
    gpio_register(&_stm32_pin_ops);
}
/* Private functions ---------------------------------------------------------*/


