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

/* Exported typedef ----------------------------------------------------------*/
/**
 * @brief HRTIM 脉冲配置参数
 */
typedef struct {
    uint16_t periodA;           /**< 定时器周期值 */
    uint16_t repetitionA;       /**< 重复计数值 */
    uint16_t compareA1;         /**< 比较通道1值 */
    uint16_t compareA2;         /**< 比较通道2值 */
    uint16_t compareA3;         /**< 比较通道3值 */
    uint16_t periodB;           /**< 定时器B周期值 */
    uint16_t compareB3;         /**< 比较通道3值 */
    uint16_t new_periodB;
    uint16_t new_compareB3;
} bsp_hrtim_pulse_cfg_t;

/**
 * @brief HRTIM TIMER A 重复事件回调函数类型
 */
typedef void (*bsp_hrtim_rep_callback_t)(void);

/* Exported macro ------------------------------------------------------------*/

/* Exported variable prototypes ----------------------------------------------*/

/* Exported function prototypes ----------------------------------------------*/
int      bsp_hrtim_init(void);
int      bsp_hrtim_register_rep_callback(bsp_hrtim_rep_callback_t callback);
void     bsp_hrtim_configure_pulse(const bsp_hrtim_pulse_cfg_t *cfg);
void     bsp_hrtim_start_output(void);
void     bsp_hrtim_stop_output(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BSP_HRTIM_H__ */

