/**
  ******************************************************************************
  * @file        : gpio.h
  * @author      : ZJY
  * @version     : V1.0
  * @data        : 2024-09-26
  * @brief       : 
  * @attention   : None
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

/**
  Pin Id          | Hardware Resource
  :---------------|:-----------------------
    0 ..  15      | PORTA 0..15
   16 ..  31      | PORTB 0..15
   32 ..  47      | PORTC 0..15
   48 ..  63      | PORTD 0..15
   64 ..  79      | PORTE 0..15
   80 ..  95      | PORTF 0..15
   96 .. 111      | PORTG 0..15
  112 .. 127      | PORTH 0..15
  128 .. 143      | PORTI 0..15
  144 .. 159      | PORTJ 0..15
  160 .. 175      | PORTK 0..15
  176 .. 191      | PORTM 0..15
  192 .. 207      | PORTN 0..15
  208 .. 223      | PORTO 0..15
  224 .. 239      | PORTP 0..15
  240 .. 255      | PORTZ 0..15
*/
/* Includes ------------------------------------------------------------------*/
#include "sys_def.h"
/* Exported define -----------------------------------------------------------*/
typedef enum {
    PIN_INPUT,
    PIN_OUTPUT_PP,
    PIN_OUTPUT_OD
} PIN_MODE;

/**
 * @brief GPIO Pull Resistor
 */
typedef enum {
    PIN_PULL_NONE,                   ///< None (default)
    PIN_PULL_UP,                     ///< Pull-up
    PIN_PULL_DOWN                    ///< Pull-down
} PIN_PULL_RESISTOR;

#define PIN_IRQ_MODE_RISING             0x00
#define PIN_IRQ_MODE_FALLING            0x01
#define PIN_IRQ_MODE_RISING_FALLING     0x02
#define PIN_IRQ_MODE_HIGH_LEVEL         0x03
#define PIN_IRQ_MODE_LOW_LEVEL          0x04

#define PIN_IRQ_DISABLE                 0x00
#define PIN_IRQ_ENABLE                  0x01

#define PIN_IRQ_PIN_NONE                -1
/* Exported typedef ----------------------------------------------------------*/
struct pin_irq_hdr {
    int16_t        pin;
    uint16_t       mode;
    void (*hdr)(void *args);
    void             *args;
};

struct gpio_ops {
    void    (*set_mode)     (size_t pin_id, PIN_MODE mode, PIN_PULL_RESISTOR pull_resistor);
    void    (*write)        (size_t pin_id, uint8_t value);
    uint8_t (*read)         (size_t pin_id);
    int     (*attach_irq)   (size_t pin_id, uint32_t mode, void (*hdr)(void *args), void *args);
    int     (*detach_irq)   (size_t pin_id);
    int     (*irq_enable)   (size_t pin_id, uint32_t enabled);
    int     (*get)          (const char *name);
};

/* Exported macro ------------------------------------------------------------*/

/* Exported variable prototypes ----------------------------------------------*/

/* Exported function prototypes ----------------------------------------------*/
int     gpio_get(const char *name);
void    gpio_set_mode(size_t pin_id, PIN_MODE mode, PIN_PULL_RESISTOR pull_resistor);
void    gpio_write(size_t pin_id, uint8_t value);
uint8_t gpio_read(size_t pin_id);
int     gpio_attach_irq(size_t pin_id, uint32_t mode, void (*hdr)(void *args), void *args);
int     gpio_detach_irq(size_t pin_id);
int     gpio_irq_enable(size_t pin_id, uint32_t enabled);

int     gpio_register(const struct gpio_ops *ops);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __GPIO_H__ */
