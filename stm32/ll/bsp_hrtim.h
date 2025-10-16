/**
  ******************************************************************************
  * @file        : bsp_hrtim.h
  * @author      : ZJY
  * @version     : V1.0
  * @date        : 2025-10-16
  * @brief       : 
  * @attention   : None
  ******************************************************************************
  * @history     :
  *         V1.0 : 1.Initial version
  ******************************************************************************
  */
#ifndef __BSP_HRTIM_H__
#define __BSP_HRTIM_H__

#ifdef __cplusplus
 extern "C" {
#endif /* __cplusplus */

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/* Exported define -----------------------------------------------------------*/
#define HRTIM_RESOLUTION        (0.68f)

#define HRTIM_BUSTMODE_FREQ_HZ  (332031u)

/* Exported typedef ----------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported variable prototypes ----------------------------------------------*/

/* Exported function prototypes ----------------------------------------------*/
void bsp_hrtim_init(void);
uint16_t tim_arr_from_ms(uint32_t ms);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BSP_HRTIM_H__ */

