/**
  ******************************************************************************
  * @file        : xxx.h
  * @author      : ZJY
  * @version     : V1.0
  * @date        : 20xx-xx-xx
  * @brief       : 
  * @attention   : None
  ******************************************************************************
  * @history     :
  *         V1.0 : 1.xxx
  ******************************************************************************
  */
#ifndef __BSP_TIM_H__
#define __BSP_TIM_H__

#ifdef __cplusplus
 extern "C" {
#endif /* __cplusplus */

/* Includes ------------------------------------------------------------------*/
#include "board.h"

/* Exported types ------------------------------------------------------------*/
enum {
    TIM_PICK_OK=0,
    TIM_PICK_CLIPPED_MIN=1,
    TIM_PICK_CLIPPED_MAX=2
};

/* Exported constants --------------------------------------------------------*/


/* Exported macros -----------------------------------------------------------*/


/* Exported variables --------------------------------------------------------*/
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim6;
extern TIM_HandleTypeDef htim7;
extern TIM_HandleTypeDef htim15;

/* Exported functions --------------------------------------------------------*/
void MX_TIM4_Init(void);
void MX_TIM6_Init(void);
void MX_TIM7_Init(void);
void MX_TIM15_Init(void);

static inline uint64_t udiv_round_u64(uint64_t a, uint64_t b)
{
    return (a + (b>>1)) / b;
}

static inline uint64_t udiv_ceil_u64(uint64_t a, uint64_t b)
{
    return (a + b - 1) / b;
}
static inline uint64_t uabsdiff_u64(uint64_t x, uint64_t y)
{
    return (x>y)? (x-y) : (y-x);
}

int tim_pick_psc_arr_from_ns(uint32_t fclk_hz,
                           uint64_t period_ns,
                           uint16_t *psc_out,
                           uint16_t *arr_out,
                           uint64_t *p_actual_ns);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BSP_TIM_H__ */


