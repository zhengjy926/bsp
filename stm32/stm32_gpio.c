/**
  ******************************************************************************
  * @copyright   : Copyright To Hangzhou Dinova EP Technology Co.,Ltd
  * @file        : gpio.c
  * @author      : ZJY
  * @version     : V1.0
  * @date        : 2024-09-26
  * @brief       : STM32 GPIO driver implementation
  * @attention   : None
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

#if defined(SOC_SERIES_STM32F1)
    #include "stm32f1xx.h"
#elif defined(SOC_SERIES_STM32F4)
    #include "stm32f4xx.h"
#elif defined(SOC_SERIES_STM32G4)
    #include "stm32g4xx.h"
#else
#error "Please select first the soc series used in your application!"    
#endif

#include <string.h>
/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/
/*
 * Combine the port number (port) and the pin number (pin) into an 8-bit
 * unique identifier (pin_id), with the port number in the high 4 bits and
 * the pin number in the low 4 bits
 */
#define PIN_ID(port, pin)           (((((port) & 0xFu) << 4) | ((pin) & 0xFu)))

/* Get the port index from the pin_id */
#define PIN_GET_PORT_IDX(pin_id)    ((uint8_t)(((pin_id) >> 4) & 0xFu))

/* Get the pin index from the pin_id */
#define PIN_GET_PIN_IDX(pin_id)     ((uint8_t)((pin_id) & 0xFu))

/* Pin mask on port(eg. GPIO_PIN_0, GPIO_PIN_1) */
#define PIN_MASK(pin_id)            ((uint16_t)(1u << PIN_GET_PIN_IDX(pin_id)))

/* Private variables ---------------------------------------------------------*/
static GPIO_TypeDef * const gpio_ports[] = {
  GPIOA
#ifdef GPIOB
, GPIOB
#endif
#ifdef GPIOC
, GPIOC
#endif
#ifdef GPIOD
, GPIOD
#endif
#ifdef GPIOE
, GPIOE
#endif
#ifdef GPIOF
, GPIOF
#endif
#ifdef GPIOG
, GPIOG
#endif
#ifdef GPIOH
, GPIOH
#endif
#ifdef GPIOI
, GPIOI
#endif
#ifdef GPIOJ
, GPIOJ
#endif
#ifdef GPIOK
, GPIOK
#endif
#ifdef GPIOM
, GPIOM
#endif
#ifdef GPION
, GPION
#endif
#ifdef GPIOO
, GPIOO
#endif
#ifdef GPIOP
, GPIOP
#endif
#ifdef GPIOZ
, GPIOZ
#endif
};

/* Maximum number of pins */
#define GPIO_MAX_PINS_NUM       ((sizeof(gpio_ports) / sizeof(gpio_ports[0])) * 16U)

static const IRQn_Type pin_irq_map[] = {
#if defined(SOC_SERIES_STM32F0) || defined(SOC_SERIES_STM32L0) || defined(SOC_SERIES_STM32G0)
    EXTI0_1_IRQn,           /* GPIO_PIN_0 */
    EXTI0_1_IRQn,           /* GPIO_PIN_1 */
    EXTI2_3_IRQn,           /* GPIO_PIN_2 */
    EXTI2_3_IRQn,           /* GPIO_PIN_3 */
    EXTI4_15_IRQn,          /* GPIO_PIN_4 */
    EXTI4_15_IRQn,          /* GPIO_PIN_5 */
    EXTI4_15_IRQn,          /* GPIO_PIN_6 */
    EXTI4_15_IRQn,          /* GPIO_PIN_7 */
    EXTI4_15_IRQn,          /* GPIO_PIN_8 */
    EXTI4_15_IRQn,          /* GPIO_PIN_9 */
    EXTI4_15_IRQn,          /* GPIO_PIN_10 */
    EXTI4_15_IRQn,          /* GPIO_PIN_11 */
    EXTI4_15_IRQn,          /* GPIO_PIN_12 */
    EXTI4_15_IRQn,          /* GPIO_PIN_13 */
    EXTI4_15_IRQn,          /* GPIO_PIN_14 */
    EXTI4_15_IRQn,          /* GPIO_PIN_15 */
#elif defined(SOC_SERIES_STM32MP1) || defined(SOC_SERIES_STM32L5) || defined(SOC_SERIES_STM32U5)
    EXTI0_IRQn,             /* GPIO_PIN_0 */
    EXTI1_IRQn,             /* GPIO_PIN_1 */
    EXTI2_IRQn,             /* GPIO_PIN_2 */
    EXTI3_IRQn,             /* GPIO_PIN_3 */
    EXTI4_IRQn,             /* GPIO_PIN_4 */
    EXTI5_IRQn,             /* GPIO_PIN_5 */
    EXTI6_IRQn,             /* GPIO_PIN_6 */
    EXTI7_IRQn,             /* GPIO_PIN_7 */
    EXTI8_IRQn,             /* GPIO_PIN_8 */
    EXTI9_IRQn,             /* GPIO_PIN_9 */
    EXTI10_IRQn,            /* GPIO_PIN_10 */
    EXTI11_IRQn,            /* GPIO_PIN_11 */
    EXTI12_IRQn,            /* GPIO_PIN_12 */
    EXTI13_IRQn,            /* GPIO_PIN_13 */
    EXTI14_IRQn,            /* GPIO_PIN_14 */
    EXTI15_IRQn,            /* GPIO_PIN_15 */
#elif defined(SOC_SERIES_STM32F3)
    EXTI0_IRQn,             /* GPIO_PIN_0 */
    EXTI1_IRQn,             /* GPIO_PIN_1 */
    EXTI2_TSC_IRQn,         /* GPIO_PIN_2 */
    EXTI3_IRQn,             /* GPIO_PIN_3 */
    EXTI4_IRQn,             /* GPIO_PIN_4 */
    EXTI9_5_IRQn,           /* GPIO_PIN_5 */
    EXTI9_5_IRQn,           /* GPIO_PIN_6 */
    EXTI9_5_IRQn,           /* GPIO_PIN_7 */
    EXTI9_5_IRQn,           /* GPIO_PIN_8 */
    EXTI9_5_IRQn,           /* GPIO_PIN_9 */
    EXTI15_10_IRQn,         /* GPIO_PIN_10 */
    EXTI15_10_IRQn,         /* GPIO_PIN_11 */
    EXTI15_10_IRQn,         /* GPIO_PIN_12 */
    EXTI15_10_IRQn,         /* GPIO_PIN_13 */
    EXTI15_10_IRQn,         /* GPIO_PIN_14 */
    EXTI15_10_IRQn,         /* GPIO_PIN_15 */
#else
    EXTI0_IRQn,             /* GPIO_PIN_0 */
    EXTI1_IRQn,             /* GPIO_PIN_1 */
    EXTI2_IRQn,             /* GPIO_PIN_2 */
    EXTI3_IRQn,             /* GPIO_PIN_3 */
    EXTI4_IRQn,             /* GPIO_PIN_4 */
    EXTI9_5_IRQn,           /* GPIO_PIN_5 */
    EXTI9_5_IRQn,           /* GPIO_PIN_6 */
    EXTI9_5_IRQn,           /* GPIO_PIN_7 */
    EXTI9_5_IRQn,           /* GPIO_PIN_8 */
    EXTI9_5_IRQn,           /* GPIO_PIN_9 */
    EXTI15_10_IRQn,         /* GPIO_PIN_10 */
    EXTI15_10_IRQn,         /* GPIO_PIN_11 */
    EXTI15_10_IRQn,         /* GPIO_PIN_12 */
    EXTI15_10_IRQn,         /* GPIO_PIN_13 */
    EXTI15_10_IRQn,         /* GPIO_PIN_14 */
    EXTI15_10_IRQn,         /* GPIO_PIN_15 */
#endif
};

static struct pin_irq_hdr pin_irq_hdr_tab[] = {
    {-1, 0, NULL, NULL},
    {-1, 0, NULL, NULL},
    {-1, 0, NULL, NULL},
    {-1, 0, NULL, NULL},
    {-1, 0, NULL, NULL},
    {-1, 0, NULL, NULL},
    {-1, 0, NULL, NULL},
    {-1, 0, NULL, NULL},
    {-1, 0, NULL, NULL},
    {-1, 0, NULL, NULL},
    {-1, 0, NULL, NULL},
    {-1, 0, NULL, NULL},
    {-1, 0, NULL, NULL},
    {-1, 0, NULL, NULL},
    {-1, 0, NULL, NULL},
    {-1, 0, NULL, NULL},
};

/* It is used to track which pins' interrupts are currently enabled */
static uint32_t pin_irq_enable_mask = 0;

/* Exported variables  -------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Exported functions --------------------------------------------------------*/
/**
 * @brief Get pin ID from pin name string
 * @param name Pin name string (e.g. "PA.0" for Port A Pin 0)
 * @return Pin ID on success, negative error code on failure
 */
static int stm32_gpio_get(const char *name)
{
    uint32_t pin_id, name_len = 0;
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

    for (uint32_t i = 3; i < name_len; i++) {
        hw_pin_num *= 10;
        hw_pin_num += name[i] - '0';
    }

    pin_id = PIN_ID(hw_port_num, hw_pin_num);

    return pin_id;
}

/**
 * @brief Set GPIO pin mode and pull resistor
 * @param pin_id Pin identifier
 * @param mode Pin mode (INPUT/OUTPUT_PP/OUTPUT_OD)
 * @param pull_resistor Pull resistor configuration
 * @return None
 */
static void stm32_gpio_mode(uint32_t pin_id, pin_mode_t mode, pin_pull_t pull_resistor)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    if (pin_id >= GPIO_MAX_PINS_NUM) { // If pin id is out-of-bounds
        return;
    }
    
    GPIO_TypeDef* port      = gpio_ports[PIN_GET_PORT_IDX(pin_id)];
    uint16_t      pin_mask  = PIN_MASK(pin_id);

    /* Configure GPIO_InitStructure */
    GPIO_InitStruct.Pin   = pin_mask;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    if (mode == PIN_OUTPUT_PP) {
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    } else if (mode == PIN_OUTPUT_OD) {
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    } else {
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    }
    
    if (pull_resistor == PIN_PULL_UP) {
        GPIO_InitStruct.Pull = GPIO_PULLUP;
    } else if (pull_resistor == PIN_PULL_DOWN) {
        GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    } else {
        GPIO_InitStruct.Pull = GPIO_NOPULL;
    }

    HAL_GPIO_Init(port, &GPIO_InitStruct);
}

/**
 * @brief Write digital value to GPIO pin
 * @param pin_id Pin identifier
 * @param value Digital value to write (0 or 1)
 * @return None
 */
static void stm32_gpio_write(uint32_t pin_id, uint8_t value)
{
    if (pin_id >= GPIO_MAX_PINS_NUM) { // If pin id is out-of-bounds
        return;
    }
    
    GPIO_TypeDef* port     = gpio_ports[PIN_GET_PORT_IDX(pin_id)];
    uint16_t      pin_mask = PIN_MASK(pin_id);
    
    if(value) {
        port->BSRR = pin_mask;
    } else {
        port->BSRR = (uint32_t)pin_mask << 16U;
    }
}

/**
 * @brief Read digital value from GPIO pin
 * @param pin_id Pin identifier
 * @return Digital value read from the pin (0 or 1)
 */
static uint8_t stm32_gpio_read(uint32_t pin_id)
{
    uint8_t value = 0;
    
    if (pin_id >= GPIO_MAX_PINS_NUM) { // If pin id is out-of-bounds
        return 0;
    }
    
    GPIO_TypeDef* port     = gpio_ports[PIN_GET_PORT_IDX(pin_id)];
    uint16_t      pin_mask = PIN_MASK(pin_id);
    
    if((port->IDR & pin_mask) != (uint32_t)GPIO_PIN_RESET) {
        value = 1;
    }
    
    return value;
}

/**
 * @brief Attach interrupt handler to GPIO pin
 * @param pin_id Pin identifier
 * @param event Interrupt trigger event (RISING/FALLING/RISING_FALLING)
 * @param hdr Interrupt handler function
 * @param args Argument passed to the interrupt handler
 * @return 0 on success, -ENOSYS if operation is not supported
 */
static int stm32_gpio_attach_irq(uint32_t pin_id, pin_event_t event, void (*hdr)(void *args), void *args)
{
    if (pin_id >= GPIO_MAX_PINS_NUM) { // If pin id is out-of-bounds
        return -ENOSYS;
    }
    
    uint32_t pin_index = PIN_GET_PIN_IDX(pin_id);
    
    uint32_t level = __get_BASEPRI();
    __disable_irq();
    
    if (pin_irq_hdr_tab[pin_index].pin == pin_id &&
        pin_irq_hdr_tab[pin_index].hdr == hdr &&
        pin_irq_hdr_tab[pin_index].event == event &&
        pin_irq_hdr_tab[pin_index].args == args)
    {
        __set_PRIMASK(level);
        return 0;
    }
    if (pin_irq_hdr_tab[pin_index].pin != -1)
    {
        __set_PRIMASK(level);
        return -EBUSY;
    }
    pin_irq_hdr_tab[pin_index].pin = pin_id;
    pin_irq_hdr_tab[pin_index].hdr = hdr;
    pin_irq_hdr_tab[pin_index].event = event;
    pin_irq_hdr_tab[pin_index].args = args;
    __set_PRIMASK(level);

    return 0;
}

/**
 * @brief Detach interrupt handler from GPIO pin
 * @param pin_id Pin identifier
 * @return 0 on success, -ENOSYS if operation is not supported
 */
static int stm32_gpio_deattach_irq(uint32_t pin_id)
{
    if (pin_id >= GPIO_MAX_PINS_NUM) { // If pin id is out-of-bounds
        return -ENOSYS;
    }

    uint32_t pin_index = PIN_GET_PIN_IDX(pin_id);

    uint32_t level = __get_BASEPRI();
    __disable_irq();
    if (pin_irq_hdr_tab[pin_index].pin == -1)
    {
        __set_PRIMASK(level);
        return 0;
    }
    /* Reset to the initial state. */
    pin_irq_hdr_tab[pin_index].pin = -1;
    pin_irq_hdr_tab[pin_index].hdr = NULL;
    pin_irq_hdr_tab[pin_index].event = 0;
    pin_irq_hdr_tab[pin_index].args = NULL;
    __set_PRIMASK(level);

    return 0;
}

/**
 * @brief Enable or disable GPIO interrupt
 * @param pin_id Pin identifier
 * @param enabled 1 to enable, 0 to disable
 * @return 0 on success, -ENOSYS if operation is not supported
 */
static int stm32_gpio_irq_enable(uint32_t pin_id, uint32_t enabled)
{
    uint32_t level;
    GPIO_InitTypeDef GPIO_InitStruct;

    if (pin_id >= GPIO_MAX_PINS_NUM) { // If pin id is out-of-bounds
        return -ENOSYS;
    }
    
    GPIO_TypeDef* port  = gpio_ports[PIN_GET_PORT_IDX(pin_id)];
    uint32_t pin_index  = PIN_GET_PIN_IDX(pin_id);
    uint32_t pin_mask   = PIN_MASK(pin_id);

    if (enabled) {
        level = __get_BASEPRI();
        __disable_irq();

        if (pin_irq_hdr_tab[pin_index].pin == -1)
        {
            /* This pin has not yet been attached with an interrupt handling function. */
            __set_PRIMASK(level);
            return -ENOSYS;
        }

        /* Configure GPIO_InitStructure */
        GPIO_InitStruct.Pin = pin_mask;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        switch (pin_irq_hdr_tab[pin_index].event) {
            case PIN_EVENT_RISING_EDGE:
                GPIO_InitStruct.Pull = GPIO_PULLDOWN;
                GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
                break;
            case PIN_EVENT_FALLING_EDGE:
                GPIO_InitStruct.Pull = GPIO_PULLUP;
                GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
                break;
            case PIN_EVENT_EITHER_EDGE:
                GPIO_InitStruct.Pull = GPIO_NOPULL;
                GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
                break;
            default:
                break;
        }
        HAL_GPIO_Init(port, &GPIO_InitStruct);

        HAL_NVIC_SetPriority(pin_irq_map[pin_index], 5, 0);
        HAL_NVIC_EnableIRQ(pin_irq_map[pin_index]);
        pin_irq_enable_mask |= pin_mask;

        __set_PRIMASK(level);
    }
    else
    {
        level = __get_BASEPRI();
        __disable_irq();
        HAL_GPIO_DeInit(port, pin_mask);

        pin_irq_enable_mask &= ~pin_mask;
        
        /* 
         * Since multiple pins may share the same NVIC interrupt entry(eg.EXTI15_10_IRQn), 
         * it is necessary to first determine whether other pins are also using
         * this interrupt before disabling the interrupt of the specific pin.
         */
#if defined(SOC_SERIES_STM32F0) || defined(SOC_SERIES_STM32G0)
        if ((pin_mask >= GPIO_PIN_0) && (pin_mask <= GPIO_PIN_1))
        {
            if (!(pin_irq_enable_mask & (GPIO_PIN_0 | GPIO_PIN_1)))
            {
                HAL_NVIC_DisableIRQ(pin_irq_map[pin_index]);
            }
        }
        else if ((pin_mask >= GPIO_PIN_2) && (pin_mask <= GPIO_PIN_3))
        {
            if (!(pin_irq_enable_mask & (GPIO_PIN_2 | GPIO_PIN_3)))
            {
                HAL_NVIC_DisableIRQ(pin_irq_map[pin_index]);
            }
        }
        else if ((pin_mask >= GPIO_PIN_4) && (pin_mask <= GPIO_PIN_15))
        {
            if (!(pin_irq_enable_mask & (GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 |
                                         GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15)))
            {
                HAL_NVIC_DisableIRQ(pin_irq_map[pin_index]);
            }
        }
        else
        {
            HAL_NVIC_DisableIRQ(pin_irq_map[pin_index]);
        }
#else
        if ((pin_mask >= GPIO_PIN_5) && (pin_mask <= GPIO_PIN_9))
        {
            if (!(pin_irq_enable_mask & (GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9)))
            {
                HAL_NVIC_DisableIRQ(pin_irq_map[pin_index]);
            }
        }
        else if ((pin_mask >= GPIO_PIN_10) && (pin_mask <= GPIO_PIN_15))
        {
            if (!(pin_irq_enable_mask & (GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15)))
            {
                HAL_NVIC_DisableIRQ(pin_irq_map[pin_index]);
            }
        }
        else
        {
            HAL_NVIC_DisableIRQ(pin_irq_map[pin_index]);
        }
#endif
        __set_PRIMASK(level);
    }

    return 0;
}

const static struct gpio_ops _stm32_pin_ops =
{
    stm32_gpio_mode,
    stm32_gpio_write,
    stm32_gpio_read,
    stm32_gpio_attach_irq,
    stm32_gpio_deattach_irq,
    stm32_gpio_irq_enable,
    stm32_gpio_get
};

/**
 * @brief GPIO external interrupt callback handler
 * @param GPIO_Pin Pin number that triggered the interrupt
 * @return None
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    uint32_t pin_pos;

    pin_pos = __CLZ(__RBIT(GPIO_Pin));
    
    if (pin_irq_hdr_tab[pin_pos].hdr != NULL) {
        pin_irq_hdr_tab[pin_pos].hdr(pin_irq_hdr_tab[pin_pos].args);
    }
}

/**
 * @brief EXTI0 interrupt handler
 * @return None
 */
void EXTI0_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
}

/**
 * @brief EXTI1 interrupt handler
 * @return None
 */
void EXTI1_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_1);
}

/**
 * @brief EXTI2 interrupt handler
 * @return None
 */
void EXTI2_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_2);
}

/**
 * @brief EXTI3 interrupt handler
 * @return None
 */
void EXTI3_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_3);
}

/**
 * @brief EXTI4 interrupt handler
 * @return None
 */
void EXTI4_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_4);
}

/**
 * @brief EXTI9_5 interrupt handler
 * @return None
 */
void EXTI9_5_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_5);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_6);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_7);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_8);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_9);
}

/**
 * @brief EXTI15_10 interrupt handler
 * @return None
 */
void EXTI15_10_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_10);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_11);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_12);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_13);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_14);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_15);
}

/**
 * @brief Initialize GPIO
 * @return None
 */
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


