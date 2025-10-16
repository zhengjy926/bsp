/**
  ******************************************************************************
  * @file        : bsp_dwt.h
  * @author      : ZJY
  * @version     : V1.0
  * @date        : 2025-10-16
  * @brief       : STM32 DWT driver header file
  * @attention   : None
  ******************************************************************************
  * @history     :
  *         V1.0 : 1.Initial version
  ******************************************************************************
  */
#ifndef __BSP_DWT_H__
#define __BSP_DWT_H__

#ifdef __cplusplus
 extern "C" {
#endif /* __cplusplus */

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/* Exported define -----------------------------------------------------------*/

/* Exported typedef ----------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported variable prototypes ----------------------------------------------*/

/* Exported function prototypes ----------------------------------------------*/
int32_t bsp_dwt_init(void);
void bsp_dwt_update_overflow_counter(void);
double bsp_dwt_get_seconds(void);
void bsp_dwt_delay_us(uint32_t time_us);
void bsp_dwt_delay_ms(uint32_t time_ms);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BSP_DWT_H__ */

