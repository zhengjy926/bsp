/**
  ******************************************************************************
  * @file        : xxx.h
  * @author      : ZJY
  * @version     : V1.0
  * @data        : 20xx-xx-xx
  * @brief       : 
  * @attattention: None
  ******************************************************************************
  * @history     :
  *         V1.0 : 1.xxx
  *
  *
  ******************************************************************************
  */
#ifndef __GPIO_H__
#define __GPIO_H__

#ifdef __cplusplus
 extern "C" {
#endif /* __cplusplus */

/* Includes ------------------------------------------------------------------*/
#include "sys_def.h"
/* Exported define -----------------------------------------------------------*/
#define PIN_LOW                         0x00
#define PIN_HIGH                        0x01

#define PIN_MODE_INPUT                  0x00
#define PIN_MODE_OUTPUT_PP              0x01
#define PIN_MODE_OUTPUT_OD              0x02

#define PIN_PULL_RESISTOR_NONE          0x00
#define PIN_PULL_RESISTOR_UP            0x01
#define PIN_PULL_RESISTOR_DOWN          0x02

#define PIN_IRQ_MODE_RISING             0x00
#define PIN_IRQ_MODE_FALLING            0x01
#define PIN_IRQ_MODE_RISING_FALLING     0x02
#define PIN_IRQ_MODE_HIGH_LEVEL         0x03
#define PIN_IRQ_MODE_LOW_LEVEL          0x04

#define PIN_IRQ_DISABLE                 0x00
#define PIN_IRQ_ENABLE                  0x01

#define PIN_IRQ_PIN_NONE                -1
/* Exported typedef ----------------------------------------------------------*/
typedef uint32_t gpio_pin_t;

struct gpio_mode
{
    uint16_t pin;
    uint16_t mode;
};

struct gpio_status
{
    uint16_t pin;
    uint16_t status;
};

struct pin_irq_hdr
{
    int16_t        pin;
    uint16_t       mode;
    void (*hdr)(void *args);
    void             *args;
};

struct gpio_ops
{
    void (*set_mode)(size_t pin_id, size_t mode, size_t pull_resistor);
    void (*write)(size_t pin_id, uint8_t value);
    uint8_t  (*read)(size_t pin_id);
    int (*attach_irq)(size_t pin_id,
            uint8_t mode, void (*hdr)(void *args), void *args);
    int (*detach_irq)(size_t pin_id);
    int (*irq_enable)(size_t pin_id, uint8_t enabled);
    int (*get)(const char *name);
};

/* Exported macro ------------------------------------------------------------*/

/* Exported variable prototypes ----------------------------------------------*/

/* Exported function prototypes ----------------------------------------------*/
int     gpio_get(const char *name);
void    gpio_set_mode(size_t pin_id, size_t mode, size_t pull_resistor);
void    gpio_write(size_t pin_id, uint8_t value);
uint8_t gpio_read(size_t pin);
int     gpio_attach_irq(int32_t pin_id, uint32_t mode, void (*hdr)(void *args), void  *args);
int     gpio_detach_irq(int32_t pin_id);
int     gpio_irq_enable(size_t pin_id, uint32_t enabled);

int     gpio_register(const struct gpio_ops *ops);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __GPIO_H__ */
