/**
  ******************************************************************************
  * @file        : bsp_sram.h
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
#ifndef __BSP_SRAM_H__
#define __BSP_SRAM_H__

#ifdef __cplusplus
 extern "C" {
#endif /* __cplusplus */

/* Includes ------------------------------------------------------------------*/

/* Exported define -----------------------------------------------------------*/
#if defined(__clang__) || defined(__CC_ARM)
  #define EXTSRAM __attribute__((section("EXTSRAM"), aligned(4)))
#else
  #define EXTSRAM
#endif
/* Exported typedef ----------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported variable prototypes ----------------------------------------------*/

/* Exported function prototypes ----------------------------------------------*/
int bsp_sram_init(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BSP_SRAM_H__ */


