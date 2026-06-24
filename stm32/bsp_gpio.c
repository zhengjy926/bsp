 /**
  ******************************************************************************
  * @copyright   : Copyright To Hangzhou Dinova EP Technology Co.,Ltd
  * @file        : bsp_gpio.c
  * @author      : ZJY
  * @version     : V1.0
  * @date        : 2025-10-16
  * @brief       : STM32 GPIO driver implementation
  * @attention   : None
  ******************************************************************************
  * @history     :
  *         V1.0 : 1.Initial version
  *
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "bsp_gpio.h"
#include "gpio.h"
#include "errno-base.h"
#include "bsp_conf.h"
#include <string.h>

#include "elog.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/
/*
 * Combine the port number (port) and the pin number (pin) into an 8-bit
 * unique identifier (pin_id), with the port number in the high 4 bits and
 * the pin number in the low 4 bits
 */
#define PIN_ID(port, pin)           (((((port) & 0xFu) << 4) | ((pin) & 0x0Fu)))

/* Get the port index from the pin_id */
#define GET_PORT_IDX(pin_id)        ((uint8_t)(((pin_id) >> 4) & 0x0Fu))

/* Get the pin index from the pin_id */
#define PIN_GET_PIN_IDX(pin_id)     ((uint8_t)((pin_id) & 0x0Fu))

/* Pin mask on port(eg. GPIO_PIN_0, GPIO_PIN_1) */
#define PIN_MASK(pin_id)            ((uint16_t)(1u << PIN_GET_PIN_IDX(pin_id)))

/* EXTI NVIC preemption priority (must match HAL_NVIC_SetPriority in irq_enable). */
#define BSP_GPIO_EXTI_IRQ_PRIORITY    (5U)

/* EXTI IMR/EMR register name differs between STM32 families. */
#if defined(EXTI_IMR1_IM0)
#define IMR_REG    IMR1
#define EMR_REG    EMR1
#else
#define IMR_REG    IMR
#define EMR_REG    EMR
#endif

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

static const char port_name_map[] = {
  'A'
#ifdef GPIOB
, 'B'
#endif
#ifdef GPIOC
, 'C'
#endif
#ifdef GPIOD
, 'D'
#endif
#ifdef GPIOE
, 'E'
#endif
#ifdef GPIOF
, 'F'
#endif
#ifdef GPIOG
, 'G'
#endif
#ifdef GPIOH
, 'H'
#endif
#ifdef GPIOI
, 'I'
#endif
#ifdef GPIOJ
, 'J'
#endif
#ifdef GPIOK
, 'K'
#endif
#ifdef GPIOM
, 'M'
#endif
#ifdef GPION
, 'N'
#endif
#ifdef GPIOO
, 'O'
#endif
#ifdef GPIOP
, 'P'
#endif
#ifdef GPIOZ
, 'Z'
#endif
};

/* Maximum number of pins */
#define GPIO_MAX_PINS_NUM       ((sizeof(gpio_ports) / sizeof(gpio_ports[0])) * 16U)

static const IRQn_Type pin_irq_map[] = {
#if defined(STM32F0) || defined(STM32L0) || defined(STM32G0)
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
#elif defined(STM32MP1) || defined(STM32L5) || defined(STM32U5)
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
#elif defined(STM32F3)
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

/* It is used to track which EXTI lines have been configured by HAL_GPIO_Init */
static uint32_t irq_initialed_mask = 0;

/* Exported variables  -------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
static void    STM32_GPIO_ClkEnable(GPIO_TypeDef *port);
static int32_t STM32_GPIO_GetPinId(const char *name, uint8_t *pin_id);

/* Exported functions --------------------------------------------------------*/
/**
 * @brief Enable the RCC peripheral clock for the given GPIO port
 * @param port Pointer to the GPIO port base address
 * @return None
 */
static void STM32_GPIO_ClkEnable(GPIO_TypeDef *port)
{
    if (port == GPIOA) {
        __HAL_RCC_GPIOA_CLK_ENABLE();
    }
#ifdef GPIOB
    else if (port == GPIOB) {
        __HAL_RCC_GPIOB_CLK_ENABLE();
    }
#endif
#ifdef GPIOC
    else if (port == GPIOC) {
        __HAL_RCC_GPIOC_CLK_ENABLE();
    }
#endif
#ifdef GPIOD
    else if (port == GPIOD) {
        __HAL_RCC_GPIOD_CLK_ENABLE();
    }
#endif
#ifdef GPIOE
    else if (port == GPIOE) {
        __HAL_RCC_GPIOE_CLK_ENABLE();
    }
#endif
#ifdef GPIOF
    else if (port == GPIOF) {
        __HAL_RCC_GPIOF_CLK_ENABLE();
    }
#endif
#ifdef GPIOG
    else if (port == GPIOG) {
#ifdef STM32L4
        HAL_PWREx_EnableVddIO2();
#endif
        __HAL_RCC_GPIOG_CLK_ENABLE();
    }
#endif
#ifdef GPIOH
    else if (port == GPIOH) {
        __HAL_RCC_GPIOH_CLK_ENABLE();
    }
#endif
#ifdef GPIOI
    else if (port == GPIOI) {
        __HAL_RCC_GPIOI_CLK_ENABLE();
    }
#endif
#ifdef GPIOJ
    else if (port == GPIOJ) {
        __HAL_RCC_GPIOJ_CLK_ENABLE();
    }
#endif
#ifdef GPIOK
    else if (port == GPIOK) {
        __HAL_RCC_GPIOK_CLK_ENABLE();
    }
#endif
    else {
        /* Unknown port — no clock to enable */
    }
}

/**
 * @brief Check whether the pin is configured as GPIO input mode.
 * @param port GPIO port instance.
 * @param pin_index Pin index on port (0..15).
 * @return 1 if input mode, 0 otherwise.
 */
static uint32_t STM32_GPIO_IsInputMode(const GPIO_TypeDef *port, uint32_t pin_index)
{
    uint32_t moder;

    moder = (port->MODER >> (pin_index * 2U)) & 0x03U;
    return (moder == 0x00U) ? 1U : 0U;
}

/**
 * @brief Set GPIO pin mode and pull resistor
 * @param pin_id Pin identifier
 * @param mode Pin mode (INPUT/OUTPUT_PP/OUTPUT_OD)
 * @param pull_resistor Pull resistor configuration
 * @return None
 */
static int32_t STM32_GPIO_SetMode(uint8_t pin_id, PIN_Mode_e mode, PIN_Pull_e pull_resistor)
{
    GPIO_InitTypeDef  GPIO_InitStruct;
    GPIO_TypeDef     *port;
    uint16_t          pin_mask;

    if (pin_id >= GPIO_MAX_PINS_NUM) {
        return -ERR_INVAL;
    }

    port     = gpio_ports[GET_PORT_IDX(pin_id)];
    pin_mask = PIN_MASK(pin_id);

    STM32_GPIO_ClkEnable(port);

    /* Configure GPIO_InitStructure */
    GPIO_InitStruct.Pin   = (uint32_t)pin_mask;
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
    return 0;
}

/**
 * @brief Write digital value to GPIO pin
 * @param pin_id Pin identifier
 * @param value Digital value to write (0 or 1)
 * @return None
 */
static int32_t STM32_GPIO_Write(uint8_t pin_id, uint8_t value)
{
    if (pin_id >= GPIO_MAX_PINS_NUM) {
        return -ERR_INVAL;
    }
    
    GPIO_TypeDef* port     = gpio_ports[GET_PORT_IDX(pin_id)];
    uint16_t      pin_mask = PIN_MASK(pin_id);
    
    if(value) {
        port->BSRR = pin_mask;
    } else {
        port->BSRR = (uint32_t)pin_mask << 16U;
    }
    return 0;
}

/**
 * @brief Read digital value from GPIO pin
 * @param pin_id Pin identifier
 * @param value Pointer to the value to store the read value
 * @return 0 on success, -ERR_INVAL if pin id is out-of-bounds
 */
static int32_t STM32_GPIO_Read(uint8_t pin_id, uint8_t *value)
{
    if (pin_id >= GPIO_MAX_PINS_NUM) {
        return -ERR_INVAL;
    }
    
    GPIO_TypeDef* port     = gpio_ports[GET_PORT_IDX(pin_id)];
    uint16_t      pin_mask = PIN_MASK(pin_id);
    
    if((port->IDR & pin_mask) != (uint32_t)GPIO_PIN_RESET) {
        *value = 1;
    } else {
        *value = 0;
    }
    return 0;
}

/**
 * @brief Attach interrupt handler to GPIO pin
 * @param pin_id Pin identifier
 * @param event Interrupt trigger event (RISING/FALLING/RISING_FALLING)
 * @param hdr Interrupt handler function
 * @param args Argument passed to the interrupt handler
 * @return 0 on success, -ERR_NOSYS if operation is not supported
 */
static int32_t STM32_GPIO_AttachIrq(uint8_t pin_id, PIN_Event_e event, void (*hdr)(void *args), void *args)
{
    if (pin_id >= GPIO_MAX_PINS_NUM) {
        return -ERR_INVAL;
    }
    
    uint32_t pin_index = PIN_GET_PIN_IDX(pin_id);
    
    if (pin_irq_hdr_tab[pin_index].pin != -1)
    {
        log_w("Pin %d already has an interrupt handler", pin_id);
        return -ERR_BUSY;
    }
    pin_irq_hdr_tab[pin_index].pin   = pin_id;
    pin_irq_hdr_tab[pin_index].hdr   = hdr;
    pin_irq_hdr_tab[pin_index].event = event;
    pin_irq_hdr_tab[pin_index].args  = args;
    return 0;
}

/**
 * @brief Detach interrupt handler from GPIO pin
 * @param pin_id Pin identifier
 * @return 0 on success, -ERR_NOSYS if operation is not supported
 */
static int32_t STM32_GPIO_DeattachIrq(uint8_t pin_id)
{
    if (pin_id >= GPIO_MAX_PINS_NUM) { // If pin id is out-of-bounds
        return -ERR_INVAL;
    }

    uint8_t pin_index  = PIN_GET_PIN_IDX(pin_id);
    uint16_t pin_mask  = PIN_MASK(pin_id);
    uint32_t level;

    if (pin_irq_hdr_tab[pin_index].pin == -1) {
        return 0;
    }

    level = __get_PRIMASK();
    __disable_irq();

    EXTI->IMR_REG &= ~pin_mask;
    irq_initialed_mask &= ~pin_mask;

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
 * @return 0 on success, -ERR_INVAL if pin id is out-of-bounds or pin is not input mode
 */
static int32_t STM32_GPIO_IrqEnable(uint8_t pin_id, uint32_t enabled)
{
    uint32_t level;
    uint8_t pin_index;
    uint8_t port_index;
    uint16_t pin_mask;

    if (pin_id >= GPIO_MAX_PINS_NUM) {
        return -ERR_INVAL;
    }
    
    port_index = GET_PORT_IDX(pin_id);
    pin_index  = PIN_GET_PIN_IDX(pin_id);
    pin_mask   = PIN_MASK(pin_id);

    if (enabled) {
        if (pin_irq_hdr_tab[pin_index].pin == -1) {
            log_w("Pin %d has not been attached with an interrupt handler", pin_id);
            return -ERR_NOTSUPP;
        }

        level = __get_PRIMASK();
        __disable_irq();
        
        /* EXTI 线冲突检测：在临界区内读取外设寄存器，避免竞态 */
        if ( ( (EXTI->IMR_REG & pin_mask) != 0 ) ||
             ( (EXTI->EMR_REG & pin_mask) != 0 ) ) {
            log_w("EXTI Conflict: Line %d is already used\n", pin_index);
        }

        /* 引脚模式检测：在临界区内读取 MODER 寄存器 */
        if (STM32_GPIO_IsInputMode((GPIO_TypeDef*)gpio_ports[port_index], pin_index) == 0U) {
            __set_PRIMASK(level);
            log_w("Pin %d is not in input mode, call GPIO_SetMode(PIN_INPUT) first", pin_id);
            return -ERR_INVAL;
        }

        if ((irq_initialed_mask & pin_mask) == 0U) {
            EXTI_HandleTypeDef hexti;
            EXTI_ConfigTypeDef exti_config;
            HAL_StatusTypeDef  hal_ret;

            exti_config.Line    = EXTI_LINE_0 + pin_index;
            exti_config.Mode    = EXTI_MODE_INTERRUPT;
            exti_config.GPIOSel = (uint32_t)port_index;

            switch (pin_irq_hdr_tab[pin_index].event) {
                case PIN_EVENT_RISING_EDGE:
                    exti_config.Trigger = EXTI_TRIGGER_RISING;
                    break;
                case PIN_EVENT_FALLING_EDGE:
                    exti_config.Trigger = EXTI_TRIGGER_FALLING;
                    break;
                case PIN_EVENT_EITHER_EDGE:
                    exti_config.Trigger = EXTI_TRIGGER_RISING_FALLING;
                    break;
                default:
                    __set_PRIMASK(level);
                    log_w("Pin %d has invalid interrupt event", pin_id);
                    return -ERR_INVAL;
            }

            hal_ret = HAL_EXTI_SetConfigLine(&hexti, &exti_config);
            if (hal_ret != HAL_OK) {
                __set_PRIMASK(level);
                return -ERR_IO;
            }
            HAL_NVIC_SetPriority(pin_irq_map[pin_index], BSP_GPIO_EXTI_IRQ_PRIORITY, 0);
            HAL_NVIC_EnableIRQ(pin_irq_map[pin_index]);
            irq_initialed_mask |= pin_mask;
        } else {
            __HAL_GPIO_EXTI_CLEAR_FLAG(pin_mask);
            EXTI->IMR_REG |= pin_mask;
        }
    } else {
        __HAL_GPIO_EXTI_CLEAR_FLAG(pin_mask);
        EXTI->IMR_REG &= ~pin_mask;
    }
    return 0;
}

/**
 * @brief Get GPIO pin identifier from pin name string
 * @param name Pin name string, format "P<port><pin>", e.g. "PA0", "PB15"
 * @param pin_id Pointer to store the pin identifier
 * @return 0 on success, -ERR_INVAL if name is invalid or port does not exist
 */
static int32_t STM32_GPIO_GetPinId(const char *name, uint8_t *pin_id)
{
    size_t  len;
    char    port_letter;
    uint8_t pin_num;
    uint8_t port_idx;
    uint8_t num_ports;

    len = strlen(name);

    /* Valid formats: "PA0".."PZ9" (len==3) or "PA10".."PZ15" (len==4) */
    if ((len < 3U) || (len > 4U)) {
        return -ERR_INVAL;
    }

    if (name[0] != 'P') {
        return -ERR_INVAL;
    }

    port_letter = name[1];

    if ((name[2] < '0') || (name[2] > '9')) {
        return -ERR_INVAL;
    }

    if (len == 3U) {
        pin_num = (uint8_t)(name[2] - '0');
    } else {
        if ((name[3] < '0') || (name[3] > '9')) {
            return -ERR_INVAL;
        }
        pin_num = (uint8_t)(((uint8_t)(name[2] - '0') * 10U) +
                             (uint8_t)(name[3] - '0'));
    }

    if (pin_num > 15U) {
        return -ERR_INVAL;
    }

    num_ports = (uint8_t)(sizeof(port_name_map) / sizeof(port_name_map[0]));
    for (port_idx = 0U; port_idx < num_ports; port_idx++) {
        if (port_name_map[port_idx] == port_letter) {
            *pin_id = PIN_ID(port_idx, pin_num);
            return 0;
        }
    }

    return -ERR_INVAL;
}

const static struct gpio_ops _stm32_gpio_ops =
{
    .set_mode   = STM32_GPIO_SetMode,
    .write      = STM32_GPIO_Write,
    .read       = STM32_GPIO_Read,
    .attach_irq = STM32_GPIO_AttachIrq,
    .detach_irq = STM32_GPIO_DeattachIrq,
    .irq_enable = STM32_GPIO_IrqEnable,
    .get_pin_id = STM32_GPIO_GetPinId,
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
    uint32_t pending = LL_EXTI_ReadFlag_0_31(GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9);
    if ((pending & GPIO_PIN_5) != 0U) { HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_5); }
    if ((pending & GPIO_PIN_6) != 0U) { HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_6); }
    if ((pending & GPIO_PIN_7) != 0U) { HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_7); }
    if ((pending & GPIO_PIN_8) != 0U) { HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_8); }
    if ((pending & GPIO_PIN_9) != 0U) { HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_9); }
}

/**
 * @brief EXTI15_10 interrupt handler
 * @return None
 */
void EXTI15_10_IRQHandler(void)
{
    uint32_t pending = LL_EXTI_ReadFlag_0_31(GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15);
    if ((pending & GPIO_PIN_10) != 0U) { HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_10); }
    if ((pending & GPIO_PIN_11) != 0U) { HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_11); }
    if ((pending & GPIO_PIN_12) != 0U) { HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_12); }
    if ((pending & GPIO_PIN_13) != 0U) { HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_13); }
    if ((pending & GPIO_PIN_14) != 0U) { HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_14); }
    if ((pending & GPIO_PIN_15) != 0U) { HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_15); }
}

/**
 * @brief Initialize BSP GPIO
 * @return None
 */
int32_t BSP_GPIO_Init(void)
{
    return GPIO_Register(&_stm32_gpio_ops);
}
/* Private functions ---------------------------------------------------------*/
